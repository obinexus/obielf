# OBIELF

OBIELF is a Rust implementation of the Universal Part File Architecture for the
OBINexus execution fabric. The current crate focuses on deterministic,
policy-governed transport and assembly of binary parts.

## Implemented

- Universal token triples
- Validated part metadata and CRC-32 checksums
- Deterministic sequence assembly
- Validation and integrity policy gates
- Level-aware part registration
- Bounded segment streaming
- Thread-safe telemetry counters

The implementation forbids unsafe Rust and has no third-party dependencies.

## Quick start

```bash
cargo test
cargo clippy --all-targets -- -D warnings
```

```rust
use obielf::{
    Assembler, DefaultPolicyEngine, PartFile, PartId, PartMetadata,
    ValidationMetric, crc32,
};

let data = b"OBIELF".to_vec();
let part = PartFile::new(
    PartMetadata {
        id: PartId::from_bytes([1; 16]),
        index: 0,
        total: 1,
        size: data.len() as u64,
        offset: 0,
        final_part: true,
        expected_checksum: crc32(&data),
        integrity_score: 1.0,
        validation: ValidationMetric::new(1.0, 0, 0)?,
        minimum_ring_level: 0,
    },
    data,
)?;

let result = Assembler::new(DefaultPolicyEngine::default()).assemble([part])?;
assert_eq!(result.data, b"OBIELF");
# Ok::<(), Box<dyn std::error::Error>>(())
```

## Scope

This release implements the part-file contracts in
[`docs/obielf_formal_spec.md`](docs/obielf_formal_spec.md). It does not yet
define a new ELF object-file encoding or a NASM backend. Those require a
separate byte-level format specification.

## Native SDL example

[`fruit-ninja-with-obielf`](fruit-ninja-with-obielf/) is a playable C11/SDL2
example. It demonstrates direct resource paths and a checksummed `.obielf`
resource pack compiled into a linkable static library.
