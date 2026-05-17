# Heads-Up Display (.HUD)

FA_2.LIB contains 46 `.HUD` files — one per aircraft type (e.g. `A7.HUD`, `F22.HUD`). Each defines the cockpit HUD layout for that aircraft. Each is a **Win32 PE DLL** loaded at runtime via `LoadLibrary`.

## Format

Win32 PE DLL. All observed `.HUD` files decompressed to **4608 bytes**.

## Content

String analysis of `F22.HUD` and `B2.HUD` reveals the asset reference pattern:

| String | Role |
|--------|------|
| `~f22` / `~b2` | Aircraft 3D model reference |
| `~f22h` / `~b2h` | HUD overlay image (heads-up display graphic) |
| `~f22s` / `~b2s` | HUD symbol set |
| `hudsym` | HUD symbol font (`HUDSYM*.FNT`) |
| `GEAR`, `FLAP`, `BRAKE` | Indicator label strings |
| `~f22_p` / `~b2_p` | Aircraft propulsion/engine panel reference |
| `~f22_w` / `~b2_w` | Weapons panel reference |
| `winfont` | Window font (`WIN*.FNT`) reference |

The `~` prefix indicates LIB-resident asset references. The HUD DLL binds its aircraft-specific assets at load time using these names.

## Location

| LIB | Count |
|-----|-------|
| FA_2.LIB | 46 |

## CODE Section Layout (Confirmed)

HUD files use **Phar Lap PE format** (signature `PL\0\0`). The CODE section is a **pure data structure** — no imports, no dispatch table, no x86 code. The engine loads HUD assets by name at runtime.

All HUD files have identical CODE virtual size (`0x2BB` = 699 bytes), confirming a fixed-size struct regardless of aircraft.

### Loading mechanism

`FUN_00406040` (HUD init, called at aircraft load time):
1. Loads the HUD DLL by name via `FUN_004a6ae0`
2. Bulk-copies the entire CODE section (0xAC dwords + 2 bytes = **690 bytes**) to `DAT_00521360`
3. Scales every gauge parameter left by the display xscale/yscale factor (`bVar3`/`bVar4`)

The anchor point (`DAT_00521d94`/`DAT_00521d96`) is **not** read directly from the HUD file — it is computed dynamically at runtime via a smooth-follow interpolation that tracks the player aircraft's screen position.

### String layout (A7.HUD)

| VA | String | Role |
|----|--------|------|
| 00001001 | `~a7` | Aircraft model base name |
| 0000100E | `~a7h` | High-AoA/hook variant |
| 0000101B | `~a7s` | Speed-brake variant |
| 00001038 | `hud` | HUD identifier |
| 00001051 | `hudsym` | HUD symbol set |
| 0000113C–000011A4 | `~a7_l/c/r`, `~a7_lh/ch/rh`, `~a7_ls/cs/rs` | Left/centre/right sub-panel states |
| 00001245 | `GEAR`, `FLAP`, `BRAKE`, `HOOK` | Warning indicator labels |
| 00001275 | `~a7_p`, `~a7_w` | Engine/weapons panel sprites |
| 00001297 | `winfont` | Window font reference |

### Sub-panel sprite suffix semantics (confirmed)

| Suffix pattern | Meaning |
|----------------|---------|
| `_l` / `_c` / `_r` | Left / centre / right panel — normal state |
| `_lh` / `_ch` / `_rh` | Left / centre / right panel — high-AoA state |
| `_ls` / `_cs` / `_rs` | Left / centre / right panel — stowed/small state |

F22.HUD omits all `_l/c/r` sprites entirely — the F22 has no separate sub-panels. It also uses `BAY` (weapons bay indicator) instead of `HOOK`.

### Gauge parameter layout (confirmed)

All gauge positions are stored as signed s16 offsets from the HUD anchor point. Confirmed by tracing the HUD draw functions (`FUN_00406040`, `?HUDDrawHeading`, `?HUDDrawSpeed`, `?HUDDrawAlt`, `?HUDDrawHVel`, `?HUDDrawWeaponInfo`, `?HUDDrawRangeInfo`) in FA.EXE via Ghidra.

After loading, the struct is resident at `DAT_00521360`. Field offsets within the copied struct:

| Struct offset | Global | Gauge | Field |
|---------------|--------|-------|-------|
| `0x1E1` | `DAT_00521541` | Heading tape | width (pixels) |
| `0x1E5` | `DAT_00521545` | Heading tape | dy from anchor |
| `0x1ED` | `DAT_0052154d` | Heading tape | tick spacing (dy) |
| `0x1F7` | `DAT_00521557` | Speed tape | dx from anchor |
| `0x1FB` | `DAT_0052155b` | Speed tape | dy from anchor |
| `0x1FF` | `DAT_0052155f` | Speed tape | height (pixels) |
| `0x209` | `DAT_00521569` | Speed tape | tick increment |
| `0x214` | `DAT_00521574` | Altitude tape | dx from anchor |
| `0x218` | `DAT_00521578` | Altitude tape | dy from anchor |
| `0x21C` | `DAT_0052157c` | Altitude tape | height (pixels) |
| `0x226` | `DAT_00521586` | Altitude tape | tick increment |
| `0x231` | `DAT_00521591` | Flight path marker | dx from anchor |
| `0x233` | `DAT_00521593` | Flight path marker | dy from anchor |
| `0x235` | `DAT_00521595` | Flight path marker | box half-width |
| `0x237` | `DAT_00521597` | Flight path marker | box half-height |
| `0x265` | `DAT_005215c5` | (unresolved) | dx |
| `0x267` | `DAT_005215c7` | (unresolved) | dy |
| `0x269` | `DAT_005215c9` | (unresolved) | dx |
| `0x26B` | `DAT_005215cb` | (unresolved) | dy |
| `0x26D` | `DAT_005215cd` | Weapon info | dx from anchor |
| `0x26F` | `DAT_005215cf` | Weapon info | dy from anchor |
| `0x271` | `DAT_005215d1` | Range info | dx from anchor |
| `0x273` | `DAT_005215d3` | Range info | dy from anchor |

## Toolkit Roadmap

- New `lib/src/hud.cpp` + `lib/include/ft/hud.h` — parse sprite name table and gauge parameter block
- New `cli/cmd_hud.cpp` — `ft hud dump <file.HUD>` prints gauge table as JSON `[{gauge, dx, dy}]`
- GUI: overlay viewer that renders gauge positions on a 640×480 canvas

## TODO

- Confirm exact struct byte offsets for each field by hex-diffing A7.HUD vs F22.HUD (the offsets in the table above are derived from global address arithmetic and should be verified against raw file bytes)
- Identify the four unresolved `(0x265–0x26B)` gauge coordinate pairs
- Confirm per-aircraft anchor point source (the `0x10 0x10` init values in `FUN_00406040` suggest a default; actual per-aircraft position may come from the PT file or a separate config)

## Related

- [BRF.md](BRF.md) — `.PT` aircraft type records that likely reference the corresponding `.HUD`
- [FNT.md](FNT.md) — font files used to render HUD text elements
- [PIC.md](PIC.md) — bitmap assets used for HUD graphical elements
