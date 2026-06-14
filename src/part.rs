use core::fmt;

use crate::PartError;

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct PartId([u8; 16]);

impl PartId {
    #[must_use]
    pub const fn from_bytes(bytes: [u8; 16]) -> Self {
        Self(bytes)
    }

    #[must_use]
    pub const fn as_bytes(&self) -> &[u8; 16] {
        &self.0
    }
}

impl fmt::Debug for PartId {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        for (index, byte) in self.0.iter().enumerate() {
            if matches!(index, 4 | 6 | 8 | 10) {
                formatter.write_str("-")?;
            }
            write!(formatter, "{byte:02x}")?;
        }
        Ok(())
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct ValidationMetric {
    score: f64,
    validation_bits: u64,
    assessed_at_unix_seconds: u64,
}

impl ValidationMetric {
    /// Creates a bounded validation metric.
    ///
    /// # Errors
    ///
    /// Returns [`PartError::InvalidValidationScore`] when `score` is not finite
    /// or lies outside `[0, 1]`.
    pub fn new(
        score: f64,
        validation_bits: u64,
        assessed_at_unix_seconds: u64,
    ) -> Result<Self, PartError> {
        if !score.is_finite() || !(0.0..=1.0).contains(&score) {
            return Err(PartError::InvalidValidationScore(score));
        }
        Ok(Self {
            score,
            validation_bits,
            assessed_at_unix_seconds,
        })
    }

    #[must_use]
    pub const fn score(self) -> f64 {
        self.score
    }

    #[must_use]
    pub const fn validation_bits(self) -> u64 {
        self.validation_bits
    }

    #[must_use]
    pub const fn assessed_at_unix_seconds(self) -> u64 {
        self.assessed_at_unix_seconds
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct PartMetadata {
    pub id: PartId,
    pub index: u32,
    pub total: u32,
    pub size: u64,
    pub offset: u64,
    pub final_part: bool,
    pub expected_checksum: u32,
    pub integrity_score: f64,
    pub validation: ValidationMetric,
    pub minimum_ring_level: usize,
}

#[derive(Debug, Clone, PartialEq)]
pub struct PartFile {
    metadata: PartMetadata,
    data: Vec<u8>,
}

impl PartFile {
    /// Creates and validates an immutable part.
    ///
    /// # Errors
    ///
    /// Returns [`PartError`] when metadata is inconsistent with the data,
    /// including invalid sequence fields, size, checksum, or scores.
    pub fn new(metadata: PartMetadata, data: Vec<u8>) -> Result<Self, PartError> {
        if metadata.total == 0 {
            return Err(PartError::InvalidTotal);
        }
        if metadata.index >= metadata.total {
            return Err(PartError::IndexOutOfRange {
                index: metadata.index,
                total: metadata.total,
            });
        }
        if metadata.final_part != (metadata.index + 1 == metadata.total) {
            return Err(PartError::FinalFlagMismatch {
                index: metadata.index,
                total: metadata.total,
            });
        }
        if data.is_empty() {
            return Err(PartError::EmptyData);
        }
        let actual_size = data.len() as u64;
        if metadata.size != actual_size {
            return Err(PartError::SizeMismatch {
                declared: metadata.size,
                actual: actual_size,
            });
        }
        let actual_checksum = crc32(&data);
        if metadata.expected_checksum != actual_checksum {
            return Err(PartError::ChecksumMismatch {
                expected: metadata.expected_checksum,
                actual: actual_checksum,
            });
        }
        if !metadata.integrity_score.is_finite() || !(0.0..=1.0).contains(&metadata.integrity_score)
        {
            return Err(PartError::InvalidIntegrityScore(metadata.integrity_score));
        }
        Ok(Self { metadata, data })
    }

    #[must_use]
    pub const fn metadata(&self) -> &PartMetadata {
        &self.metadata
    }

    #[must_use]
    pub fn data(&self) -> &[u8] {
        &self.data
    }

    /// Recomputes and verifies the part checksum.
    ///
    /// # Errors
    ///
    /// Returns [`PartError::ChecksumMismatch`] when the current bytes do not
    /// match the checksum stored in metadata.
    pub fn verify_checksum(&self) -> Result<(), PartError> {
        let actual = crc32(&self.data);
        if actual == self.metadata.expected_checksum {
            Ok(())
        } else {
            Err(PartError::ChecksumMismatch {
                expected: self.metadata.expected_checksum,
                actual,
            })
        }
    }
}

/// Computes the IEEE CRC-32 checksum used for part and segment integrity.
#[must_use]
pub fn crc32(data: &[u8]) -> u32 {
    let mut crc = u32::MAX;
    for &byte in data {
        crc ^= u32::from(byte);
        for _ in 0..8 {
            let mask = (crc & 1).wrapping_neg();
            crc = (crc >> 1) ^ (0xedb8_8320 & mask);
        }
    }
    !crc
}
