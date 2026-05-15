# M / MM -- Mission and Map Files (.M / .MM)

> Format documented by the [OpenFA project](https://gitlab.com/openfa/openfa).
> Primary reference: `crates/asset/mmm/`. Our implementation is in `lib/src/mission.cpp`.

---

## Overview

`.M` files define individual missions; `.MM` files define map/theater data. Both use
the same `[textFormat]` container with `[key value]`-style bracketed tokens.

---

## File Structure

```
[textFormat]
[brief]
[mapName BALTICMAP]
[layer BALTIC 0]
[time 6 0]
[wind 270 15]
[clouds 20]
[sides 0 1 0 1 0 0 0 0]
[objects
    [pos 12345.0 0.0 67890.0]
    [id {GUID-or-name}]
    [type FIGHTER]
    [side 0]
    ...
\t]
```

### Token format

- `[key value]` — single-line key-value pair; value may be a string, number, or space-
  separated list
- `[key\n ... \t]` — bracketed block: contents terminated by `\t]` on its own line
- Blocks may be nested

### Top-level keys (.M)

| Key | Value | Description |
|-----|-------|-------------|
| `brief` | (no value) | Mission has a briefing |
| `mapName` | string | Theater map identifier |
| `layer` | name index | Terrain layer |
| `time` | hour min | Start time of day |
| `wind` | dir_deg speed | Wind conditions |
| `clouds` | percent | Cloud cover 0–100 |
| `sides` | 8 integers | Team assignments for each side slot |
| `objects` | block | Object placement list |

### Object block fields

| Key | Value |
|-----|-------|
| `pos` | x y z (feet) |
| `id` | unique identifier string |
| `type` | object type name (references .OT/.PT files) |
| `side` | integer team index |
| `heading` | degrees |
| `speed` | initial speed |
| `alt` | initial altitude (feet) |

Field list varies by object type; unknown fields are preserved verbatim on round-trip.

---

## Round-Trip Notes

- Parse → serialize produces byte-identical files for all 592 M/MM files in FA_2.LIB.
- Tab-indented blocks must use a real tab character, not spaces.
- The `\t]` terminator is literally `<TAB>]` (not backslash-t).

---

## ft commands

```
ft mission info   <file.M|.MM>              # map name, time, object count
ft mission unpack <file.M|.MM> [-o out.txt] # editable text
ft mission pack   <in.txt>     -o out.M     # write back

ft mm     info   <file.MM>
ft mm     unpack <file.MM> [-o out.mm.txt]
ft mm     pack   <in.txt>  -o out.MM
```
