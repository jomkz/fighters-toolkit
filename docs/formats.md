# File Format Reference

Game assets for the Jane's combat simulator family (FA, USNF, ATF) use a small set of proprietary formats documented here. All multi-byte integers are little-endian.

---

## EALIB — Archive format (.LIB)

All game assets are packed into `.LIB` files using the EALIB container.

### File layout

```
Offset  Size  Description
------  ----  -----------
0       5     Magic: "EALIB" (ASCII, no null terminator)
5       2     uint16: number of directory entries
7+      18×N  Directory entries (see below)
[data]        File data immediately follows the directory
```

### Directory entry (18 bytes)

```
Offset  Size  Description
------  ----  -----------
0       13    Filename, null-padded, 8.3 DOS format (max 12 chars + null)
13       1    Flags byte (see below)
14       4    uint32: absolute byte offset of file data in the .LIB
```

### Flags

| Value | Name | Description |
|-------|------|-------------|
| 0     | raw  | Uncompressed — data stored verbatim |
| 1     | lzss | LZSS compressed (4-byte decompressed-size prefix) |
| 3     | pxpk | Raw with a 4-byte `PXPK` inline header |
| 4     | dcl  | PKWare DCL ("Blast/Implode") with 4-byte EA size prefix |

### File sizes

Sizes are **not stored** in directory entries. Each entry's raw size is computed as:

```
entry[i].raw_size = entry[i+1].offset - entry[i].offset
last entry's raw_size = file_size - entry[N-1].offset
```

The decompressed size for flags=4 entries is stored in the first 4 bytes of the compressed data (see EA compression wrapper below).

### Known .LIB files (Fighters Anthology)

| File | Location | Contents |
|------|----------|----------|
| FA_1.LIB | Install dir | Unknown (not yet inventoried) |
| FA_2.LIB | Install dir | Mission text, pilot bios, PAL, SEQ, PIC, misc assets |
| FA_3.LIB | CD (F:\) | 822 PIC aircraft textures (all JPEG), 269 INF aircraft data files |
| FA_4B.LIB | Unknown | Not yet examined |

---

## EA Compression — PKWare DCL wrapper (flags=4)

Compressed entries prepend a 4-byte header to a standard PKWARE DCL stream:

```
Offset  Size  Description
------  ----  -----------
0        4    uint32: decompressed size
4        1    litmode: 0x00 = binary (only supported mode)
5        1    dictbits: window size — 4=1024, 5=2048, 6=4096 bytes
6+            PKWARE DCL bitstream (LSB-first, see below)
```

### PKWare DCL algorithm

The DCL format is a direct implementation of the PKWARE Data Compression Library (implode/explode). The reference implementation is `blast.c` by Mark Adler (zlib project).

**Bit stream:** bits are read LSB-first, loaded one byte at a time into a 32-bit buffer.

**Huffman trees:** two fixed canonical trees defined by compact RLE tables.

Length symbols (16 symbols, decoded lengths 2–519):

```
RLE table: { 0x12, 0x23, 0x24, 0x35, 0x26, 0x17 }
Expanded:  [2,3,3,3,4,4,4,5,5,5,5,6,6,6,7,7]
```

Distance symbols (64 symbols):

```
RLE table: { 0x12, 0x14, 0x35, 0xE6, 0xF7, 0x97, 0xF8 }
```

Each RLE byte encodes `(reps-1)` in the high nibble and `code_length` in the low nibble.

**Decode loop:**

```
loop:
  flag = read_bits(1)
  if flag == 0:
    emit read_bits(8) as literal byte
  else:
    lsym = decode_huffman(len_tree)
    len  = lenbase[lsym] + read_bits(lenextra[lsym])
    if len == 519: break  // END symbol
    dsym = decode_huffman(dist_tree)
    if len == 2:
      dist = (dsym << 2) + read_bits(2) + 1
    else:
      dist = (dsym << dictbits) + read_bits(dictbits) + 1
    copy len bytes from output[-dist]  // overlapping copy, pre-fill zeros
```

**Critical detail — bit inversion:** the canonical Huffman decode loop inverts each raw bit before accumulating it into the code word:

```c
code |= (bitbuf & 1) ^ 1;   // ← ^1 inversion is required
```

This matches `blast.c` exactly. Without the inversion many files decompress silently to a wrong (shorter) output.

**Length/distance tables:**

```c
static const int lenbase[]  = {3,2,4,5,6,7,8,9,10,12,16,24,40,72,136,264};
static const int lenextra[] = {0,0,0,0,0,0,0,0, 1, 2, 3, 4,  5,  6,  7,  8};
```

---

## PAL — VGA palette (.PAL)

A `.PAL` file is exactly 768 bytes: 256 RGB triplets in VGA 6-bit format.

```
Offset  Size  Description
------  ----  -----------
0       768   256 × 3 bytes: R, G, B for each palette index
```

Each channel is stored as a 6-bit value (0–63). Scale to 8-bit:

```c
uint8_t to_8bit(uint8_t v) { return (v << 2) | (v >> 6); }
```

The primary system palette is `PALETTE.PAL` inside `FA_2.LIB`. It must be extracted before decoding any paletted PIC file. Each game version (USNF, ATF, FA) has its own palette — do not mix them.

Palette index **0xFF (255)** is reserved as transparent across all PIC formats.

---

## PIC — Palettized image (.PIC)

The PIC format is a custom image format used for textures, HUD overlays, instrument panels, and cockpit graphics. Three sub-formats exist, identified by the first 2 bytes.

### Header (64 bytes)

```
Offset  Size  Description
------  ----  -----------
0        2    uint16: format (0=dense, 1=sparse, 0xD8FF=JPEG)
2        4    uint32: width in pixels
6        4    uint32: height in pixels
10       4    uint32: pixels_offset — byte offset of pixel data
14       4    uint32: pixels_size — size of pixel data in bytes
18       4    uint32: palette_offset — byte offset of inline palette
22       4    uint32: palette_size — size of inline palette in bytes
26       4    uint32: spans_offset — byte offset of span records (format=1 only)
30       4    uint32: spans_size — size of span records (format=1 only)
34       4    uint32: rowheads_offset — byte offset of row table (format=0 only)
38       4    uint32: rowheads_size — size of row table (format=0 only)
42       4    uint32: font_data_offset (rarely used)
46      18    Padding, zeroed
```

Unused fields are zero for the given format.

### Pixel data

One byte per pixel — each byte is an index into a 256-color palette. Index 0xFF = transparent (fully transparent black in RGBA output). Pixels are not premultiplied.

### Palette

The inline palette (if `palette_size > 0`) is a VGA 6-bit fragment at `palette_offset`. It overlays the system palette: `palette_size / 3` entries starting at index 0.

If `palette_size == 0`, the PIC relies entirely on the system `PALETTE.PAL`.

VGA 6-bit to 8-bit scaling: `out = (in << 2) | (in >> 6)` (rotate-left-2).

### Format 0 — Dense / Texture

Used for aircraft skins, terrain textures, and full-screen backgrounds. Pixel data is sequential, row-major (top to bottom, left to right). Width × height bytes total.

```
pixels_data[y * width + x] = palette_index
```

A row-head table at `rowheads_offset` stores the absolute byte offset of each row's start as a `uint32` array (`height` entries). This is a pre-computed lookup for fast scanline access on old hardware. It must be reconstructed correctly when encoding.

```c
rowheads[y] = pixels_offset + y * width
```

### Format 1 — Sparse / Image

Used for HUD overlays, cockpit instruments, and UI elements. Most pixels are transparent; only opaque runs are stored as span records.

`spans_offset` points to an array of 10-byte span records, terminated by a record with `row = 0xFFFF`:

```
Offset  Size  Description
------  ----  -----------
0        2    uint16: row index
2        2    uint16: start column (inclusive)
4        2    uint16: end column (inclusive)
6        4    uint32: byte offset into pixels_data for this span's pixels
```

The number of pixels in a span is `end - start + 1`. `rowheads_offset` and `rowheads_size` are 0.

### Format 0xD8FF — JPEG

The entire `.PIC` file is a standard JPEG. Pass it directly to any JPEG decoder. All header fields other than the format word are meaningless. FA_3.LIB contains 822 PIC files all in this format.

---

## Text formats

### .TXT — Mission briefings, pilot bios, narrative text

Plain ASCII with FA engine markup tags on their own lines:

| Tag | Effect |
|-----|--------|
| `.body` | Switch to body text style |
| `.header` | Large header style |
| `.title` | Title/banner style |
| `.center` | Centre-align following text |
| `.right` | Right-align following text |
| `.left` | Left-align following text |
| `.button` | Interactive button |

### .INF — Aircraft information sheets

Structured ASCII with sections for aircraft stats, performance data, and descriptive text. Format varies by game version.

---

## Unimplemented / partially supported formats

| Format | Flag | Notes |
|--------|------|-------|
| LZSS | flags=1 | Decompressed size prefix + LZSS stream. Not yet implemented in ft_lib. |
| PxPk | flags=3 | 4-byte `PXPK` inline header; raw data follows. |
| .SEQ | — | Sequence / animation format. Not yet reverse-engineered. |
| .SH | — | 3D shape/model format. Documented in OpenFA `crates/shape/`. |
| .XT / .OT / .PT | — | Object, plane, and type definitions. Parsers in OpenFA. |
| .M | — | Mission definition files. OpenFA can load them directly. |
