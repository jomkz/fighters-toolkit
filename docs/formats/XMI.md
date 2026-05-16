# Extended MIDI Audio (.XMI)

FA_2.LIB contains 78 `.XMI` files. XMI is Miles Sound System's Extended MIDI format — a compact variant of Standard MIDI that supports multiple tracks within a single file and was widely used in DOS-era games.

## Format

XMI is a well-documented format produced by The Audio Interface Library (AIL) / Miles Sound System. Key characteristics:

- Begins with an IFF-style RIFF chunk header: `FORM` + size + `XDIR`
- Contains one or more `FORM XMID` chunks, each an independent MIDI sequence
- Tempo and timing differ from Standard MIDI — XMI uses a fixed 120 BPM base with AIL-specific delta encoding

Conversion to Standard MIDI (`.MID`) is possible with tools such as `xmi2mid` from the WildMIDI project.

## Location

| LIB | Count |
|-----|-------|
| FA_2.LIB | 78 |

## Related

- [MUS.md](MUS.md) — `.MUS` music files, relationship to `.XMI` not yet confirmed
- [AUDIO.md](AUDIO.md) — PCM audio formats (`.5K`, `.11K`) used for sound effects and voice
