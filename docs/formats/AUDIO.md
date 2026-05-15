# Audio -- Raw PCM (.11K / .5K)

> Format identified from FA audio file analysis. Sample rate confirmed against the
> [OpenFA project](https://gitlab.com/openfa/openfa).

---

## Overview

FA audio files are raw, headerless, signed 8-bit mono PCM. The sample rate is encoded
in the file extension:

| Extension | Sample rate |
|-----------|------------|
| `.11K` | 11025 Hz |
| `.5K` | 5512 Hz |

Files whose names begin with `&` (e.g. `&AFTB2.11K`) are looping sounds — this is an
engine convention, not a format difference. `ft lib unpack` maps `&` to `_` on extraction
because Windows rejects `&` in filenames.

---

## PCM Encoding

- Signed 8-bit (`int8_t`), range −128..127
- Mono (1 channel)
- No header, no footer — the file is raw samples from byte 0

---

## WAV Conversion

WAV stores 8-bit audio as **unsigned** (0..255), not signed. Apply `^ 0x80` on both
directions:

```c
// FA raw → WAV sample
wav_byte = (uint8_t)((int8_t)fa_byte + 128);   // equivalently: fa_byte ^ 0x80

// WAV sample → FA raw
fa_byte = (int8_t)(wav_byte - 128);
```

WAV header for 11025 Hz, mono, 8-bit:

```
RIFF chunk:  "RIFF" + (file_size - 8) u32LE + "WAVE"
fmt  chunk:  "fmt " + 16 u32LE + 1 u16LE (PCM) + 1 u16LE (channels)
             + 11025 u32LE (sample rate) + 11025 u32LE (byte rate)
             + 1 u16LE (block align) + 8 u16LE (bits per sample)
data chunk:  "data" + sample_count u32LE + [samples]
```

---

## ft commands

```
ft audio info   <file.11K|.5K>              # sample rate, sample count, duration
ft audio unpack <file.11K|.5K> [-o out.wav] # raw PCM → WAV
ft audio pack   <in.wav> -o <out.11K|.5K>   # WAV → raw PCM
                          [-r 11025]         # override sample rate (default from ext)
```

The output extension determines the stored sample rate when packing.
Input WAV must be mono, 8-bit; `ft` rejects stereo or 16-bit input.
