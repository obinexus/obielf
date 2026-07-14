use obielf::{
    ArtifactKind, Assembler, AssemblyError, DefaultPolicyEngine, NASM_OBIELF32_FORMAT,
    NASM_OBIELF64_FORMAT, ObiElfFormat, PartError, PartFile, PartFileToken, PartId, PartMetadata,
    PolicyAction, PolicyContext, PolicyEngine, PolicyError, RingError, RingNode, StreamError,
    TargetArtifact, TargetLayout, TargetPackage, ValidationMetric, crc32, stream_part_segment,
};

fn part(id_byte: u8, index: u32, total: u32, offset: u64, data: &[u8]) -> PartFile {
    let validation = ValidationMetric::new(0.954, 0b111, 1).unwrap();
    PartFile::new(
        PartMetadata {
            id: PartId::from_bytes([id_byte; 16]),
            index,
            total,
            size: data.len() as u64,
            offset,
            final_part: index + 1 == total,
            expected_checksum: crc32(data),
            integrity_score: 0.95,
            validation,
            minimum_ring_level: 0,
        },
        data.to_vec(),
    )
    .unwrap()
}

#[test]
fn assembles_out_of_order_parts_deterministically() {
    let first = part(1, 0, 2, 0, b"OBI");
    let second = part(2, 1, 2, 3, b"ELF");
    let assembler = Assembler::new(DefaultPolicyEngine::default());

    let result = assembler.assemble([second, first]).unwrap();

    assert_eq!(result.data, b"OBIELF");
    assert_eq!(result.total_size, 6);
    assert!((result.minimum_integrity_score - 0.95).abs() < f64::EPSILON);
    assert_eq!(assembler.telemetry().snapshot().assemblies_successful, 1);
}

#[test]
fn rejects_duplicate_indices() {
    let first = part(1, 0, 2, 0, b"A");
    let duplicate = PartFile::new(
        PartMetadata {
            id: PartId::from_bytes([2; 16]),
            index: 0,
            total: 2,
            size: 1,
            offset: 0,
            final_part: false,
            expected_checksum: crc32(b"B"),
            integrity_score: 1.0,
            validation: ValidationMetric::new(1.0, 0, 0).unwrap(),
            minimum_ring_level: 0,
        },
        b"B".to_vec(),
    )
    .unwrap();

    assert_eq!(
        Assembler::new(DefaultPolicyEngine::default()).assemble([first, duplicate]),
        Err(AssemblyError::DuplicateIndex(0))
    );
}

#[test]
fn rejects_non_contiguous_offsets() {
    let first = part(1, 0, 2, 0, b"A");
    let second = part(2, 1, 2, 9, b"B");

    assert_eq!(
        Assembler::new(DefaultPolicyEngine::default()).assemble([first, second]),
        Err(AssemblyError::OffsetMismatch {
            index: 1,
            expected: 1,
            actual: 9,
        })
    );
}

#[test]
fn rejects_invalid_checksum_at_construction() {
    let result = PartFile::new(
        PartMetadata {
            id: PartId::from_bytes([1; 16]),
            index: 0,
            total: 1,
            size: 1,
            offset: 0,
            final_part: true,
            expected_checksum: 0,
            integrity_score: 1.0,
            validation: ValidationMetric::new(1.0, 0, 0).unwrap(),
            minimum_ring_level: 0,
        },
        b"A".to_vec(),
    );
    assert!(matches!(result, Err(PartError::ChecksumMismatch { .. })));
}

#[test]
fn threshold_boundaries_are_inclusive() {
    let candidate = part(1, 0, 1, 0, b"A");
    let decision = DefaultPolicyEngine::default()
        .evaluate(
            candidate.metadata(),
            PolicyContext {
                severity: 6,
                part_count: 1,
            },
        )
        .unwrap();
    assert!(decision.allowed);
    assert_eq!(decision.action, PolicyAction::Log);
}

#[test]
fn severity_outside_domain_is_rejected() {
    let candidate = part(1, 0, 1, 0, b"A");
    assert_eq!(
        DefaultPolicyEngine::default().evaluate(
            candidate.metadata(),
            PolicyContext {
                severity: 13,
                part_count: 1,
            }
        ),
        Err(PolicyError::SeverityOutOfRange(13))
    );
}

#[test]
fn ring_enforces_level_and_duplicate_identity() {
    let mut restricted = part(1, 0, 1, 0, b"A");
    let mut metadata = restricted.metadata().clone();
    metadata.minimum_ring_level = 2;
    restricted = PartFile::new(metadata, b"A".to_vec()).unwrap();

    let mut level_one = RingNode::<1, _>::new(7, DefaultPolicyEngine::default());
    assert_eq!(
        level_one.register_part_token(&PartFileToken {
            part: restricted.clone()
        }),
        Err(RingError::InsufficientPrivilege {
            required: 2,
            actual: 1,
        })
    );

    let mut level_two = RingNode::<2, _>::new(8, DefaultPolicyEngine::default());
    level_two
        .register_part_token(&PartFileToken {
            part: restricted.clone(),
        })
        .unwrap();
    assert_eq!(
        level_two.register_part_token(&PartFileToken { part: restricted }),
        Err(RingError::DuplicatePart(PartId::from_bytes([1; 16])))
    );
    assert_eq!(level_two.len(), 1);
    assert_eq!(
        level_two
            .get(&PartId::from_bytes([1; 16]))
            .unwrap()
            .minimum_ring_level,
        2
    );
}

#[test]
fn streaming_checks_buffer_and_bounds() {
    let source = part(1, 0, 1, 0, b"OBIELF");
    let segment = stream_part_segment(&source, 1, 3, 3).unwrap();
    assert_eq!(segment.data, b"BIE");

    assert_eq!(
        stream_part_segment(&source, 0, 4, 3),
        Err(StreamError::SegmentTooLarge {
            size: 4,
            maximum: 3,
        })
    );
    assert!(matches!(
        stream_part_segment(&source, 5, 2, 3),
        Err(StreamError::SegmentOutOfBounds { .. })
    ));
    assert_eq!(stream_part_segment(&source, 6, 0, 3).unwrap().data, b"");
}

#[test]
fn nasm_format_names_are_canonical_and_parseable() {
    assert_eq!(NASM_OBIELF32_FORMAT, "obielf32");
    assert_eq!(NASM_OBIELF64_FORMAT, "obielf64");
    assert_eq!(
        ObiElfFormat::from_nasm_format("obielf32").unwrap(),
        ObiElfFormat::ObiElf32
    );
    assert_eq!(
        ObiElfFormat::from_nasm_format("OBIELF64").unwrap(),
        ObiElfFormat::ObiElf64
    );
    assert_eq!(ObiElfFormat::ObiElf64.pointer_width_bits(), 64);
    assert_eq!(ObiElfFormat::ObiElf32.elf_ident_class(), 1);
}

#[test]
fn target_layout_separates_executable_object_and_library_outputs() {
    let layout = TargetLayout::cargo_default("release").unwrap();

    assert_eq!(
        layout
            .path_for("kernel", ObiElfFormat::ObiElf64, ArtifactKind::Executable)
            .unwrap()
            .to_string_lossy()
            .replace('\\', "/"),
        "target/obielf/release/obielf64/bin/kernel.obielf64"
    );
    assert_eq!(
        layout
            .path_for(
                "kernel",
                ObiElfFormat::ObiElf64,
                ArtifactKind::RelocatableObject,
            )
            .unwrap()
            .to_string_lossy()
            .replace('\\', "/"),
        "target/obielf/release/obielf64/obj/kernel.obielf64.o"
    );
    assert_eq!(
        layout
            .path_for(
                "kernel",
                ObiElfFormat::ObiElf64,
                ArtifactKind::StaticLibrary,
            )
            .unwrap()
            .to_string_lossy()
            .replace('\\', "/"),
        "target/obielf/release/obielf64/lib/static/libkernel.obielf64.a"
    );
    assert!(ArtifactKind::RelocatableObject.is_linkable());
    assert!(!ArtifactKind::Metadata.is_linkable());
}

#[test]
fn writes_package_into_target_directory() {
    let source = part(1, 0, 1, 0, b"OBIELF");
    let assembly = Assembler::new(DefaultPolicyEngine::default())
        .assemble([source])
        .unwrap();
    let artifact = TargetArtifact::with_profile(
        "hello",
        ObiElfFormat::ObiElf64,
        ArtifactKind::Executable,
        "test",
    )
    .unwrap();
    let package = TargetPackage::new(artifact, assembly.data).unwrap();
    let root = std::env::temp_dir().join(format!(
        "obielf-target-test-{}-{}",
        std::process::id(),
        std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_nanos()
    ));

    let output = package.write_into(&root).unwrap();

    assert_eq!(std::fs::read(output).unwrap(), b"OBIELF");
}
