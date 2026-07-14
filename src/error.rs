use core::fmt;
use std::path::PathBuf;

use crate::PartId;

#[derive(Debug, Clone, PartialEq)]
pub enum PartError {
    EmptyData,
    IndexOutOfRange { index: u32, total: u32 },
    InvalidTotal,
    FinalFlagMismatch { index: u32, total: u32 },
    SizeMismatch { declared: u64, actual: u64 },
    ChecksumMismatch { expected: u32, actual: u32 },
    InvalidValidationScore(f64),
    InvalidIntegrityScore(f64),
}

#[derive(Debug, Clone, PartialEq)]
pub enum PolicyError {
    ValidationBelowThreshold { actual: f64, required: f64 },
    IntegrityBelowThreshold { actual: f64, required: f64 },
    ExcessivePartCount { actual: usize, maximum: usize },
    SeverityOutOfRange(u8),
    Rejected(String),
}

#[derive(Debug, Clone, PartialEq)]
pub enum AssemblyError {
    EmptyAssembly,
    TooManyParts {
        actual: usize,
        maximum: usize,
    },
    PartTooLarge {
        id: PartId,
        size: u64,
        maximum: u64,
    },
    DuplicateIndex(u32),
    TotalMismatch {
        expected: u32,
        actual: u32,
    },
    MissingIndex(u32),
    OffsetMismatch {
        index: u32,
        expected: u64,
        actual: u64,
    },
    FinalPartMissing,
    MultipleFinalParts,
    InvalidFinalPart(u32),
    Part(PartError),
    Policy(PolicyError),
    TotalSizeOverflow,
}

#[derive(Debug, Clone, PartialEq)]
pub enum RingError {
    ValidationBelowThreshold { actual: f64, required: f64 },
    InsufficientPrivilege { required: usize, actual: usize },
    PolicyViolation(PolicyError),
    DuplicatePart(PartId),
}

#[derive(Debug, Clone, PartialEq)]
pub enum StreamError {
    OffsetOutOfBounds {
        offset: u64,
        total_size: u64,
    },
    SegmentOutOfBounds {
        offset: u64,
        size: usize,
        total_size: u64,
    },
    SegmentTooLarge {
        size: usize,
        maximum: usize,
    },
    ArithmeticOverflow,
    ChecksumMismatch {
        expected: u32,
        actual: u32,
    },
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum TargetError {
    UnknownNasmFormat(String),
    UnknownArtifactKind(String),
    InvalidTargetComponent {
        field: &'static str,
        value: String,
    },
    EmptyArtifactData,
    MissingParentDirectory(PathBuf),
    Io {
        operation: &'static str,
        path: PathBuf,
        message: String,
    },
}

macro_rules! display_with_debug {
    ($($kind:ty),+ $(,)?) => {
        $(
            impl fmt::Display for $kind {
                fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
                    write!(formatter, "{self:?}")
                }
            }

            impl std::error::Error for $kind {}
        )+
    };
}

display_with_debug!(
    PartError,
    PolicyError,
    AssemblyError,
    RingError,
    StreamError,
    TargetError
);

impl From<PartError> for AssemblyError {
    fn from(error: PartError) -> Self {
        Self::Part(error)
    }
}

impl From<PolicyError> for AssemblyError {
    fn from(error: PolicyError) -> Self {
        Self::Policy(error)
    }
}
