# Formal Specification Review

Review date: June 15, 2026

## Summary

The original 1.0.0 document described a useful part-file assembly subsystem,
but it was not yet a formal specification for an executable and linkable file
format. Version 1.1.0 narrows the claims to testable behavior and records ELF,
linker, loader, cryptographic, and distributed-system work as separate
specifications.

## Findings resolved in 1.1.0

1. **Undefined terminology**

   The original quality gate used terminology that implied properties unrelated
   to byte integrity or policy compliance. It has been replaced by the
   operational terms `validation score`, `integrity score`, and `policy gate`.

2. **Encoding corruption**

   Mathematical symbols in the source were mojibake-corrupted in common
   Windows tooling. The revision uses UTF-8 text with ASCII equations where
   practical.

3. **Invalid sequence indexing**

   The original invariant mixed one-based quantified indices with zero-based
   part indices. The revision consistently uses `[0, n)`.

4. **Final-part ambiguity**

   Requiring the final flag on the set's nth enumerated item is not meaningful
   for an unordered set. The revision requires exactly one final part whose
   index is `n - 1`.

5. **Missing offset invariant**

   The assembly pseudocode ignored declared offsets. The revision requires
   contiguous offsets and the Rust implementation rejects mismatches.

6. **Unproven score monotonicity**

   A minimum input score does not mathematically prove an output score under
   the original formula. The implementation reports the minimum accepted input
   scores instead of claiming an unsupported theorem.

7. **Policy contradiction**

   The original policy function returned `allowed = true` for `Restrict` and
   `Isolate` severity actions. The revision states that only `Allow` and `Log`
   may proceed.

8. **Incomplete severity domain**

   The original generator expression failed implicitly outside severity
   `0..12`. The revision makes out-of-range severity a typed error.

9. **Checksum and integrity underspecified**

   The original required checksums without naming an algorithm. The reference
   implementation uses IEEE CRC-32 and explicitly states that it is not a
   cryptographic signature.

10. **Lookup complexity mismatch**

    The original Rust sketch used `HashMap` while claiming distributed ring
    lookup in `O(log n)`. The reference uses `BTreeMap` for deterministic local
    `O(log n)` lookup and makes no distributed-routing claim.

11. **Liveness overclaim**

    “Eventually completes” needs scheduler, storage, and network assumptions.
    The revision guarantees termination only for finite in-memory input with
    available memory.

12. **Operational targets presented as proofs**

    Latency, success-rate, and reliability percentages are deployment service
    objectives. They are no longer presented as properties guaranteed by the
    data model.

13. **Resource model gaps**

    The original did not define overflow handling or ownership of concurrency
    limits. The revision requires checked arithmetic and assigns concurrent
    assembly limits to the embedding runtime.

14. **OBIELF scope conflict**

    Repository notes describe a System V ELF extension with `.obi.*` sections,
    while the formal document describes part transport and assembly. The
    revision explicitly defers binary section layouts, ABI behavior, NASM
    integration, linking, and loading to future normative documents.

## Remaining specification work

- Define an ELF64 byte layout and compatibility rules.
- Assign `.obi.*` section names, types, flags, alignment, and payload schemas.
- Define signature algorithms, canonical bytes, key identifiers, and failure
  behavior before making cryptographic trust claims.
- Define durable part manifests and wire encodings.
- Define distributed routing only after selecting a concrete topology and
  failure model.
