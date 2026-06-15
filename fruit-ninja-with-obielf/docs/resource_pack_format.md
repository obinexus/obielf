# OBIELF Resource Pack v1

The Fruit Ninja example uses a small, deterministic resource container named
`OBIRPACK`. It is an application-level OBIELF part artifact, not a replacement
for the ELF executable format.

All integers are unsigned little-endian. Offsets are absolute from the start of
the file. Records and payloads are 16-byte aligned.

## Header

The header is 64 bytes.

| Offset | Size | Field |
|---:|---:|---|
| 0 | 8 | Magic: `OBIRPACK` |
| 8 | 2 | Version: `1` |
| 10 | 2 | Header size: `64` |
| 12 | 2 | Entry size: `96` |
| 14 | 2 | Flags |
| 16 | 4 | Entry count |
| 20 | 8 | Directory offset |
| 28 | 8 | Data offset |
| 36 | 8 | Total file size |
| 44 | 4 | CRC-32 of the complete directory |
| 48 | 4 | CRC-32 of the header with this field set to zero |
| 52 | 12 | Reserved, zero |

## Directory entry

Each entry is 96 bytes.

| Offset | Size | Field |
|---:|---:|---|
| 0 | 64 | UTF-8 logical name, NUL terminated and zero padded |
| 64 | 8 | Payload offset |
| 72 | 8 | Payload size |
| 80 | 4 | IEEE CRC-32 of payload |
| 84 | 2 | Resource kind (`1` = PNG) |
| 86 | 2 | Flags |
| 88 | 8 | Reserved, zero |

Entries are sorted by logical name, permitting binary search. Names must not
contain `..`, start with `/`, or use a backslash.

## Linkable form

`obielf-pack` can translate an archive into C:

```c
extern const unsigned char fruit_assets_obielf[];
extern const size_t fruit_assets_obielf_size;
```

On GCC and Clang the bytes are placed in `.obi.resources`, aligned to 16 bytes,
and retained with the `used` attribute. CMake archives this generated object as
`libfruit_assets_obielf.a` before linking it into the game.

The file and linkable forms contain identical bytes and are validated by the
same C API.

