use std::collections::BTreeMap;

use crate::{AssemblyError, PartFile, PolicyAction, PolicyContext, PolicyEngine, Telemetry, crc32};

pub const DEFAULT_MAX_PART_SIZE: u64 = 100 * 1024 * 1024;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AssemblyStatus {
    Success,
    Failure,
}

#[derive(Debug, Clone, PartialEq)]
pub struct AssemblyResult {
    pub status: AssemblyStatus,
    pub data: Vec<u8>,
    pub total_size: u64,
    pub checksum: u32,
    pub minimum_integrity_score: f64,
    pub minimum_validation_score: f64,
}

pub struct Assembler<P> {
    policy_engine: P,
    max_parts: usize,
    max_part_size: u64,
    telemetry: Telemetry,
}

impl<P: PolicyEngine> Assembler<P> {
    #[must_use]
    pub fn new(policy_engine: P) -> Self {
        Self {
            policy_engine,
            max_parts: 10_000,
            max_part_size: DEFAULT_MAX_PART_SIZE,
            telemetry: Telemetry::default(),
        }
    }

    #[must_use]
    pub const fn with_limits(mut self, max_parts: usize, max_part_size: u64) -> Self {
        self.max_parts = max_parts;
        self.max_part_size = max_part_size;
        self
    }

    #[must_use]
    pub const fn telemetry(&self) -> &Telemetry {
        &self.telemetry
    }

    /// Validates and deterministically assembles a complete part sequence.
    ///
    /// # Errors
    ///
    /// Returns [`AssemblyError`] when the sequence, part data, resource limits,
    /// or policy checks are invalid.
    pub fn assemble(
        &self,
        parts: impl IntoIterator<Item = PartFile>,
    ) -> Result<AssemblyResult, AssemblyError> {
        self.telemetry.record_assembly_attempt();
        let parts: Vec<_> = parts.into_iter().collect();
        let result = self.validate_and_assemble(parts);
        if result.is_err() {
            self.telemetry.record_assembly_failure();
        }
        result
    }

    fn validate_and_assemble(&self, parts: Vec<PartFile>) -> Result<AssemblyResult, AssemblyError> {
        if parts.is_empty() {
            return Err(AssemblyError::EmptyAssembly);
        }
        if parts.len() > self.max_parts {
            return Err(AssemblyError::TooManyParts {
                actual: parts.len(),
                maximum: self.max_parts,
            });
        }
        let expected_total = parts[0].metadata().total;
        if usize::try_from(expected_total).ok() != Some(parts.len()) {
            return Err(AssemblyError::TotalMismatch {
                expected: expected_total,
                actual: u32::try_from(parts.len()).unwrap_or(u32::MAX),
            });
        }

        let mut by_index = BTreeMap::new();
        let mut final_count = 0_u32;
        for part in parts {
            let metadata = part.metadata();
            self.telemetry.record_part_discovered();
            if metadata.total != expected_total {
                return Err(AssemblyError::TotalMismatch {
                    expected: expected_total,
                    actual: metadata.total,
                });
            }
            if metadata.size > self.max_part_size {
                return Err(AssemblyError::PartTooLarge {
                    id: metadata.id,
                    size: metadata.size,
                    maximum: self.max_part_size,
                });
            }
            part.verify_checksum()?;
            if metadata.final_part {
                final_count += 1;
            }
            let decision = self.policy_engine.evaluate(
                metadata,
                PolicyContext {
                    severity: 0,
                    part_count: usize::try_from(expected_total).unwrap_or(usize::MAX),
                },
            )?;
            if !decision.allowed
                || matches!(
                    decision.action,
                    PolicyAction::Restrict | PolicyAction::Isolate
                )
            {
                self.telemetry.record_policy_violation();
                return Err(AssemblyError::Policy(crate::PolicyError::Rejected(
                    decision.reason,
                )));
            }
            let index = metadata.index;
            if by_index.insert(index, part).is_some() {
                return Err(AssemblyError::DuplicateIndex(index));
            }
        }

        match final_count {
            0 => return Err(AssemblyError::FinalPartMissing),
            1 => {}
            _ => return Err(AssemblyError::MultipleFinalParts),
        }

        self.concatenate_parts(&by_index, expected_total)
    }

    fn concatenate_parts(
        &self,
        by_index: &BTreeMap<u32, PartFile>,
        expected_total: u32,
    ) -> Result<AssemblyResult, AssemblyError> {
        let total_capacity = by_index.values().try_fold(0_u64, |total, part| {
            total
                .checked_add(part.metadata().size)
                .ok_or(AssemblyError::TotalSizeOverflow)
        })?;
        let capacity =
            usize::try_from(total_capacity).map_err(|_| AssemblyError::TotalSizeOverflow)?;
        let mut data = Vec::with_capacity(capacity);
        let mut expected_offset = 0_u64;
        let mut minimum_integrity_score = 1.0_f64;
        let mut minimum_validation_score = 1.0_f64;

        for index in 0..expected_total {
            let part = by_index
                .get(&index)
                .ok_or(AssemblyError::MissingIndex(index))?;
            let metadata = part.metadata();
            if metadata.offset != expected_offset {
                return Err(AssemblyError::OffsetMismatch {
                    index,
                    expected: expected_offset,
                    actual: metadata.offset,
                });
            }
            if metadata.final_part != (index + 1 == expected_total) {
                return Err(AssemblyError::InvalidFinalPart(index));
            }
            data.extend_from_slice(part.data());
            expected_offset = expected_offset
                .checked_add(metadata.size)
                .ok_or(AssemblyError::TotalSizeOverflow)?;
            minimum_integrity_score = minimum_integrity_score.min(metadata.integrity_score);
            minimum_validation_score = minimum_validation_score.min(metadata.validation.score());
        }

        self.telemetry.record_assembly_success(expected_offset);
        Ok(AssemblyResult {
            status: AssemblyStatus::Success,
            checksum: crc32(&data),
            data,
            total_size: expected_offset,
            minimum_integrity_score,
            minimum_validation_score,
        })
    }
}
