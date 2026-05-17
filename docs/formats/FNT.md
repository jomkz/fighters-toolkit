# Font Bitmap (.FNT)

FA_1.LIB contains 15 `.FNT` files. These define the bitmapped fonts used for HUD text, menus, and briefing screens. Each is a **Win32 PE DLL** loaded at runtime via `LoadLibrary`.

## Format

Win32 PE DLL. File sizes vary — `4X12.FNT` decompresses to **12800 bytes** (0x3200). The large size relative to other 4608-byte overlays reflects the glyph function code covering the full printable ASCII range.

## File Inventory

| File | Likely dimensions / context |
|------|-----------------------------|
| `4X6.FNT` | 4×6 pixel glyphs (tiny text) |
| `4X12.FNT` | 4×12 pixel glyphs |
| `HUD00.FNT` | HUD numeric / status text |
| `HUD01.FNT` | HUD alternate style |
| `HUD11.FNT` | HUD variant |
| `HUDSYM00.FNT` | HUD symbol glyphs (non-alphanumeric) |
| `HUDSYM01.FNT` | HUD symbol variant |
| `HUDSYM11.FNT` | HUD symbol variant |
| `HUI11.FNT` | HUD interface text |
| `HUISYM11.FNT` | HUD interface symbols |
| `MAPFONT.FNT` | Theater map labels |
| `WII11.FNT` | Window interface text |
| `WIN00.FNT` | Window text (referenced as `winfont` from `.HUD` files) |
| `WIN01.FNT` | Window text variant |
| `WIN11.FNT` | Window text variant |

The name prefix encodes context (`HUD`, `WIN`, `MAP`) and suffix may encode locale or variant (`00`=base, `01`=alt, `11`=third).

## Location

| LIB | Count |
|-----|-------|
| FA_1.LIB | 13 |

## CODE Section Layout (Confirmed)

FNT files use **Phar Lap PE format** (signature `PL\0\0`). No imports. The CODE section contains a **FONT struct** — a pointer table of compiled x86 glyph functions followed by a width table, then the glyph function bodies.

### FONT struct

Starts at CODE section offset 0 (VA 0x1000). Confirmed from tracing `@G_Print@16` in FA.EXE (`0x004986B0`), which accesses the loaded FNT DLL via the global `?cFont@@3PAUFONT@@A`:

```
u32  font_height          height of all glyphs in this font, in pixels
u32  glyph_fn[256]        VAs of compiled glyph functions, one per ASCII 0–255
u32  glyph_width[256]     advance width of each glyph in pixels
```

- `cFont[0]` — font height (used in `@G_Print@16` clip bounds check)
- `cFont[char + 1]` — called as a function pointer: `(*(code *)cFont[char + 1])(dst_ptr)`
- `cFont[char + 0x101]` — advance width; passed to `_G_Blit@36` as the glyph width

Total struct: 1 + 256 + 256 = 513 `u32` values = **2052 bytes** minimum before glyph bodies.

### Glyph functions

**Each glyph is a compiled x86 function**, not encoded bitmap data. `@G_Print@16` calls each glyph function directly, passing the destination framebuffer position in registers.

**Confirmed calling convention** (traced from `@G_Print@16` at `0x004986B0` + glyph body at raw file offset `0xA25`):

| Register | Role |
|----------|------|
| `EDI` | Current row pointer in destination framebuffer |
| `ECX` | Scanline stride (bytes per row) |
| `AL` | Pixel color value |

**Instruction pattern:**

| Sequence | Meaning |
|----------|---------|
| `03 F9` = `ADD EDI, ECX` | Advance to next row |
| `88 07` = `MOV [EDI], AL` | Write pixel at current position |
| `ADD EDI, ECX` alone | Skip a blank row (no pixel written) |
| `C3` = `RET` | End of glyph |

**`0xC3` = `RET`** — the blank/space glyph is a single-byte function that returns immediately, writing nothing. Control characters (ASCII 0–31) and space (ASCII 32) all point to `0xC3` bytes (VA 0x1804–0x1824).

Printable character functions begin at VA 0x1825 (raw file offset `0xA25`). Confirmed disassembly of ASCII 33 (`!`):

```asm
ADD  EDI, ECX      ; row 0 — lit (bar)
MOV  [EDI], AL
ADD  EDI, ECX      ; row 1 — lit
MOV  [EDI], AL
ADD  EDI, ECX      ; row 2 — lit
MOV  [EDI], AL
ADD  EDI, ECX      ; row 3 — blank
ADD  EDI, ECX      ; row 4 — lit (dot)
MOV  [EDI], AL
ADD  EDI, ECX      ; row 5 — blank
ADD  EDI, ECX      ; row 6 — trailing advance
RET
```

7 row advances for a font named `4X6` suggests `cFont[0]` (font height) = 7 — the cell is 6 glyph rows + 1 inter-line spacing row. The raw bytes `{03 F9 88 07}` are two x86 instructions, not an encoded bitmap format.

**Ghidra navigation note:** When importing a FNT file as a raw binary with base `0x1000`, the CODE section (file offset `0x200`) appears at Ghidra address `0x1200`. Loaded VAs (e.g. `0x1825`) correspond to Ghidra address `0x1000 + file_offset = 0x1000 + (VA - 0x1000 + 0x200)` = VA + `0x200`.

### $$DOSX metadata

The $$DOSX section (512 bytes) contains a small header. Both 4X6.FNT and 4X12.FNT show identical $$DOSX values (`u16[4]=16, u16[5]=6`). These are system constants shared by all FNT files, not per-file cell dimensions.

## Toolkit Roadmap

Pointer table and glyph data layout are confirmed. Blocked on glyph encoding scheme (needs Ghidra trace of the drawing routine) before PNG export can produce correct output:

- New `lib/src/fnt.cpp` + `lib/include/ft/fnt.h` — parse pointer table + glyph bitmaps
- New `cli/cmd_fnt.cpp` — `ft fnt unpack <file.FNT> -o <dir>/` extracts each glyph as a 1-bpp PNG; writes `metrics.csv` with `{char, width, height}` per row
- GUI: `fnt_viewer.h/cpp` in `gui/src/editors/` for interactive glyph grid preview

## TODO

- Confirm `cFont[0]` font height value by reading the pointer table header at raw file offset `0x200` — expected 7 for `4X6.FNT` based on glyph row count
- Verify whether `00`/`01`/`11` suffix encodes locale, resolution, or style variant
- Resolve count discrepancy: file inventory lists 15 files but FA_1.LIB contains 13

## Related

- [PIC.md](PIC.md) — 8-bit indexed bitmaps, also in FA_1.LIB
- [HUD.md](HUD.md) — HUD layouts that consume font data
