# Fighters Toolkit (`ft`)

A native C++17 library and command-line tool for reading, converting, and repacking game assets from the Jane's combat simulator family.

**Supported games**
- Jane's Fighters Anthology (FA, 1998)
- U.S. Navy Fighters (USNF, 1994)
- Advanced Tactical Fighters (ATF, 1996)

---

## Why this exists

The original **FATK** (Fighters Anthology ToolKit, DuoSoft 1998) is a 16-bit Windows application that cannot run on 64-bit Windows. The [OpenFA](https://gitlab.com/openfa/openfa) project reversed-engineered all file formats in Rust and distributes `ofa-tools`, but there was no C/C++ library suitable for embedding in a GUI tool or scripting from Python/C#.

Fighters Toolkit fills that gap:

- **Zero runtime dependencies** — single statically-linked executable, no installer, no .NET, no Python
- **Library-first design** — `ft_lib` is a static C++ library; link it into any GUI frontend (Win32, ImGui, WPF via P/Invoke, Python via ctypes)
- **Replaces FATK** — handles all archive and image operations the original tool did, on modern 64-bit Windows
- **Scriptable CLI** — `ft.exe` works in batch files, PowerShell pipelines, and CI scripts

---

## Building

**Requirements:** Visual Studio 2022 or 2026 Community (MSVC) — no other tools needed.
CMake is bundled with Visual Studio; it is not in PATH by default.

```powershell
# Run from the repo root. CMake is bundled with VS but not in PATH — use the full path:
$cmake = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -B build -G "Visual Studio 18 2026"
& $cmake --build build --config Release
```

Output: `build\cli\Release\ft.exe`

The executable is self-contained — copy it anywhere, no DLLs required.

---

## CLI Reference

### Library commands

#### `ft lib ls <file.LIB>`

List the contents of a `.LIB` archive.

```
> ft lib ls FA_2.LIB
Name           Flags      Size
-------------  -----  --------
&AFTB2.11K     dcl       26488
BALTIC.TXT     dcl        3421
PALETTE.PAL    dcl        2310
...
286 file(s)
```

Flags: `raw` = uncompressed, `lzss` = LZSS, `pxpk` = PxPk, `dcl` = PKWare DCL.

---

#### `ft lib unpack <file.LIB> [output_dir]`

Extract all files from a `.LIB`, decompressing automatically. Output goes to `output_dir` (defaults to the archive stem).

```powershell
ft lib unpack FA_2.LIB out\FA_2
ft lib unpack FA_3.LIB          # extracts to .\FA_3\
```

Compressed entries (flags=4, DCL) are decompressed transparently. The extracted files are plain, uncompressed originals.

---

#### `ft lib pack <dir> <output.LIB>`

Pack all files in `dir` into a new `.LIB`. Files are stored uncompressed (flags=0); the game engine accepts both raw and compressed entries.

```powershell
ft lib pack out\FA_2 FA_2_mod.LIB
```

Filenames are truncated to 12 characters if longer (8.3 DOS format).

---

#### `ft lib patch <src.LIB> <name> <file> <output.LIB>`

Non-destructively replace one named entry in an existing `.LIB`. All other entries are preserved. The replacement file is stored uncompressed.

```powershell
# Replace BALTIC.TXT with an edited version:
ft lib patch FA_2.LIB BALTIC.TXT my_edits\BALTIC.TXT FA_2_mod.LIB

# Replace a texture:
ft lib patch FA_3.LIB F16C_0.PIC F16C_mod.PIC FA_3_mod.LIB
```

---

### Picture commands

#### `ft pic info <file.PIC>`

Display the PIC file header.

```
> ft pic info F16C_0.PIC
File:    F16C_0.PIC
Format:  0xD8FF (JPEG)

> ft pic info SOMEPLANE.PIC
File:    SOMEPLANE.PIC
Format:  0x0000 (Dense/Texture)
Size:    512 x 384
Pixels:  offset=64  size=196608
Palette: offset=196672  size=768 (256 colors)
RowHds:  offset=197440  size=1536
```

---

#### `ft pic unpack <file.PIC> [-p PALETTE.PAL] [-o output.png]`

Decode a PIC to a PNG. Handles all three PIC sub-formats: JPEG, dense, and sparse.

```powershell
# JPEG PIC (no palette needed):
ft pic unpack F16C_0.PIC -o F16C_0.png

# Dense or sparse PIC with system palette:
ft pic unpack SOMEPLANE.PIC -p PALETTE.PAL -o SOMEPLANE.png

# Default output: replaces .PIC extension with .png
ft pic unpack SOMEPLANE.PIC -p PALETTE.PAL
```

If `-p` is omitted for a paletted PIC, a greyscale palette is used (index value = grey level).

---

#### `ft pic pack <file.png> [-p PALETTE.PAL] [-o output.PIC]`

Encode a PNG to a dense PIC (format 0) with a full 256-color inline palette. The input is quantized to the nearest palette color using Euclidean RGB distance. Pixels with alpha < 128 become transparent (index 0xFF).

```powershell
ft pic pack F16C_mod.png -p PALETTE.PAL -o F16C_mod.PIC

# Default output: replaces .png extension with .PIC
ft pic pack F16C_mod.png -p PALETTE.PAL
```

**Important:** Always provide the same `PALETTE.PAL` used to unpack. The encoded PIC embeds a full 256-color inline palette derived from it.

---

## Modding Workflow

### Texture mod (FA_3.LIB aircraft skins)

```powershell
# 1. Extract textures from the CD disc image
ft lib unpack F:\FA_3.LIB out\FA_3

# 2. Decode one texture to PNG (FA_3 textures are all JPEG — no palette needed)
ft pic unpack out\FA_3\F16C_0.PIC -o F16C_0.png

# 3. Edit F16C_0.png in Photoshop, GIMP, etc. Keep the same dimensions.

# 4. Re-encode to PIC (use PALETTE.PAL extracted from FA_2.LIB)
ft pic pack F16C_0.png -p PALETTE.PAL -o F16C_mod.PIC

# 5. Patch back into the LIB
ft lib patch F:\FA_3.LIB F16C_0.PIC F16C_mod.PIC FA_3_mod.LIB

# 6. Place FA_3_mod.LIB in the game directory (overrides the CD copy if present)
```

### Text / data mod (FA_2.LIB mission text, pilot bios, etc.)

```powershell
# 1. Extract all files
ft lib unpack FA_2.LIB out\FA_2

# 2. Edit a text file (UTF-8 compatible with FA markup tags)
notepad out\FA_2\BALTIC.TXT

# 3. Patch back
ft lib patch FA_2.LIB BALTIC.TXT out\FA_2\BALTIC.TXT FA_2_mod.LIB
```

---

## Library API

`ft_lib` is a static C++ library (`lib\` directory). Link it into any project:

```cmake
add_subdirectory(fighters-toolkit/lib)
target_link_libraries(your_target PRIVATE ft_lib)
```

Include headers via the `ft/` namespace:

```cpp
#include "ft/ealib.h"
#include "ft/pic.h"
#include "ft/pal.h"
#include "ft/blast.h"

using namespace ft; // or qualify explicitly as ft::Entry, ft::Palette, etc.
```

### ealib.h — EALIB archive

```cpp
namespace ft {

struct Entry {
    char     name[13];   // null-terminated 8.3 filename
    uint8_t  flags;      // 0=raw, 1=lzss, 3=pxpk, 4=dcl
    uint32_t offset;     // absolute byte offset in the .LIB
    uint32_t size;       // compressed/raw size in the LIB
};

std::vector<Entry>   ealib_read_dir(const uint8_t* data, size_t size);
std::vector<uint8_t> ealib_extract(const uint8_t* data, size_t size,
                                    const Entry& entry, bool decompress = true);
std::vector<uint8_t> ealib_build(
    const std::vector<std::pair<std::string, std::vector<uint8_t>>>& files);
std::vector<uint8_t> ealib_patch(const uint8_t* data, size_t size,
                                  const std::string& name,
                                  const std::vector<uint8_t>& new_data);
} // namespace ft
```

### pic.h — PIC image codec

```cpp
namespace ft {

struct PicInfo {
    uint16_t format;          // 0=dense, 1=sparse, 0xD8FF=JPEG
    uint32_t width, height;
    uint32_t pixels_offset, pixels_size;
    uint32_t palette_offset, palette_size;
    uint32_t spans_offset, spans_size;
    uint32_t rowheads_offset, rowheads_size;
};

bool             pic_info(const uint8_t* data, size_t size, PicInfo* info);
std::vector<uint8_t> pic_decode(const uint8_t* data, size_t size,
                                 const Palette* sys_pal);
std::vector<uint8_t> pic_encode(const uint8_t* rgba, int w, int h,
                                 const Palette& pal);
} // namespace ft
```

### pal.h — VGA palette

```cpp
namespace ft {

struct Palette {
    uint8_t r[256], g[256], b[256]; // scaled to 8-bit
};

Palette pal_load(const uint8_t* data, size_t size);
void    pal_save(const Palette& pal, uint8_t out[768]);
} // namespace ft
```

### blast.h — PKWare DCL decompressor

```cpp
// Decompress a standard PKWare DCL stream (litmode=0, dictbits=4-6).
// Returns number of bytes written, or -1 on error.
int blast_decompress(const uint8_t* in, size_t in_size,
                     uint8_t* out, size_t out_capacity);

// EA wrapper: strips the 4-byte decompressed-size prefix before decompressing.
int blast_decompress_ea(const uint8_t* in, size_t in_size,
                        uint8_t* out, size_t out_capacity);
```

---

## File Format Reference

See [docs/formats.md](docs/formats.md) for complete format specifications.

---

## License

MIT — see [LICENSE](LICENSE).

The PKWare DCL decompressor (`lib/src/blast.cpp`) is based on `blast.c` by Mark Adler (zlib/libpng license). The `stb_image` and `stb_image_write` single-header libraries (MIT/Public Domain) are bundled in `lib/vendor/`.

### Note on game file formats

The file formats documented and implemented here were determined through reverse engineering for interoperability purposes. Jane's Fighters Anthology, U.S. Navy Fighters, and Advanced Tactical Fighters are trademarks of their respective owners. This tool does not include or distribute any copyrighted game content.
