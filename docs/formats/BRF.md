# BRF -- Brent's Relocatable Format (.OT / .NT / .PT / .JT / .SEE / .ECM / .GAS)

> Format fully documented by the [OpenFA project](https://gitlab.com/openfa/openfa).
> Primary reference: `crates/asset/xt_parse/src/lib.rs`, `crates/asset/ot/src/lib.rs`,
> `crates/asset/pt/src/lib.rs`. Our implementation is in `lib/src/brf.cpp` and
> `lib/src/ot.cpp`.

---

## Overview

BRF is a **plain ASCII text** container for all game type definitions. Seven file
extensions share the same tokenizer; the `struct_type` field distinguishes them.

| Extension | struct_type | Contents |
|-----------|-------------|----------|
| `.OT` | 1 | Object type (generic game object) |
| `.NT` | 3 | NPC type (AI unit / crew) |
| `.PT` | 5 | Plane type (aircraft aerodynamics + avionics) |
| `.JT` | 7 | Jettison type (projectile / weapon physics) |
| `.SEE` | ? | Seeker type (missile guidance) |
| `.ECM` | ? | ECM pod definition |
| `.GAS` | ? | Gas / smoke type |

---

## File Structure

```
[brent's_relocatable_format]

:<ptr_name1>
:<ptr_name2>
\tend

<kind> <value>
<kind> <value>
...
\tend

[<section_name>]
<kind> <value>
...
\tend
```

### Tokens

| Kind | Value syntax | C++ type |
|------|-------------|---------|
| `byte` | decimal integer | `uint8_t` |
| `word` | decimal integer | `uint16_t` / `int16_t` |
| `dword` | decimal integer | `uint32_t` / `int32_t` |
| `ptr` | `"filename"` or `NULL` | `std::string` (may be empty) |
| `symbol` | `NAME` | `std::string` |
| `string` | `"text"` | `std::string` |

`\tend` terminates each block. The pointer table (`:name` lines) comes first, then the
main field block, then optional named subsections.

---

## OT Fields (Object Type)

OT versioning is determined by field count: V0=49, V1=51, V2=63, V3=64.

Key fields (abridged — full list in `crates/asset/ot/src/lib.rs`):

```
struct_type         byte    1=OT, 3=NT, 5=PT, 7=JT
type_size           word
instance_size       word
short_name          ptr     e.g. "F-16C"
long_name           ptr     e.g. "General Dynamics F-16C Fighting Falcon"
file_name           ptr     e.g. "F16C"
ot_flags            dword
obj_class           word    0x8000=Fighter, 0x4000=Bomber, 0x2000=Ship, ...
shape               ptr     3D model filename (no extension)
shadow_shape        ptr
max_vis_dist        word    feet
camera_dist         word
hit_points          word
dmg_planes          word    damage dealt to each target type
dmg_ships           word
dmg_structs         word
dmg_armor           word
dmg_other           word
explosion_type      byte
crater_size         byte
empty_weight        dword   pounds
; V2+ adds: dmg_debris_pos i16[3], dst_debris_pos i16[3], dmg_type dword
; V3+ adds: year_available dword
```

---

## PT Fields (Plane Type)

PT extends OT with ~80 additional aerodynamic and avionics fields, including:

```
thrust              dword   lbf (afterburner)
thrust_dry          dword   lbf (military power)
fuel_capacity       dword   pounds
max_speed           word    mph
stall_speed         word    mph
service_ceiling     dword   feet
turn_rate           word    deg/sec × 10
; ... many more: envelope tables, weapon hardpoints, radar params, etc.
```

Full field list: `crates/asset/pt/src/lib.rs`.

---

## Round-Trip Notes

- Parse → serialize produces byte-identical files for all OT/NT/PT files in FA_2.LIB.
- Null pointers are written as `ptr NULL`.
- Integer field sign interpretation follows OpenFA's type assignments; wrong signedness
  produces visually wrong values in `info` output.

---

## ft commands

```
# Same pattern for all seven extensions:
ft ot  info   <file.OT>              # human-readable field dump
ft ot  unpack <file.OT>  [-o out.txt] # editable text
ft ot  pack   <in.txt>   -o out.OT   # write back

ft nt  info / unpack / pack
ft pt  info / unpack / pack
ft jt  info / unpack / pack
ft see info / unpack / pack
ft ecm info / unpack / pack
ft gas info / unpack / pack
```

Example: `ft pt info F16C.PT` → thrust, max_speed, fuel, stall speed, ceiling.
