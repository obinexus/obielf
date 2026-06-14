use std::sync::atomic::{AtomicU64, Ordering};

#[derive(Debug, Default)]
pub struct Telemetry {
    parts_discovered: AtomicU64,
    assemblies_attempted: AtomicU64,
    assemblies_successful: AtomicU64,
    assembly_failures: AtomicU64,
    bytes_assembled: AtomicU64,
    policy_violations: AtomicU64,
}

#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub struct TelemetrySnapshot {
    pub parts_discovered: u64,
    pub assemblies_attempted: u64,
    pub assemblies_successful: u64,
    pub assembly_failures: u64,
    pub bytes_assembled: u64,
    pub policy_violations: u64,
}

impl Telemetry {
    pub(crate) fn record_part_discovered(&self) {
        self.parts_discovered.fetch_add(1, Ordering::Relaxed);
    }

    pub(crate) fn record_assembly_attempt(&self) {
        self.assemblies_attempted.fetch_add(1, Ordering::Relaxed);
    }

    pub(crate) fn record_assembly_success(&self, bytes: u64) {
        self.assemblies_successful.fetch_add(1, Ordering::Relaxed);
        self.bytes_assembled.fetch_add(bytes, Ordering::Relaxed);
    }

    pub(crate) fn record_assembly_failure(&self) {
        self.assembly_failures.fetch_add(1, Ordering::Relaxed);
    }

    pub(crate) fn record_policy_violation(&self) {
        self.policy_violations.fetch_add(1, Ordering::Relaxed);
    }

    #[must_use]
    pub fn snapshot(&self) -> TelemetrySnapshot {
        TelemetrySnapshot {
            parts_discovered: self.parts_discovered.load(Ordering::Relaxed),
            assemblies_attempted: self.assemblies_attempted.load(Ordering::Relaxed),
            assemblies_successful: self.assemblies_successful.load(Ordering::Relaxed),
            assembly_failures: self.assembly_failures.load(Ordering::Relaxed),
            bytes_assembled: self.bytes_assembled.load(Ordering::Relaxed),
            policy_violations: self.policy_violations.load(Ordering::Relaxed),
        }
    }
}
