use std::{
    fs,
    path::{Path, PathBuf},
    str::FromStr,
};

use crate::{AssemblyResult, TargetError};

pub const CARGO_TARGET_DIR: &str = "target";
pub const OBIELF_TARGET_DIR: &str = "obielf";
pub const NASM_OBIELF32_FORMAT: &str = "obielf32";
pub const NASM_OBIELF64_FORMAT: &str = "obielf64";
pub const OBIELF_SECTION_PREFIX: &str = ".obi.";
pub const OBIELF_POLICY_SECTION: &str = ".obi.policy";
pub const OBIELF_BYTECODE_SECTION: &str = ".obi.bytecode";
pub const OBIELF_TRACE_SECTION: &str = ".obi.trace";
pub const OBIELF_TRUST_SECTION: &str = ".obi.trust";

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ObiElfFormat {
    ObiElf32,
    ObiElf64,
}

impl ObiElfFormat {
    #[must_use]
    pub const fn nasm_format(self) -> &'static str {
        match self {
            Self::ObiElf32 => NASM_OBIELF32_FORMAT,
            Self::ObiElf64 => NASM_OBIELF64_FORMAT,
        }
    }

    #[must_use]
    pub const fn pointer_width_bits(self) -> u8 {
        match self {
            Self::ObiElf32 => 32,
            Self::ObiElf64 => 64,
        }
    }

    #[must_use]
    pub const fn pointer_width_bytes(self) -> u8 {
        self.pointer_width_bits() / 8
    }

    #[must_use]
    pub const fn elf_ident_class(self) -> u8 {
        match self {
            Self::ObiElf32 => 1,
            Self::ObiElf64 => 2,
        }
    }

    #[must_use]
    pub const fn default_alignment(self) -> u64 {
        match self {
            Self::ObiElf32 => 4,
            Self::ObiElf64 => 8,
        }
    }

    /// Parses the canonical NASM `-f` spelling reserved by OBIELF.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError::UnknownNasmFormat`] when the string is not
    /// `obielf32` or `obielf64`.
    pub fn from_nasm_format(value: &str) -> Result<Self, TargetError> {
        let value = value.trim();
        if value.eq_ignore_ascii_case(NASM_OBIELF32_FORMAT) {
            Ok(Self::ObiElf32)
        } else if value.eq_ignore_ascii_case(NASM_OBIELF64_FORMAT) {
            Ok(Self::ObiElf64)
        } else {
            Err(TargetError::UnknownNasmFormat(value.to_owned()))
        }
    }
}

impl FromStr for ObiElfFormat {
    type Err = TargetError;

    fn from_str(value: &str) -> Result<Self, Self::Err> {
        Self::from_nasm_format(value)
    }
}

impl std::fmt::Display for ObiElfFormat {
    fn fmt(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        formatter.write_str(self.nasm_format())
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum ArtifactKind {
    Executable,
    RelocatableObject,
    StaticLibrary,
    DynamicLibrary,
    Metadata,
}

impl ArtifactKind {
    #[must_use]
    pub const fn directory(self) -> &'static str {
        match self {
            Self::Executable => "bin",
            Self::RelocatableObject => "obj",
            Self::StaticLibrary => "lib/static",
            Self::DynamicLibrary => "lib/dynamic",
            Self::Metadata => "meta",
        }
    }

    #[must_use]
    pub const fn canonical_name(self) -> &'static str {
        match self {
            Self::Executable => "executable",
            Self::RelocatableObject => "object",
            Self::StaticLibrary => "static-library",
            Self::DynamicLibrary => "dynamic-library",
            Self::Metadata => "metadata",
        }
    }

    #[must_use]
    pub const fn is_executable(self) -> bool {
        matches!(self, Self::Executable)
    }

    #[must_use]
    pub const fn is_linkable(self) -> bool {
        matches!(
            self,
            Self::RelocatableObject | Self::StaticLibrary | Self::DynamicLibrary
        )
    }

    #[must_use]
    pub fn file_name(self, stem: &str, format: ObiElfFormat) -> String {
        match self {
            Self::Executable => format!("{stem}.{}", format.nasm_format()),
            Self::RelocatableObject => format!("{stem}.{}.o", format.nasm_format()),
            Self::StaticLibrary => format!("lib{stem}.{}.a", format.nasm_format()),
            Self::DynamicLibrary => format!("lib{stem}.{}.so", format.nasm_format()),
            Self::Metadata => format!("{stem}.{}.meta", format.nasm_format()),
        }
    }

    /// Parses CLI and manifest spellings for an OBIELF output kind.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError::UnknownArtifactKind`] when the string does not
    /// name one of the supported artifact kinds.
    pub fn parse(value: &str) -> Result<Self, TargetError> {
        let normalized = value.trim().to_ascii_lowercase();
        match normalized.as_str() {
            "executable" | "exec" | "bin" => Ok(Self::Executable),
            "object" | "obj" | "relocatable" | "non-executable" | "non_executable" => {
                Ok(Self::RelocatableObject)
            }
            "static-library" | "static_library" | "staticlib" | "static" | "archive" => {
                Ok(Self::StaticLibrary)
            }
            "dynamic-library" | "dynamic_library" | "dylib" | "shared" | "shared-library" => {
                Ok(Self::DynamicLibrary)
            }
            "metadata" | "meta" => Ok(Self::Metadata),
            _ => Err(TargetError::UnknownArtifactKind(value.to_owned())),
        }
    }
}

impl FromStr for ArtifactKind {
    type Err = TargetError;

    fn from_str(value: &str) -> Result<Self, Self::Err> {
        Self::parse(value)
    }
}

impl std::fmt::Display for ArtifactKind {
    fn fmt(&self, formatter: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        formatter.write_str(self.canonical_name())
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TargetArtifact {
    pub name: String,
    pub format: ObiElfFormat,
    pub kind: ArtifactKind,
    pub profile: String,
}

impl TargetArtifact {
    /// Creates an artifact descriptor under the default `debug` profile.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] when `name` is not a safe path component.
    pub fn new(
        name: impl Into<String>,
        format: ObiElfFormat,
        kind: ArtifactKind,
    ) -> Result<Self, TargetError> {
        Self::with_profile(name, format, kind, "debug")
    }

    /// Creates an artifact descriptor for a cargo profile.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] when `name` or `profile` is not a safe path
    /// component.
    pub fn with_profile(
        name: impl Into<String>,
        format: ObiElfFormat,
        kind: ArtifactKind,
        profile: impl Into<String>,
    ) -> Result<Self, TargetError> {
        let name = name.into();
        let profile = profile.into();
        validate_path_component("artifact name", &name)?;
        validate_path_component("profile", &profile)?;
        Ok(Self {
            name,
            format,
            kind,
            profile,
        })
    }

    #[must_use]
    pub fn file_name(&self) -> String {
        self.kind.file_name(&self.name, self.format)
    }

    #[must_use]
    pub fn relative_path(&self) -> PathBuf {
        PathBuf::from(OBIELF_TARGET_DIR)
            .join(&self.profile)
            .join(self.format.nasm_format())
            .join(self.kind.directory())
            .join(self.file_name())
    }

    #[must_use]
    pub fn path_in(&self, target_root: impl AsRef<Path>) -> PathBuf {
        target_root.as_ref().join(self.relative_path())
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TargetLayout {
    pub root: PathBuf,
    pub profile: String,
}

impl TargetLayout {
    /// Creates a layout rooted at cargo's default `target` directory.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] when `profile` is not a safe path component.
    pub fn cargo_default(profile: impl Into<String>) -> Result<Self, TargetError> {
        Self::new(CARGO_TARGET_DIR, profile)
    }

    /// Creates a target layout rooted at the supplied path.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] when `profile` is not a safe path component.
    pub fn new(root: impl Into<PathBuf>, profile: impl Into<String>) -> Result<Self, TargetError> {
        let profile = profile.into();
        validate_path_component("profile", &profile)?;
        Ok(Self {
            root: root.into(),
            profile,
        })
    }

    /// Creates an artifact descriptor inside this layout.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] when `name` is not a safe path component.
    pub fn artifact(
        &self,
        name: impl Into<String>,
        format: ObiElfFormat,
        kind: ArtifactKind,
    ) -> Result<TargetArtifact, TargetError> {
        TargetArtifact::with_profile(name, format, kind, self.profile.clone())
    }

    /// Computes an artifact path inside this layout.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] when `name` is not a safe path component.
    pub fn path_for(
        &self,
        name: impl Into<String>,
        format: ObiElfFormat,
        kind: ArtifactKind,
    ) -> Result<PathBuf, TargetError> {
        Ok(self.artifact(name, format, kind)?.path_in(&self.root))
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TargetPackage {
    pub artifact: TargetArtifact,
    bytes: Vec<u8>,
}

impl TargetPackage {
    /// Creates a package from bytes ready to write under `target/obielf`.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError::EmptyArtifactData`] when `bytes` is empty.
    pub fn new(artifact: TargetArtifact, bytes: impl Into<Vec<u8>>) -> Result<Self, TargetError> {
        let bytes = bytes.into();
        if bytes.is_empty() {
            return Err(TargetError::EmptyArtifactData);
        }
        Ok(Self { artifact, bytes })
    }

    /// Creates a package from a validated assembly result.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] when artifact metadata is invalid or the
    /// assembly result contains no bytes.
    pub fn from_assembly_result(
        name: impl Into<String>,
        format: ObiElfFormat,
        kind: ArtifactKind,
        profile: impl Into<String>,
        assembly: &AssemblyResult,
    ) -> Result<Self, TargetError> {
        let artifact = TargetArtifact::with_profile(name, format, kind, profile)?;
        Self::new(artifact, assembly.data.clone())
    }

    #[must_use]
    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    /// Writes the package into an explicit cargo target directory.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] if the output directory cannot be created or the
    /// artifact bytes cannot be written.
    pub fn write_into(&self, target_root: impl AsRef<Path>) -> Result<PathBuf, TargetError> {
        let path = self.artifact.path_in(target_root);
        let parent = path
            .parent()
            .ok_or_else(|| TargetError::MissingParentDirectory(path.clone()))?;
        fs::create_dir_all(parent).map_err(|error| io_error(parent, "create_dir_all", &error))?;
        fs::write(&path, &self.bytes).map_err(|error| io_error(&path, "write", &error))?;
        Ok(path)
    }

    /// Writes the package into cargo's default `./target` directory.
    ///
    /// # Errors
    ///
    /// Returns [`TargetError`] if the output directory cannot be created or the
    /// artifact bytes cannot be written.
    pub fn write_to_default_target(&self) -> Result<PathBuf, TargetError> {
        self.write_into(CARGO_TARGET_DIR)
    }
}

fn validate_path_component(field: &'static str, value: &str) -> Result<(), TargetError> {
    let valid = !value.is_empty()
        && value != "."
        && value != ".."
        && value
            .chars()
            .all(|character| !matches!(character, '/' | '\\' | ':' | '\0'));
    if valid {
        Ok(())
    } else {
        Err(TargetError::InvalidTargetComponent {
            field,
            value: value.to_owned(),
        })
    }
}

fn io_error(path: &Path, operation: &'static str, error: &std::io::Error) -> TargetError {
    TargetError::Io {
        operation,
        path: path.to_path_buf(),
        message: error.to_string(),
    }
}
