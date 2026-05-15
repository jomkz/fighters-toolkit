# CB8 -- FMV Container Format (.CB8)

Multiplexed audio/video container for full-motion video. Used for intros,
cutscenes, and per-aircraft presentation clips. Each `.CB8` is paired with a
`.11K` audio file of the same stem (for playback outside the container).

Found in: `FA_4C.LIB`, `FA_4D.LIB`, `FA_10.LIB`, `FA_10B.LIB`, `FA_11.LIB`, `FA_11B.LIB`

---

## File Layout

The file begins with a 64-byte DRBC header, followed by a sequence of
variable-length typed chunks packed back-to-back. Each chunk starts with a
4-byte ASCII type tag and a 4-byte total size (including the tag and size field).

```
Offset  Size   Description
------  ----   -----------
0        64    DRBC file header
64+      var   Typed chunks: MRFA, MRFI, VooM (see below)
```

Chunk structure:
```
Offset  Size  Description
------  ----  -----------
0        4    ASCII type tag ("MRFA", "MRFI", or "VooM")
4        4    uint32 LE: total size of this chunk in bytes (including these 8 bytes)
8+       var  Chunk payload
```

---

## DRBC File Header (64 bytes)

```
Offset  Size  Description
------  ----  -----------
0        4    Magic: "DRBC" (ASCII)
4        4    uint32 LE: flags (observed: 0x00000000 or 0x00000001)
8        8    Unknown constant (observed identical across all files)
16       2    Unknown (observed: 0x0000 or 0x0080)
18      46    0xFF padding
```

---

## Chunk Type: MRFA — Audio Block

Raw PCM audio data. The chunk payload contains uncompressed 8-bit unsigned
PCM samples at 11025 Hz (matching the `.11K` convention). Silence is 0x80.

```
Offset  Size  Description
------  ----  -----------
0        4    Magic: "MRFA"
4        4    uint32 LE: chunk size (observed: 7374)
8        4    uint32 LE: unknown (observed: 128 — not a pixel dimension)
12       4    uint32 LE: unknown (observed: 0)
16       4    uint32 LE: unknown (observed: 8)
20       4    uint32 LE: unknown (observed: 1)
24    7350    Raw 8-bit unsigned PCM samples at 11025 Hz
```

7350 samples ÷ 11025 Hz = 666.7 ms = exactly 10 video frames at 15 fps.

---

## Chunk Type: MRFI — Inter Video Frame

A single delta-coded (P-frame) video frame. Sizes vary (~8,240–12,716 bytes
observed). The internal encoding has not been reversed.

```
Offset  Size  Description
------  ----  -----------
0        4    Magic: "MRFI"
4        4    uint32 LE: chunk size (variable)
8+       var  Compressed delta frame data
```

---

## Chunk Type: VooM — Video Index / Key Frame

Serves as both a video key frame and an A/V index table. The payload begins
with a fixed header describing the video stream, followed by a flat array of
8-byte index entries that alternate between audio and video chunks.

```
VooM header:
Offset  Size  Description
------  ----  -----------
0        4    Magic: "VooM"
4        4    uint32 LE: chunk size
8        4    uint32 LE: total index entry count (audio + video combined)
12       4    uint32 LE: width in pixels
16       4    uint32 LE: height in pixels
20       4    uint32 LE: unknown (observed: 6000)
24+      var  Index entries (see below), then key frame pixel data
```

Index entries (8 bytes each, starting at offset 24):
```
Offset  Size  Description
------  ----  -----------
0        4    uint32 LE: byte offset of chunk data (absolute in file)
4        4    uint32 LE: byte size of chunk data
```

Entries alternate audio/video: even entries point to MRFA audio blocks (constant
400-byte size, sequential offsets), odd entries point to MRFI video frames
(variable size, cumulative offsets).

---

## Typical Chunk Sequence

```
DRBC header
MRFA  — silent/blank audio lead-in
MRFA  — first audio block
VooM  — A/V index + video key frame (I-frame)
MRFI  — delta frame
MRFI  — delta frame
MRFI  — delta frame
MRFI  — delta frame
MRFI  — delta frame
MRFA  — next audio block (covers the preceding 5 MRFI frames + upcoming 5)
MRFI × 5
MRFA
MRFI × 5
...
```

---

## Observed Files

| File | Video | Audio | Source LIB |
|------|-------|-------|------------|
| ATF.CB8 | VooM 320×240 | .11K (external) | FA_4C.LIB |
| C_INTRO.CB8 | VooM 320×240 | .11K (external) | FA_4C.LIB |
| JANELOGO.CB8 | MRFI delta frames | MRFA blocks (11025 Hz) | FA_4C.LIB |
| B2_D.CB8 | MRFI delta frames | MRFA blocks (11025 Hz) | FA_10.LIB |
