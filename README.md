# OBIELF

OBIELF is the OBINexus Executable and Linkable Format workbench developed by
Nnamdi Michael Okpala for the OBINexus execution fabric. This crate provides the
Rust implementation that is ready to build, test, package, and publish through
Cargo.

The current implementation covers the stable part of the specification:
validated part metadata, deterministic assembly, bounded streaming, policy
gates, ring-level registration, telemetry, canonical NASM format identifiers,
and Cargo `target/` packaging helpers. A full upstream NASM object backend is a
separate integration item because stock NASM must know a format before
`nasm -f obielf64` or `nasm -f obielf32` can work directly.

## Implemented

- Universal token triples and AST-aware bytecode token tags.
- Validated part metadata and IEEE CRC-32 checksums.
- Deterministic sequence assembly for static and dynamic byte artifacts.
- Validation and integrity policy thresholds from the formal spec.
- Level-aware part registration for OBINexus ring deployments.
- Bounded segment streaming and thread-safe telemetry counters.
- Canonical NASM target names: `obielf32` and `obielf64`.
- Target packaging under `target/obielf/<profile>/<format>/...`.
- Cargo-installable `obielf` CLI for local packaging workflows.

The implementation forbids unsafe Rust and has no third-party dependencies.

## Target Layout

OBIELF package output is rooted under Cargo's normal `./target` directory:

```text
target/
  obielf/
    debug/
      obielf64/
        bin/                 executable OBIELF artifacts
        obj/                 non-executable relocatable objects
        lib/static/          static link libraries
        lib/dynamic/         dynamic link libraries
        meta/                metadata manifests
```

Examples:

```text
target/obielf/debug/obielf64/bin/kernel.obielf64
target/obielf/debug/obielf64/obj/kernel.obielf64.o
target/obielf/debug/obielf64/lib/static/libkernel.obielf64.a
target/obielf/debug/obielf64/lib/dynamic/libkernel.obielf64.so
```

## Build And Test

Install the current Rust toolchain, then build from this repository:

```bash
rustup update stable
rustup default stable
rustup component add clippy
cargo build
cargo test
cargo clippy --all-targets -- -D warnings
```

Build an optimized release:

```bash
cargo build --release
```

Install the local CLI during development:

```bash
cargo install --path .
obielf formats
```

After publication to crates.io:

```bash
cargo install obielf
```

## CLI Packaging

Package one or more byte artifacts into the OBIELF target layout:

```bash
obielf package \
  --format obielf64 \
  --kind executable \
  --name hello \
  --profile debug \
  --target-dir target \
  ./build/hello.part
```

Supported `--kind` values are `executable`, `object`, `static-library`,
`dynamic-library`, and `metadata`. Input files are treated as ordered OBIELF
parts, validated, assembled, and written to the matching target path.

## Rust API

```rust
use obielf::{
    ArtifactKind, Assembler, DefaultPolicyEngine, ObiElfFormat, PartFile,
    PartId, PartMetadata, TargetPackage, ValidationMetric, crc32,
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
let package = TargetPackage::from_assembly_result(
    "hello",
    ObiElfFormat::ObiElf64,
    ArtifactKind::Executable,
    "debug",
    &result,
)?;
let path = package.write_to_default_target()?;
assert!(path.ends_with("hello.obielf64"));
# Ok::<(), Box<dyn std::error::Error>>(())
```

## NASM Compatibility

OBIELF reserves these NASM object-format names:

```bash
nasm -f obielf64 main.asm -o target/obielf/debug/obielf64/obj/main.obielf64.o
nasm -f obielf32 main.asm -o target/obielf/debug/obielf32/obj/main.obielf32.o
```

Those commands require a NASM build that includes an OBIELF backend or patch.
With stock NASM today, assemble to the closest System V ELF target and use the
OBIELF crate to validate, package, and track the artifact:

```bash
nasm -f elf64 main.asm -o build/main.elf64.o
obielf package --format obielf64 --kind object --name main build/main.elf64.o
```

OBIELF-specific sections use the `.obi.*` prefix. The crate exposes stable
section constants for `.obi.policy`, `.obi.bytecode`, `.obi.trace`, and
`.obi.trust`; the byte-level ELF section schema remains a deferred normative
specification in `docs/obielf_formal_spec.md`.

## Publishing To crates.io

Before publishing, make sure the crate version is final and unique. crates.io
versions are immutable.

```bash
cargo fmt --check
cargo test
cargo clippy --all-targets -- -D warnings
cargo package --list
cargo package
cargo publish --dry-run
```

Log in once with a crates.io API token:

```bash
cargo login <CRATES_IO_API_TOKEN>
```

Publish:

```bash
cargo publish
```

After the publish completes, users can consume OBIELF as either a dependency or
a CLI:

```bash
cargo add obielf
cargo install obielf
```

## Specification Status

The implemented Rust contracts follow `docs/obielf_formal_spec.md`. The AST-aware
bytecode notes in `docs/bytecode/` guide target identity, validation, and trust
metadata. The following are intentionally tracked as separate normative work:

- OBIELF ELF32/ELF64 byte layout.
- `.obi.*` section payload schemas and alignment rules.
- Native NASM `obielf32` and `obielf64` backend integration.
- Linker and loader behavior for static binaries and dynamic libraries.
- Cryptographic signing, key management, and trust-chain verification.

## License

MIT License. See `LICENSE`.
