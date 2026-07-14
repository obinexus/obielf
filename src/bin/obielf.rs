#![allow(clippy::print_stderr, clippy::print_stdout)]

use std::{env, fs, path::PathBuf, process::ExitCode, str::FromStr};

use obielf::{
    ArtifactKind, Assembler, DefaultPolicyEngine, ObiElfFormat, PartFile, PartId, PartMetadata,
    TargetArtifact, TargetPackage, ValidationMetric, crc32,
};

fn main() -> ExitCode {
    match run() {
        Ok(()) => ExitCode::SUCCESS,
        Err(message) => {
            eprintln!("{message}");
            ExitCode::from(1)
        }
    }
}

fn run() -> Result<(), String> {
    let mut args = env::args();
    let _program = args.next();
    let Some(command) = args.next() else {
        return Err(usage());
    };

    match command.as_str() {
        "formats" => {
            println!("obielf32");
            println!("obielf64");
            Ok(())
        }
        "package" => package(parse_package_args(args)?),
        "--help" | "-h" | "help" => Err(usage()),
        unknown => Err(format!("unknown command: {unknown}\n\n{}", usage())),
    }
}

#[derive(Debug)]
struct PackageArgs {
    format: ObiElfFormat,
    kind: ArtifactKind,
    name: String,
    profile: String,
    target_dir: PathBuf,
    inputs: Vec<PathBuf>,
}

fn parse_package_args(args: impl IntoIterator<Item = String>) -> Result<PackageArgs, String> {
    let mut format = None;
    let mut kind = None;
    let mut name = None;
    let mut profile = String::from("debug");
    let mut target_dir = PathBuf::from("target");
    let mut inputs = Vec::new();
    let mut args = args.into_iter();

    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--format" | "-f" => {
                let value = next_value(&mut args, "--format")?;
                format = Some(
                    ObiElfFormat::from_nasm_format(&value)
                        .map_err(|error| format!("{error}; expected obielf32 or obielf64"))?,
                );
            }
            "--kind" | "-k" => {
                let value = next_value(&mut args, "--kind")?;
                kind = Some(ArtifactKind::from_str(&value).map_err(|error| {
                    format!("{error}; expected executable, object, static-library, dynamic-library, or metadata")
                })?);
            }
            "--name" | "-n" => {
                name = Some(next_value(&mut args, "--name")?);
            }
            "--profile" => {
                profile = next_value(&mut args, "--profile")?;
            }
            "--target-dir" => {
                target_dir = PathBuf::from(next_value(&mut args, "--target-dir")?);
            }
            "--help" | "-h" => return Err(package_usage()),
            value if value.starts_with('-') => {
                return Err(format!("unknown option: {value}\n\n{}", package_usage()));
            }
            input => inputs.push(PathBuf::from(input)),
        }
    }

    let Some(format) = format else {
        return Err(format!("missing --format\n\n{}", package_usage()));
    };
    let Some(kind) = kind else {
        return Err(format!("missing --kind\n\n{}", package_usage()));
    };
    let Some(name) = name else {
        return Err(format!("missing --name\n\n{}", package_usage()));
    };
    if inputs.is_empty() {
        return Err(format!("missing input files\n\n{}", package_usage()));
    }

    Ok(PackageArgs {
        format,
        kind,
        name,
        profile,
        target_dir,
        inputs,
    })
}

fn next_value(args: &mut impl Iterator<Item = String>, flag: &str) -> Result<String, String> {
    args.next()
        .ok_or_else(|| format!("missing value for {flag}\n\n{}", package_usage()))
}

fn package(args: PackageArgs) -> Result<(), String> {
    let parts = read_parts(&args.inputs)?;
    let assembly = Assembler::new(DefaultPolicyEngine::default())
        .assemble(parts)
        .map_err(|error| format!("assembly failed: {error}"))?;
    let artifact = TargetArtifact::with_profile(args.name, args.format, args.kind, args.profile)
        .map_err(|error| format!("invalid target artifact: {error}"))?;
    let package = TargetPackage::new(artifact, assembly.data)
        .map_err(|error| format!("package creation failed: {error}"))?;
    let output = package
        .write_into(args.target_dir)
        .map_err(|error| format!("package write failed: {error}"))?;
    println!("{}", output.display());
    Ok(())
}

fn read_parts(paths: &[PathBuf]) -> Result<Vec<PartFile>, String> {
    let total = u32::try_from(paths.len()).map_err(|_| "too many input files".to_owned())?;
    let validation =
        ValidationMetric::new(1.0, 0, 0).map_err(|error| format!("validation metric: {error}"))?;
    let mut offset = 0_u64;
    let mut parts = Vec::with_capacity(paths.len());

    for (index, path) in paths.iter().enumerate() {
        let data = fs::read(path).map_err(|error| format!("read {}: {error}", path.display()))?;
        let index = u32::try_from(index).map_err(|_| "too many input files".to_owned())?;
        let size = u64::try_from(data.len()).map_err(|_| {
            format!(
                "input file is too large for OBIELF metadata: {}",
                path.display()
            )
        })?;
        let checksum = crc32(&data);
        let metadata = PartMetadata {
            id: part_id(index, size, checksum),
            index,
            total,
            size,
            offset,
            final_part: index + 1 == total,
            expected_checksum: checksum,
            integrity_score: 1.0,
            validation,
            minimum_ring_level: 0,
        };
        offset = offset
            .checked_add(size)
            .ok_or_else(|| "total input size overflowed OBIELF metadata".to_owned())?;
        parts.push(
            PartFile::new(metadata, data).map_err(|error| {
                format!("invalid part metadata for {}: {error}", path.display())
            })?,
        );
    }

    Ok(parts)
}

fn part_id(index: u32, size: u64, checksum: u32) -> PartId {
    let mut bytes = [0_u8; 16];
    bytes[..4].copy_from_slice(&index.to_le_bytes());
    bytes[4..12].copy_from_slice(&size.to_le_bytes());
    bytes[12..].copy_from_slice(&checksum.to_le_bytes());
    PartId::from_bytes(bytes)
}

fn usage() -> String {
    "usage:\n  obielf formats\n  obielf package --format obielf64 --kind executable --name NAME [--profile debug] [--target-dir target] INPUT..."
        .to_owned()
}

fn package_usage() -> String {
    "usage:\n  obielf package --format <obielf32|obielf64> --kind <executable|object|static-library|dynamic-library|metadata> --name NAME [--profile debug] [--target-dir target] INPUT..."
        .to_owned()
}
