# Research Backlog

Outstanding RE and documentation tasks, grouped by effort.

---

## Binary Analysis (no disassembly tool required)

- **PLT field gap (0xB0–campaign block start)**: Field layout from offset `0xB0` to the campaign block start is unmapped. `PilotSave(PILOT*, short)` confirmed at 0x467180 — decompile in Ghidra (with FA.SMS labels applied) to map the full `PILOT` struct and confirm all field offsets directly. Alternatively: diff two pilot saves with known differences (aircraft, loadout) byte-by-byte. See [formats/PLT.md](formats/PLT.md). *(Requires gameplay — 4-pass methodology documented in PLT.md)*

---

## Win32 PE DLL Disassembly (FA.EXE targets)

Overlay DLL Ghidra projects already imported under `%FA_PROJECT%\overlay_projects\`. FA.EXE project at `%FA_PROJECT%\fa-re.gpr` has all 3,829 FA.SMS symbols applied. Use the Ghidra GUI for manual tracing — the static VA scanner scripts cannot resolve the items below (see blockers).

- **HUD damage overlay (bits 0–4, 28–31)**: Bits 0–4 are written by `_DAMAGEDoHit@12`; bits 28–31 written by the same path. The display function that *reads* these bits (damage overlay / cockpit warning lights) has not been identified. `FUN_00407930` (advisory icon renderer) confirmed NOT the reader — it only handles bits 6–11. *Blocker: no constant `0x1F` or `0xF0000000` mask found in automated scan; the reader likely tests individual bits inline.* Manual trace from `_DAMAGEDoHit@12` call sites into the render loop required. See [formats/HUD.md](formats/HUD.md).

- **HGR hangar slot structure**: `H_AIRB.HGR` (land base) and `H_AIRB2.HGR` (carrier). Loader `FUN_004543c0` confirmed: 30 × 8-byte slot entries at offset +0x2D; X/Y arrays built at load. Remaining: decode the 8-byte entry (icon position, angle, camera?) and trace `FUN_004a6cc0(pcVar5, 0x8104)` sub-resource key. *Blocker: no Ghidra analysis script was written targeting HGR — open the HGR Ghidra project (`overlay_projects\hud\` or FA.EXE) and trace from `FUN_004543c0`.* See [formats/HGR.md](formats/HGR.md).

- **NT `ot_flags` bits 18–20, 25–26**: Labels "arrestor-wire/catapult/VSTOL deck" (18–20) and "emplaced AA / SA-2A" (25–26) are inferred from carrier and AA category patterns only. *Blocker: no dedicated test function exists for these bits — they are tested inline inside larger functions (e.g. carrier landing handler). `AnalyzeOTNT.java` confirmed this. Requires manually reading the containing functions in the Ghidra GUI.* See [formats/NT.md](formats/NT.md).

- **MM `obj flags` bits 9 and 10**: Bit 9 (mission-critical?) and bit 10 (friendly vs hostile ownership?) semantics unconfirmed. *Blocker: inline-only tests; no helper function to target via script. Trace `_MISSIONTextProc@16` (`FUN_00481c10`) obj handler in FA.EXE Ghidra project.* See [formats/MM.md](formats/MM.md).

---

## Overlay DLL Disassembly (must use overlay Ghidra projects, not FA.EXE scripts)

These items require opening the relevant format's Ghidra project under `%FA_PROJECT%\overlay_projects\`. Running FA.EXE analysis scripts against them will not work — the logic does not exist in FA.EXE's address space.

- **CAM binary layout**: Mission state tables and weapon lists are inside the `.CAM` PE DLLs, not FA.EXE. The FA.EXE caller chain is confirmed (`FUN_00428412` → `_CallMissionProc_8`), but that only shows how the DLL is invoked. *Approach: open `overlay_projects\cam\` in Ghidra GUI, load `UKRAINE.CAM`, trace from the DLL's exported entry point to map the data section layout (offsets, sizes, field encoding). Identify which `.MC` files each campaign loads via `_mc_M`/`_mc_nato_M` naming.* See [formats/CAM.md](formats/CAM.md).

- **MC condition check logic**: `.MC` per-mission condition files are PE DLLs. The FA.EXE dispatch chain is confirmed (main loop → `FUN_00495e80` → DLL export), but the condition logic itself is inside the DLL. *Approach: open `overlay_projects\mc\` in Ghidra GUI, load `UKR01.MC`, trace from the exported entry point through all condition checks and object alias lookups. Also trace `FUN_00495e80` (`.MC` string handler) to clarify its role. Clarify `EXTRA01.MC` purpose (multiplayer extra?).* See [formats/MC.md](formats/MC.md).

- **T2 surface class → PIC atlas mapping and tile-summary algorithm**: T2 loading code is in the T2 terrain overlay DLL — confirmed absent from FA.EXE. Entry point `@T_Load@4` (0x4c5d70) and remap handler `FUN_004d3064` are FA.EXE call sites into the DLL, not the implementation. Sub-header bytes 4–16 class constants and tile-summary record 0 selection algorithm also open. *Approach: open the T2 terrain overlay Ghidra project (likely under `overlay_projects\`), locate the `BIT2` magic handler, trace `FUN_004d3064` to map surface class byte → PIC atlas row/column. Trace the sub-header parser — constant `0x95` found in `FUN_0043faf0`, `FUN_004672c0`, `FUN_0046eedf`, `FUN_00480230` — these are the candidate entry points.* See [formats/T2.md](formats/T2.md).

---

## Mission System Formats

- **AI / BI opcode 0x28 FRAME consumer**: FRAME writes two s16 values to `DAT_00546c44`/`DAT_00546c46`. No reader found via address-based scan. *Blocker: the consumer accesses these globals through a pointer to the surrounding interpreter state block, not by direct address — xref-by-address scanning misses it. Approach: in the FA.EXE Ghidra project, find the struct containing `DAT_00546c44` at its known offset, then find all functions that load a pointer to that struct and read the field by offset.* Candidate consumers from offset scan: `_INFO2Draw`, `_FMFlight@0`, `_MANAdd@24`, `_GVDoCurrentWaypoint`, `?MPStatusSet@@YIXJ@Z`, `FUN_0048e740`. See [formats/AI.md](formats/AI.md) and [formats/BI.md](formats/BI.md).

- **MM `tdic id=256`**: Whether tile-type index 256 is a sentinel, default, or T2 reference is unconfirmed. Trace `tdic` keyword handler in `FUN_00481c10`. See [formats/MM.md](formats/MM.md).

---

## Format Deep Dives (BRF Numeric Fields)

- **JT physics gap bytes (+0x50–0x6E, 30 bytes)**: Turn rate, g-limit, and other flight-model params unresolved. *Blocker: `_PROJProc` is called through a virtual function table slot — no static CALL to a known VA exists. Static address-range scanning (`DumpPROJPhysics3`, `DumpPROJDispatch`) found nothing. Approach: in Ghidra GUI, find a known missile entity instance, follow its vtable to `_PROJProc`, then trace offset reads in the 0x50–0x6E range.* See [formats/JT.md](formats/JT.md).

- **JT warhead flags bits 1–3, 5–6**: All other warhead flag bits confirmed. No function found testing these bits from `missile+0xa6`. Likely structural/unused. Low priority — may simply be unset in all shipped `.JT` files. See [formats/JT.md](formats/JT.md).

- **Entity runtime flags bits 17 and 21**: Bit 17 acts as "low fuel" trigger when bit 21 is clear; bit 21 inhibits the "running on fumes" voice line. Full semantic scope across all entity types (not just player aircraft) not confirmed. See [formats/JT.md](formats/JT.md).

---

## Undocumented Loose Files

- **EA.CFG**: Map all fields by toggling settings in-game and diffing. `CN_ReadConfig` and `CN_WriteConfig` confirmed in FA.SMS (`void CN_ReadConfig(CN_INFO*, unsigned char*)` / `void CN_WriteConfig(CN_INFO*, unsigned char*)`). Decompile both in Ghidra to map the `CN_INFO` struct. See [formats/CFG.md](formats/CFG.md).

- **NET.DAT**: Map multiplayer network config fields. Cross-reference `CN_INFO` struct via FA.SMS. Confirm whether NET.DAT holds one transport block or a union of all transport configs. See [formats/NET.md](formats/NET.md).

---

## Implementation (RE complete — ready to build)

Formats whose structure is fully or sufficiently documented to implement. All need a lib parser and a `ft <fmt>` CLI command unless noted.

### Trivial (existing lib, just needs CLI wiring)

- **`ft pal` CLI command**: `pal.cpp` / `pal.h` already exist in lib. No CLI command is wired up. Add `ft pal info <file>` and `ft pal unpack <file>` to expose palette dump and PNG export.

### Low effort (format fully documented, straightforward parsing)

- **INF parser**: Dot-command markup (`.body`, `.title`, `.center`, `.left`) plus `LENGTH`/`HEIGHT`/`WINGSPAN`/`WEIGHT`/`PERFORMANCE` footer key-values. Implement text parser → `ft inf unpack <file>` outputs structured JSON. See [formats/INF.md](formats/INF.md).

- **HUD data-section parser** *(distinct from HUD advisory bits RE task above)*: Fixed 0x2BB CODE-section layout confirmed; gauge parameter offsets and anchor-point coordinates documented. Implement PE section reader → decode gauge table → `ft hud dump <file>` outputs `{aircraft, gauges: [{name, x, y}]}`. See [formats/HUD.md](formats/HUD.md).

### Medium effort (PE DLL reader required, structs known)

- **LAY parser**: LAYER struct (fog_density, color_entry_ptr, vis_lo/vis_hi, fog_alt_low/alt_high) and 30-dword header block confirmed. Implement PE data-section extractor → decode LAYER array → `ft lay dump <file>` exports sky/atmosphere parameters. See [formats/LAY.md](formats/LAY.md).

- **FNT glyph extractor**: FONT struct (font_height dword + 256 glyph function pointers + 256 width table) fully mapped; x86 glyph execution semantics (ADD EDI,ECX / MOV [EDI],AL) documented. Implement PE section parser → extract glyph metrics → `ft fnt unpack <file>` exports metrics CSV and per-glyph PNGs. See [formats/FNT.md](formats/FNT.md).

- **MUS bytecode disassembler**: All opcodes documented (FF playlist ID, FA setup, FB play XMI, FE conditional, FD loop/jump, FC shuffle); game-state IDs confirmed. Implement PE CODE-section reader → decode opcode sequence → `ft mus dump <file>` exports human-readable script. See [formats/MUS.md](formats/MUS.md).
