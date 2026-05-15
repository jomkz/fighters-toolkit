# Library API

`ft_lib` is a static C++17 library. Link it from CMake:

```cmake
add_subdirectory(fighters-toolkit/lib)
target_link_libraries(your_target PRIVATE ft_lib)
```

All public headers are under `lib/include/ft/`. Include them with the `ft/` prefix:

```cpp
#include "ft/ealib.h"
#include "ft/pic.h"
// etc.
```

---

## ealib.h — Archive container

```cpp
namespace ft {

struct Entry {
    char     name[13];   // null-terminated 8.3 filename
    uint8_t  flags;      // 0=raw, 1=lzss, 3=pxpk, 4=dcl
    uint32_t offset;     // absolute byte offset in the .LIB
    uint32_t size;       // compressed/raw size in the .LIB
};

// Read the directory from a memory-mapped .LIB
std::vector<Entry>   ealib_read_dir(const uint8_t* data, size_t size);

// Extract one entry (decompress if decompress=true and flags=4)
std::vector<uint8_t> ealib_extract(const uint8_t* data, size_t size,
                                    const Entry& entry, bool decompress = true);

// Build a new .LIB from a list of (name, data) pairs (stored uncompressed)
std::vector<uint8_t> ealib_build(
    const std::vector<std::pair<std::string, std::vector<uint8_t>>>& files);

// Return a new .LIB with one entry replaced
std::vector<uint8_t> ealib_patch(const uint8_t* data, size_t size,
                                  const std::string& name,
                                  const std::vector<uint8_t>& new_data);
} // namespace ft
```

---

## pal.h — VGA palette

```cpp
namespace ft {

struct Palette {
    uint8_t r[256], g[256], b[256]; // already scaled to 8-bit (0–255)
};

Palette pal_load(const uint8_t* data, size_t size); // load a .PAL file
void    pal_save(const Palette& pal, uint8_t out[768]);
} // namespace ft
```

---

## pic.h — PIC image codec

```cpp
namespace ft {

struct PicInfo {
    uint16_t format;          // 0=dense, 1=sparse, 0xD8FF=JPEG
    uint32_t width, height;
    uint32_t pixels_offset, pixels_size;
    uint32_t palette_offset, palette_size;
    uint32_t spans_offset,   spans_size;
    uint32_t rowheads_offset, rowheads_size;
};

bool                 pic_info(const uint8_t* data, size_t size, PicInfo* info);

// Decode any PIC sub-format to RGBA8 (width * height * 4 bytes).
// sys_pal may be nullptr for JPEG or when the PIC has a full inline palette.
std::vector<uint8_t> pic_decode(const uint8_t* data, size_t size,
                                 const Palette* sys_pal);

// Encode RGBA8 to a dense PIC (format 0) with a full inline palette.
// Pixels with alpha < 128 become transparent (index 0xFF).
std::vector<uint8_t> pic_encode(const uint8_t* rgba, int w, int h,
                                 const Palette& pal);
} // namespace ft
```

---

## blast.h — PKWare DCL decompressor

```cpp
// Decompress a raw PKWare DCL stream (litmode=0, dictbits=4–6).
// Returns bytes written, or -1 on error.
int blast_decompress(const uint8_t* in, size_t in_size,
                     uint8_t* out, size_t out_capacity);

// EA wrapper: strips the 6-byte EA header before decompressing.
int blast_decompress_ea(const uint8_t* in, size_t in_size,
                        uint8_t* out, size_t out_capacity);
```

---

## seq.h — Cutscene timeline

```cpp
namespace ft {

struct SeqEvent {
    bool        relative;        // true = time is +N ticks from previous event
    int         ticks;
    std::string command;
    std::vector<std::string> args;
};

struct SeqFile {
    std::vector<std::string> header_comments;
    std::vector<SeqEvent>    events;
};

SeqFile              seq_parse(const uint8_t* data, size_t size);
std::vector<uint8_t> seq_serialize(const SeqFile&);
} // namespace ft
```

---

## audio.h — Raw PCM audio

```cpp
namespace ft {

struct AudioInfo {
    int    sample_rate;   // Hz
    size_t sample_count;
    double duration_sec;
};

AudioInfo            audio_info(const uint8_t* data, size_t size, int sample_rate);
std::vector<uint8_t> audio_to_wav(const uint8_t* data, size_t size, int sample_rate);
std::vector<uint8_t> audio_from_wav(const uint8_t* wav, size_t size,
                                     int* sample_rate_out);
} // namespace ft
```

---

## brf.h / ot.h — Type definitions

```cpp
namespace ft {

// ObjectType covers OT, NT, JT, SEE, ECM, GAS.
// PlaneType extends ObjectType with aerodynamic fields.
// Full field lists are in lib/include/ft/ot.h.

ObjectType           ot_parse(const uint8_t* data, size_t size);
PlaneType            pt_parse(const uint8_t* data, size_t size);
std::vector<uint8_t> ot_serialize(const ObjectType&);
std::vector<uint8_t> pt_serialize(const PlaneType&);
// nt_parse, jt_parse, see_parse, ecm_parse, gas_parse follow the same pattern
} // namespace ft
```

---

## mission.h — Mission and map files

```cpp
namespace ft {

struct MissionFile { /* map name, time, wind, clouds, object list */ };

MissionFile          mission_parse(const uint8_t* data, size_t size);
std::vector<uint8_t> mission_serialize(const MissionFile&);
} // namespace ft
```

---

## sh.h — 3D shape / model

```cpp
namespace ft {

struct ShVertex { float x, y, z; };   // world coordinates, feet

struct ShFace {
    uint8_t  color;          // palette index for untextured rendering
    std::string texture;     // filename from last TextureFile instruction
    std::vector<uint32_t> indices;  // 0-based into ShMesh::vertices
};

struct ShInfo {
    int   scale_raw;         // raw scale field (8 = 1 ft/unit)
    float scale;             // multiplier: raw_coord * scale = feet
    int   vert_count, face_count;
    float bbox[6];           // min_x min_y min_z max_x max_y max_z (feet)
    std::vector<std::string> textures;
};

struct ShMesh {
    float scale;
    std::vector<ShVertex>    vertices;
    std::vector<ShFace>      faces;
    std::vector<std::string> textures;
};

ShInfo      sh_parse_info(const uint8_t* data, size_t size);
ShMesh      sh_parse_mesh(const uint8_t* data, size_t size);
std::string sh_to_obj(const ShMesh& mesh);   // returns Wavefront OBJ text
} // namespace ft
```
