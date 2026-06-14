# OBIELF Universal Part File Architecture

**Document version:** 1.1.0

**Specification type:** Mathematical and computational

**Target system:** OBINexus OBIELF execution fabric

**Revised:** June 15, 2026

## Abstract

This document specifies the OBIELF Universal Part File Architecture. It defines
testable semantics for tokens, part validation, policy decisions, deterministic
assembly, level-aware registration, bounded streaming, and telemetry.

The architecture is an artifact transport and assembly layer. It does not, by
itself, define an ELF file encoding, linker ABI, executable loader, distributed
hash ring, or cryptographic trust protocol.

## 1. Terms

Normative words `MUST`, `MUST NOT`, `SHOULD`, and `MAY` are interpreted as in
RFC 2119.

- **Part:** an immutable byte sequence and its metadata.
- **Assembly:** an ordered concatenation of a complete part sequence.
- **Validation score:** a bounded policy input in `[0, 1]`.
- **Integrity score:** a bounded quality input in `[0, 1]`.
- **Checksum:** an error-detection value computed over bytes.
- **Ring level:** a local authorization level. This specification does not
  prescribe a network topology.

## 2. Universal token

A universal token is a triple:

```text
T = <type, value, memory>
```

`type` classifies the token, `value` contains its payload, and `memory` is a
finite map of named byte bindings.

The token representation is an interchange abstraction. Implementations MUST
NOT assume that all runtime artifacts can be losslessly reconstructed from a
token unless a concrete encoding is separately specified.

## 3. Part domain

A part is:

```text
p = <id, index, total, size, offset, final, checksum,
     integrity_score, validation_metric, minimum_ring_level, data>
```

The fields have these constraints:

```text
total > 0
0 <= index < total
size = length(data)
final <=> index = total - 1
checksum = CRC32(data)
0 <= integrity_score <= 1
0 <= validation_metric.score <= 1
```

`id` MUST uniquely identify a part within a registry. A checksum detects
accidental corruption; it is not a cryptographic signature.

## 4. Complete sequence

For a finite assembly set `A`, let `n = |A|`. `complete(A)` holds exactly when:

```text
n > 0
forall p in A: p.total = n
forall i in [0, n): exists exactly one p in A where p.index = i
exists exactly one p in A where p.final = true
the final part has index n - 1
part[0].offset = 0
forall i in [1, n): part[i].offset =
    part[i - 1].offset + part[i - 1].size
```

Duplicate indices, missing indices, inconsistent totals, invalid final flags,
and non-contiguous offsets MUST cause assembly failure.

## 5. Validation and policy

The default thresholds are:

```text
validation_threshold = 0.954
integrity_threshold = 0.95
```

A part is eligible only when:

```text
part.validation_score >= validation_threshold
and part.integrity_score >= integrity_threshold
```

The fixed default thresholds are compatibility constants, not scientifically
derived probabilities. Implementations MAY expose configuration while recording
the active values in audit output.

Severity maps to a policy action:

| Severity | Action |
|---:|---|
| 0-3 | Allow |
| 4-6 | Log |
| 7-9 | Restrict |
| 10-12 | Isolate |

Other severity values are invalid. `Allow` and `Log` MAY proceed. `Restrict`
and `Isolate` MUST fail the requested operation.

## 6. Assembly

Given a complete sequence `A`, assembly is:

```text
assemble(A) = data(part[0]) || data(part[1]) || ... || data(part[n - 1])
```

where `||` denotes byte concatenation.

The implementation MUST:

1. reject an empty sequence;
2. enforce configured part-count and part-size limits;
3. validate each part checksum;
4. evaluate policy for every part;
5. validate completeness and contiguous offsets;
6. sort by index rather than discovery order;
7. detect arithmetic overflow before allocation;
8. return the assembled bytes, total size, checksum, minimum integrity score,
   and minimum validation score.

For a valid sequence:

```text
length(assemble(A)) = sum(p.size for p in A)
```

Assembly time is `O(n log n + B)`, where `n` is the number of parts and `B` is
the total byte count. The `n log n` term covers ordered indexing.

## 7. Level-aware registry

A registry node parameterized by level `L` MAY accept a part only when:

```text
L >= part.minimum_ring_level
```

Registration MUST also pass validation and policy checks. Registering an
existing part identifier MUST fail rather than silently replacing metadata.

The reference implementation uses an ordered map, giving `O(log n)` local
lookup. This does not establish distributed-ring routing complexity.

## 8. Streaming

A stream request is `<part, offset, size, max_buffer_size>`.

It is valid exactly when:

```text
size <= max_buffer_size
offset <= part.size
offset + size <= part.size
```

The addition MUST be checked for overflow. A successful request returns the
selected bytes and their checksum. A zero-byte request at end-of-part is valid.

The default maximum buffer target is 10 MiB. Latency depends on storage,
hardware, and scheduling and therefore cannot be guaranteed by this data model.

## 9. Telemetry

Telemetry SHOULD include:

- parts discovered;
- assemblies attempted;
- assemblies successful;
- assembly failures;
- bytes assembled;
- policy violations.

Counters MUST be safe for concurrent updates. Telemetry MUST NOT alter the
result of validation or assembly.

## 10. Safety and liveness

### Safety

- No operation may index outside a part's byte sequence.
- Invalid metadata, checksums, policies, or bounds cause explicit failure.
- Errors are returned to the caller and do not mutate a completed result.
- The reference Rust implementation forbids unsafe code.

### Liveness

For finite in-memory input and available memory, validation, assembly, and
streaming terminate with success or a typed error. This specification makes no
unbounded network or scheduler progress guarantee.

## 11. Resource limits

Reference defaults:

| Resource | Default |
|---|---:|
| Maximum part size | 100 MiB |
| Maximum streaming buffer | 10 MiB |
| Maximum parts per assembly | 10,000 |

Maximum concurrent assemblies are controlled by the embedding runtime rather
than the core data model.

## 12. Conformance

A conforming implementation MUST test:

- a valid single-part sequence;
- valid out-of-order input;
- duplicate and missing indices;
- inconsistent totals and offsets;
- absent or invalid final parts;
- checksum corruption;
- threshold boundary values;
- policy severity boundaries;
- registry authorization and duplicate identifiers;
- streaming at zero, interior, and end boundaries;
- arithmetic and configured resource limits.

Performance and reliability percentages are deployment service-level
objectives. They are not conformance proofs.

## 13. Deferred specifications

The following require separate normative documents:

- OBIELF ELF32/ELF64 binary layout;
- `.obi.*` section types and encodings;
- NASM object-format integration;
- linker and loader behavior;
- cryptographic signatures and key management;
- distributed routing and replication;
- durable on-disk part manifests.
