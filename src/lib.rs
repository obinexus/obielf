//! OBIELF Universal Part File Architecture.
//!
//! This crate implements the executable contracts in the OBIELF formal
//! specification: validated part metadata, deterministic assembly, bounded
//! streaming, policy enforcement, level-aware registration, and telemetry.

mod assembly;
mod error;
mod part;
mod policy;
mod ring;
mod stream;
mod target;
mod telemetry;
mod token;

pub use assembly::{Assembler, AssemblyResult, AssemblyStatus, DEFAULT_MAX_PART_SIZE};
pub use error::{AssemblyError, PartError, PolicyError, RingError, StreamError, TargetError};
pub use part::{PartFile, PartId, PartMetadata, ValidationMetric, crc32};
pub use policy::{DefaultPolicyEngine, PolicyAction, PolicyContext, PolicyDecision, PolicyEngine};
pub use ring::{PartFileToken, RingNode};
pub use stream::{StreamSegment, stream_part_segment};
pub use target::{
    ArtifactKind, CARGO_TARGET_DIR, NASM_OBIELF32_FORMAT, NASM_OBIELF64_FORMAT,
    OBIELF_BYTECODE_SECTION, OBIELF_POLICY_SECTION, OBIELF_SECTION_PREFIX, OBIELF_TARGET_DIR,
    OBIELF_TRACE_SECTION, OBIELF_TRUST_SECTION, ObiElfFormat, TargetArtifact, TargetLayout,
    TargetPackage,
};
pub use telemetry::{Telemetry, TelemetrySnapshot};
pub use token::{TokenMemory, TokenType, TokenValue, UniversalToken};

/// Minimum accepted validation score from the formal specification.
pub const VALIDATION_THRESHOLD: f64 = 0.954;

/// Minimum accepted integrity score from the formal specification.
pub const INTEGRITY_THRESHOLD: f64 = 0.95;
