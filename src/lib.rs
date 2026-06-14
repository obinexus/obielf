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
mod telemetry;
mod token;

pub use assembly::{Assembler, AssemblyResult, AssemblyStatus, DEFAULT_MAX_PART_SIZE};
pub use error::{AssemblyError, PartError, PolicyError, RingError, StreamError};
pub use part::{PartFile, PartId, PartMetadata, ValidationMetric, crc32};
pub use policy::{DefaultPolicyEngine, PolicyAction, PolicyContext, PolicyDecision, PolicyEngine};
pub use ring::{PartFileToken, RingNode};
pub use stream::{StreamSegment, stream_part_segment};
pub use telemetry::{Telemetry, TelemetrySnapshot};
pub use token::{TokenMemory, TokenType, TokenValue, UniversalToken};

/// Minimum accepted validation score from the formal specification.
pub const VALIDATION_THRESHOLD: f64 = 0.954;

/// Minimum accepted integrity score from the formal specification.
pub const INTEGRITY_THRESHOLD: f64 = 0.95;
