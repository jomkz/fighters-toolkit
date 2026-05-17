# Sky / Cloud / Ocean Layer (.LAY)

FA_2.LIB contains 24 `.LAY` files (e.g. `CLOUD1.LAY`, `DAY1.LAY`, `DAY2.LAY`). Each defines a complete atmospheric rendering configuration — sky gradient, cloud layers, horizon, and ocean surface — used during flight. Referenced by name from `.MM` theater files (`layer day2.LAY 0`). Each is a **Win32 PE DLL** loaded at runtime via `LoadLibrary`.

## Format

Win32 PE DLL. `CLOUD1.LAY` decompresses to **16896 bytes** — significantly larger than other overlays (4608 bytes), because `.LAY` files embed the full sky/atmosphere rendering lookup tables as data. `DAY1.LAY` decompresses to **20992 bytes**, confirming that file size is not uniform across LAY variants.

## Content

String analysis and direct CODE section analysis of `CLOUD1.LAY` and `DAY1.LAY` reveals:

- **`wave1.SH`** — animated ocean wave mesh, referenced by name in both cloud and day variants (string at CODE VA ~0x11C2)
- **`_T_HorizonProc`** — **imported** from `main.dll` (not exported). LAY files call the engine's horizon rendering function rather than implementing it.
- **Pointer table** — CODE section starts at VA 0x1000 with an array of u32 VAs pointing to sub-blocks within the same CODE section
- **Sky gradient tables** — byte arrays in CODE with values 0x09–0x3F (CLOUD1) or 0x01–0x3C (DAY1), representing sky brightness/color indices
- **Wave parameter block** — 16 bytes at CODE VA ~0x11B0: fixed-point ocean wave amplitude/frequency parameters (identical between CLOUD1 and DAY1 — not weather-specific)
- **Layer constants** — small integers and u32 counts scattered through the first 0x80 bytes of CODE

Note: the previous claim that `_T_HorizonProc` is an export was incorrect — it is an import in all observed LAY files.

## Location

| LIB | Count |
|-----|-------|
| FA_2.LIB | 24 |

## CODE Section Layout (Confirmed)

LAY files use **Phar Lap PE format** (signature `PL\0\0`). Unlike other small overlays (4608 bytes, CODE at file offset `0x200`), LAY files have a larger PE header structure:

| Section | VA | VSize | File offset | File size |
|---------|-----|-------|-------------|-----------|
| `CODE` | 0x1000 | 0x34D6 | **0x400** | 0x3600 |
| `.idata` | 0x5000 | 0x5C | 0x3A00 | 0x200 |
| `.reloc` | 0x6000 | 0x400 | 0x3C00 | 0x400 |
| `$$DOSX` | 0x7000 | 0x200 | 0x4000 | 0x200 |

(CLOUD1.LAY dimensions shown. DAY1.LAY CODE vsize = 0x40C6, file offset still 0x400.)

The CODE section contains all rendering data. The engine interprets this data; `_T_HorizonProc` (`0x4aace0` in FA.EXE, from FA.SMS) is the horizon renderer called by the LAY DLL.

### CODE section structure (VA offsets from 0x1000)

| VA range | Content |
|----------|---------|
| 0x1000–0x006F | **Pointer table** — u32 VAs to sub-blocks, with small integer group counts (6, 7) and null sentinels |
| 0x1070–0x10AF | **Layer parameters** — VA back-pointer (0x1078), u32 count=38, INT_MAX sentinel (0x7FFFFFFF), u32 5000, u32 165, and additional u32 fields |
| 0x10B0–0x1175 | **Sky gradient sub-block** — 8-byte header + 190 bytes of palette index data |
| 0x1176–0x11A5 | Zero-fill padding |
| 0x11A6–0x11FF | **Wave / scene parameter block** — u32 counts, VA pointer, wave amplitude/frequency bytes, `"wave1.SH"` string, INT_MAX sentinel, more VAs |
| 0x1200+ | **Second gradient block** + additional sub-tables (clear-sky channel) |

### Pointer table (0x1000)

Entries are u32 VAs pointing to sub-blocks within the same CODE section, mixed with u32 small integer **group count** values (6, 7). Groups are terminated by u32 zeros. First 0x70 bytes contain at least 13 sub-block VAs and 2 group counts.

**Identified sub-block types** (from inspection of pointer destinations):

| Sub-block type | Example VA | Description |
|---------------|------------|-------------|
| **Identity table** | 0x3CF8 | 256-byte passthrough: `00 01 02 ... FF`. Used when no colour remapping is needed. |
| **Colour remap table** | 0x34F8 | 256-byte table mapping code indices to upper-palette entries (0xB4–0xBF = sky blue palette range in EGA/VGA extended palette). |
| **RGB triplet array** | 0x15F8 | 3-byte tuples in `(R, G, B)` form. Observed values: `00 3F 3F` = VGA full-brightness teal/cyan. |

### Sky gradient sub-block (0x10B0)

```
31 00 00 00 00 00 10 10  ← 8-byte header
0E 3F 3F 3F 3B 3B 3C 38 38 39 35 35 36 31 31 32  ← gradient data begins
31 31 32 32 32 33 32 32 33 33 33 33 33 33 34 34
34 34 34 34 35 34 34 35 35 35 36 36 36 37 35 35
36 ...
```

**Header fields:**

| Offset | Type | Value | Meaning |
|--------|------|-------|---------|
| +0 | u32 | 0x31 = 49 | Entries per sub-table (or sub-table type ID) |
| +4 | u16 | 0 | Padding / reserved |
| +6 | u8 | 0x10 = 16 | Unknown (dimension / channel count) |
| +7 | u8 | 0x10 = 16 | Unknown (dimension / channel count) |

Gradient data (190 bytes, VA 0x10B8–0x1175) encodes sky color as **VGA 6-bit palette indices** (range 0–63). Values are non-monotonic — not a simple linear ramp. The curve represents sky brightness/color as a function of altitude band, e.g. bright at horizon (0x3F=63) transitioning through mid-sky blues (0x31–0x37) and darker values toward zenith. Multiple sequential sub-tables follow within the 190-byte range.

### Wave / scene parameter block (~0x11A6)

```
66 00 00 00  ← u32 0x66=102
DD 02 00 00  ← u32 0x2DD=733
00 00 00 00
D0 44 00 00  ← VA pointer (0x44D0, within CODE)
FE 1F 38 0E 70 62 00 00 30 0B 01 00 18 47 E8 B8  ← wave amplitude/freq params
4B 64 64 64 64                                   ← 5 bytes
77 61 76 65 31 2E 53 48 00                       ← "wave1.SH\0" — ocean mesh
FF FF FF 7F                                      ← INT_MAX sentinel
94 11 00 00  ← VA 0x1194
1C 25 00 00  ← VA 0x251C
...
```

`"wave1.SH"` is the ocean wave mesh loaded by the LAY DLL. The wave parameters (16 bytes preceding the string) are **identical across all CLOUD1 and DAY1 variants** — weather state does not affect wave motion physics.

### CLOUD1 vs DAY1 comparison

| Property | CLOUD1 | DAY1 |
|----------|--------|------|
| Total file | 16896 bytes | 20992 bytes (+4096) |
| CODE VSize | 0x34D6 (13526) | 0x40C6 (16582) |
| .reloc size | 0x400 (1024) | 0x800 (2048) |
| Differing bytes | — | 15202/16896 |

DAY1 has 3056 more bytes of CODE and 1024 more bytes of relocation data. The extra CODE contains additional sky gradient sub-tables and colour remap tables for clear-day conditions (no cloud cover). Nearly all bytes differ because inserting new sub-blocks shifts the VA space and invalidates most pointer values throughout the file.

### CLOUD1B is identical to CLOUD1

Binary diff of `CLOUD1B.LAY` vs `CLOUD1.LAY` shows **zero differences** — the files are byte-for-byte copies. The `B` suffix does **not** encode a brightness variant for this file. Its purpose (alias, duplicate, or reserved slot) is unknown.

## Toolkit Roadmap

Sub-table structure and gradient header are partially confirmed. Blocked on exact row stride and sub-block type map before JSON/PNG export can produce meaningful output:

- New `lib/src/lay.cpp` + `lib/include/ft/lay.h` — parse pointer table, gradient byte arrays, wave params
- New `cli/cmd_lay.cpp`:
  - `ft lay dump <file.LAY>` — exports atmosphere parameters as JSON
  - `ft lay gradient <file.LAY> -o gradient.png` — renders sky colour ramp as a PNG strip (one pixel per step)

## TODO — Deep Dive

- Decode gradient header `10 10` bytes — meaning of both u8 values unknown (channel count? sub-table count?)
- Confirm `0x31` is entries-per-sub-table vs. a sub-block type ID — needs `LAYER_FILE_HEADER` / `LAYER` struct layout from FA.SMS type info (symbols confirmed present but not yet loaded into Ghidra)
- Explain CLOUD1B = CLOUD1 (byte-for-byte identical — alias, stub, or reserved slot?)
- Map the `layer <name>.LAY <index>` slot index from `.MM` files to rendering layers
- Document CLOUD / DAY / other prefix naming convention

## Related

- [MM.md](MM.md) — theater files that reference `.LAY` files via the `layer` keyword
- [SH.md](SH.md) — `wave1.SH` is the ocean wave mesh loaded by `.LAY`
- [PIC.md](PIC.md) — `ocean*06.PIC` ocean texture atlas
