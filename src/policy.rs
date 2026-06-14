use crate::{INTEGRITY_THRESHOLD, PartMetadata, PolicyError, VALIDATION_THRESHOLD};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PolicyAction {
    Allow,
    Log,
    Restrict,
    Isolate,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PolicyContext {
    pub severity: u8,
    pub part_count: usize,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PolicyDecision {
    pub allowed: bool,
    pub action: PolicyAction,
    pub reason: String,
}

pub trait PolicyEngine {
    /// Evaluates a part in an operation-specific context.
    ///
    /// # Errors
    ///
    /// Returns [`PolicyError`] when a threshold, resource limit, or severity
    /// rule is violated.
    fn evaluate(
        &self,
        part: &PartMetadata,
        context: PolicyContext,
    ) -> Result<PolicyDecision, PolicyError>;
}

#[derive(Debug, Clone)]
pub struct DefaultPolicyEngine {
    pub validation_threshold: f64,
    pub integrity_threshold: f64,
    pub max_parts: usize,
}

impl Default for DefaultPolicyEngine {
    fn default() -> Self {
        Self {
            validation_threshold: VALIDATION_THRESHOLD,
            integrity_threshold: INTEGRITY_THRESHOLD,
            max_parts: 10_000,
        }
    }
}

impl PolicyEngine for DefaultPolicyEngine {
    fn evaluate(
        &self,
        part: &PartMetadata,
        context: PolicyContext,
    ) -> Result<PolicyDecision, PolicyError> {
        if part.validation.score() < self.validation_threshold {
            return Err(PolicyError::ValidationBelowThreshold {
                actual: part.validation.score(),
                required: self.validation_threshold,
            });
        }
        if part.integrity_score < self.integrity_threshold {
            return Err(PolicyError::IntegrityBelowThreshold {
                actual: part.integrity_score,
                required: self.integrity_threshold,
            });
        }
        if context.part_count > self.max_parts {
            return Err(PolicyError::ExcessivePartCount {
                actual: context.part_count,
                maximum: self.max_parts,
            });
        }

        let action = match context.severity {
            0..=3 => PolicyAction::Allow,
            4..=6 => PolicyAction::Log,
            7..=9 => PolicyAction::Restrict,
            10..=12 => PolicyAction::Isolate,
            severity => return Err(PolicyError::SeverityOutOfRange(severity)),
        };
        Ok(PolicyDecision {
            allowed: matches!(action, PolicyAction::Allow | PolicyAction::Log),
            action,
            reason: format!("severity {}", context.severity),
        })
    }
}
