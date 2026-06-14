use crate::{PartFile, PartId, StreamError, crc32};

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StreamSegment {
    pub part_id: PartId,
    pub offset: u64,
    pub data: Vec<u8>,
    pub checksum: u32,
}

/// Returns a checked, bounded byte segment from a part.
///
/// # Errors
///
/// Returns [`StreamError`] when the requested buffer exceeds its configured
/// limit, arithmetic overflows, or the range lies outside the part.
pub fn stream_part_segment(
    part: &PartFile,
    offset: u64,
    size: usize,
    max_buffer_size: usize,
) -> Result<StreamSegment, StreamError> {
    if size > max_buffer_size {
        return Err(StreamError::SegmentTooLarge {
            size,
            maximum: max_buffer_size,
        });
    }
    let total_size = part.metadata().size;
    if offset > total_size {
        return Err(StreamError::OffsetOutOfBounds { offset, total_size });
    }
    let start = usize::try_from(offset).map_err(|_| StreamError::ArithmeticOverflow)?;
    let end = start
        .checked_add(size)
        .ok_or(StreamError::ArithmeticOverflow)?;
    if end > part.data().len() {
        return Err(StreamError::SegmentOutOfBounds {
            offset,
            size,
            total_size,
        });
    }
    let data = part.data()[start..end].to_vec();
    let checksum = crc32(&data);
    Ok(StreamSegment {
        part_id: part.metadata().id,
        offset,
        data,
        checksum,
    })
}
