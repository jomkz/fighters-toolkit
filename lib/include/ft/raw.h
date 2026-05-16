#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

// RAW screenshot codec.
//
// File layout:
//   0    6   Magic: "mhwanh"
//   6    2   Width  (u16 LE)
//   8    6   Unknown header fields
//  14   18   Null padding
//  32  768   Embedded palette: 256 x RGB8 triplets (8-bit, 0-255)
// 800  w*h   Pixel indices, row-major
//
// Height is derived from file size: (size - 800) / width.
// Palette values are full 8-bit (not the 6-bit VGA format used by .PAL/.PIC).

namespace ft {

struct RawInfo {
    uint32_t width;
    uint32_t height;
};

// Parse header. Returns false if data is too short or magic is wrong.
bool raw_info(const uint8_t* data, size_t size, RawInfo* info);

// Decode RAW to a width*height*4 RGBA buffer.
// Returns empty vector on error.
std::vector<uint8_t> raw_decode(const uint8_t* data, size_t size);

} // namespace ft
