use std::collections::BTreeMap;

use crate::{
    PartFile, PartId, PartMetadata, PolicyAction, PolicyContext, PolicyEngine, RingError,
    VALIDATION_THRESHOLD,
};

#[derive(Debug, Clone, PartialEq)]
pub struct PartFileToken {
    pub part: PartFile,
}

pub struct RingNode<const LEVEL: usize, P> {
    node_id: u64,
    part_registry: BTreeMap<PartId, PartMetadata>,
    validation_threshold: f64,
    policy_engine: P,
}

impl<const LEVEL: usize, P: PolicyEngine> RingNode<LEVEL, P> {
    #[must_use]
    pub fn new(node_id: u64, policy_engine: P) -> Self {
        Self {
            node_id,
            part_registry: BTreeMap::new(),
            validation_threshold: VALIDATION_THRESHOLD,
            policy_engine,
        }
    }

    #[must_use]
    pub const fn node_id(&self) -> u64 {
        self.node_id
    }

    #[must_use]
    pub const fn level(&self) -> usize {
        LEVEL
    }

    #[must_use]
    pub fn len(&self) -> usize {
        self.part_registry.len()
    }

    #[must_use]
    pub fn is_empty(&self) -> bool {
        self.part_registry.is_empty()
    }

    #[must_use]
    pub fn get(&self, id: &PartId) -> Option<&PartMetadata> {
        self.part_registry.get(id)
    }

    /// Registers a part after authorization and policy validation.
    ///
    /// # Errors
    ///
    /// Returns [`RingError`] when validation, authorization, policy, or unique
    /// identity requirements are not satisfied.
    pub fn register_part_token(&mut self, token: &PartFileToken) -> Result<(), RingError> {
        let metadata = token.part.metadata();
        if metadata.validation.score() < self.validation_threshold {
            return Err(RingError::ValidationBelowThreshold {
                actual: metadata.validation.score(),
                required: self.validation_threshold,
            });
        }
        if LEVEL < metadata.minimum_ring_level {
            return Err(RingError::InsufficientPrivilege {
                required: metadata.minimum_ring_level,
                actual: LEVEL,
            });
        }
        let decision = self
            .policy_engine
            .evaluate(
                metadata,
                PolicyContext {
                    severity: 0,
                    part_count: self.part_registry.len() + 1,
                },
            )
            .map_err(RingError::PolicyViolation)?;
        if !decision.allowed
            || matches!(
                decision.action,
                PolicyAction::Restrict | PolicyAction::Isolate
            )
        {
            return Err(RingError::PolicyViolation(crate::PolicyError::Rejected(
                decision.reason,
            )));
        }
        if self.part_registry.contains_key(&metadata.id) {
            return Err(RingError::DuplicatePart(metadata.id));
        }
        self.part_registry.insert(metadata.id, metadata.clone());
        Ok(())
    }
}
