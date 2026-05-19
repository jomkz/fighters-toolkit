# Research Backlog

Outstanding RE and documentation tasks, grouped by effort.

---

## Binary Analysis (no disassembly tool required)

- **PLT field gap (0xB0–campaign block start)**: Field layout from offset `0xB0` to the campaign block start is unmapped. `PilotSave(PILOT*, short)` confirmed at 0x467180 — decompile in Ghidra (with FA.SMS labels applied) to map the full `PILOT` struct and confirm all field offsets directly. Alternatively: diff two pilot saves with known differences (aircraft, loadout) byte-by-byte. See [formats/PLT.md](formats/PLT.md). *(Requires gameplay — 4-pass methodology documented in PLT.md)*

---

## Win32 PE DLL Disassembly (FA.EXE targets)

Overlay DLL Ghidra projects already imported under `%FA_PROJECT%\overlay_projects\`. FA.EXE project at `%FA_PROJECT%\fa-re.gpr` has all 3,829 FA.SMS symbols applied. Use the Ghidra GUI for manual tracing — the static VA scanner scripts cannot resolve the items below (see blockers).

- ~~**HUD damage overlay (bits 0–4)**~~ **RESOLVED (2026-05-19):** Bits 0–4 are **operational inhibitor flags**, not display bits. There is no separate damage-overlay display function reading them. When set, they suppress system operations in the FM actuator functions: bit 2 (0x4) gates `@FMBrakes@8` (brakes silently disabled when damaged); other bits gate analogous subsystems via GAS damage events. Set by `_DAMAGEDoHit@12` event cases 0x50d3f7–0x50d40c. See [formats/HUD.md](formats/HUD.md).

  **Bits 28–31 reader still open:** Bit 30 (0x40000000) confirmed read by the carrier HGR hangar renderer (`AnalyzeHGR.txt` lines 4143/4315 — checks this bit to modify a display parameter in the hangar slot view). Full display consumer for all four bits not yet identified.

- **HGR hangar slot structure**: `H_AIRB.HGR` (land base) and `H_AIRB2.HGR` (carrier). Loader `FUN_004543c0` confirmed: 30 × 8-byte slot entries at offset +0x2D; X/Y arrays built at load. Remaining: decode the 8-byte entry (icon position, angle, camera?) and trace `FUN_004a6cc0(pcVar5, 0x8104)` sub-resource key. *Blocker: no Ghidra analysis script was written targeting HGR — open the HGR Ghidra project (`overlay_projects\hud\` or FA.EXE) and trace from `FUN_004543c0`.* See [formats/HGR.md](formats/HGR.md).

- **NT `ot_flags` bits 18–20, 25–26**: Labels "arrestor-wire/catapult/VSTOL deck" (18–20) and "emplaced AA / SA-2A" (25–26) are inferred from carrier and AA category patterns only. *Blocker: no dedicated test function exists for these bits — they are tested inline inside larger functions (e.g. carrier landing handler). `AnalyzeOTNT.java` confirmed this. Requires manually reading the containing functions in the Ghidra GUI.* See [formats/NT.md](formats/NT.md).

- **MM `obj flags` bits 9 and 10**: Bit 9 (mission-critical?) and bit 10 (friendly vs hostile ownership?) semantics unconfirmed. *Blocker: inline-only tests; no helper function to target via script. Trace `_MISSIONTextProc@16` (`FUN_00481c10`) obj handler in FA.EXE Ghidra project.* See [formats/MM.md](formats/MM.md).

---

## Overlay DLL Disassembly (must use overlay Ghidra projects, not FA.EXE scripts)

These items require opening the relevant format's Ghidra project under `%FA_PROJECT%\overlay_projects\`. Running FA.EXE analysis scripts against them will not work — the logic does not exist in FA.EXE's address space.

- ~~**CAM binary layout**~~ **RESOLVED (2026-05-18):** KURILE.CAM analyzed via `AnalyzeCAMDLL.java`. Dispatch function at PE offset 0x1000, command protocol 0x00–0x08 mapped, `.idata` import table extracted (all FA.EXE callbacks the DLL calls). Mission list string table at `DAT_000014e1`. See [formats/CAM.md](formats/CAM.md) and `%FA_PROJECT%/output/AnalyzeCAMDLL.txt`.

  *Remaining:* Weapon/aircraft table binary field encoding not yet decoded (needs deeper data-section walk). `FUN_00428340` role unconfirmed.

- ~~**MC condition check logic**~~ **RESOLVED (2026-05-18):** U34.MC analyzed via `AnalyzeMCDLL.java`. Condition function signature confirmed: `short FUN_00001000(short, undefined4, undefined4, short*)`. Protocol: `*param_4 == 0x00` evaluates condition (reads `DAT_00001212`/`DAT_00001211`); `*param_4 == 0x20` returns pass-through state. Data layout at PE offsets 0x1211–0x1212. See [formats/MC.md](formats/MC.md) and `%FA_PROJECT%/output/AnalyzeMCDLL.txt`.

  *Remaining:* `FUN_00495e80` role still unconfirmed. `EXTRA01.MC` purpose **RESOLVED (2026-05-18):** generic bonus-mission condition gate, shared by `EXTRA01.M`–`EXTRA20.M` and `BEXTRA01.M`–`BEXTRA13.M` via `code extra01` directive in each `.M` file. See [formats/MC.md](formats/MC.md).

- **T2 surface class → PIC atlas mapping and tile-summary algorithm**: T2 loading code is in the T2 terrain overlay DLL — confirmed absent from FA.EXE. Entry point `@T_Load@4` (0x4c5d70) and remap handler `FUN_004d3064` are FA.EXE call sites into the DLL, not the implementation. Sub-header bytes 4–16 class constants and tile-summary record 0 selection algorithm also open. *Approach: open the T2 terrain overlay Ghidra project (likely under `overlay_projects\`), locate the `BIT2` magic handler, trace `FUN_004d3064` to map surface class byte → PIC atlas row/column. Trace the sub-header parser — constant `0x95` found in `FUN_0043faf0`, `FUN_004672c0`, `FUN_0046eedf`, `FUN_00480230` — these are the candidate entry points.* See [formats/T2.md](formats/T2.md).

---

## Mission System Formats

- **AI / BI opcode 0x28 FRAME consumer**: FRAME writes two s16 values to `DAT_00546c44`/`DAT_00546c46`. No reader found via address-based scan. *Blocker: the consumer accesses these globals through a pointer to the surrounding interpreter state block, not by direct address — xref-by-address scanning misses it. Approach: in the FA.EXE Ghidra project, find the struct containing `DAT_00546c44` at its known offset, then find all functions that load a pointer to that struct and read the field by offset.* Candidate consumers from offset scan: `_INFO2Draw`, `_FMFlight@0`, `_MANAdd@24`, `_GVDoCurrentWaypoint`, `?MPStatusSet@@YIXJ@Z`, `FUN_0048e740`. See [formats/AI.md](formats/AI.md) and [formats/BI.md](formats/BI.md).

  **Update (2026-05-18):** Writer confirmed — it's case 0x28 in the BI interpreter dispatch function (large switch statement on the bytecode byte, contains opcodes 0x01–0x28+). The interpreter global `DAT_00546bea` is the bytecode pointer. Reader of `DAT_00546c44`/`0546c46` not found in full DumpAllFunctions decompile — confirmed indirect access via struct pointer. Still requires manual Ghidra GUI trace.

- ~~**MM `tdic id=256`**: Whether tile-type index 256 is a sentinel, default, or T2 reference is unconfirmed.~~ **RESOLVED (2026-05-18):** 256 (`0x100`) is the type tag for `tmap_named` entries in the terrain dictionary — written explicitly by the `tmap_named` keyword handler when adding a named terrain reference to `_tdic`. It is NOT a T2 reference; it distinguishes named-tile entries from indexed-tile entries in the tile dictionary. See [formats/MM.md](formats/MM.md).

---

## Format Deep Dives (BRF Numeric Fields)

- **JT physics gap bytes (+0x50–0x6E, 30 bytes)**: Turn rate, g-limit, and other flight-model params unresolved. *Blocker: `_PROJProc` is called through a virtual function table slot — no static CALL to a known VA exists. Static address-range scanning (`DumpPROJPhysics3`, `DumpPROJDispatch`) found nothing. Approach: in Ghidra GUI, find a known missile entity instance, follow its vtable to `_PROJProc`, then trace offset reads in the 0x50–0x6E range.* See [formats/JT.md](formats/JT.md).

- ~~**JT warhead flags bits 1–3, 5–6**~~ **RESOLVED (2026-05-18):** Complete scan of all 135 `.JT` warhead dwords (34 unique values) confirms consistent weapon-category patterns. Bit 1 = missiles/rockets; bit 2 = missiles/rockets/guns; bit 3 = guided+guns; bit 5 = heavy standoff only; bit 6 = everything except gravity bombs. No test function found in full decompile — these are structural category metadata, not runtime-dispatched flags. See [formats/JT.md](formats/JT.md).

- ~~**Entity runtime flags bits 17 and 21**~~ **RESOLVED (2026-05-19):** These are fuel warning flags in `DAT_0050cfef` (player aircraft HUD state word only — not applicable to all entity types). Bit 17 (0x20000) = running-on-fumes threshold trigger; bit 21 (0x200000) = fumes voice line already played (inhibit). Full fuel warning system mapped from `@SAYLowFuelMessage@8` decompile: bits 15–22 cover Bingo/Joker/fumes/out-of-fuel triggers (15–18) and per-level voice inhibits (19–22). See [formats/HUD.md](formats/HUD.md).

- **Entity runtime flags bits 17 and 21**: Bit 17 (`0x20000`) is toggled as `_gamePrefs` bit at menu option 0x606; also at `DAT_0050ce81` — acts as "low fuel" trigger when bit 21 is clear. Bit 21 (`0x200000`) toggles at menu option 0x616; inhibit flag for "running on fumes" voice line + 0x380000 state. These are runtime entity flags (not `ot_flags`); their full semantic scope across all entity types needs confirmation.

---

## Undocumented Loose Files

- ~~**IP.CFG**~~ **RESOLVED (2026-05-18):** Plain text, two CRLF lines: `/s` (server/standalone mode) and `/n="Fighters Anthology"` (session name). Read by `IP.EXE` at startup as default session parameters. Fully documented in [formats/CFG.md](formats/CFG.md). IP.EXE itself analyzed via `DumpOverlayDLL.java` → `Overlay_IP.EXE.txt`.

- ~~**EA.CFG**~~ **RESOLVED (2026-05-19):** Complete 347-byte struct layout mapped from `?UCONFIG_save_EA_CFG@@YGDXZ` (0x004b2980) decompile — no gameplay diff required. Fields include: joystick/rudder/throttle device indices, 48-dword axis mapping table, window modes, 10 volume sliders, MIDI device, `_gamePrefs`/`_gameMultiPrefs`/`_gameDebugPrefs` dwords, HUD brightness, campaign pilot name, 3D glasses amount. See [formats/CFG.md](formats/CFG.md). **Note:** CN_ReadConfig/CN_WriteConfig handle NET.DAT (3552 bytes), not EA.CFG.

- ~~**NET.DAT**~~ **RESOLVED (2026-05-19):** `CN_ReadConfig` reads 4-byte checksum + 0xDDC bytes = 3,552 bytes total. NET.DAT is a serialized CN_INFO struct prefixed by a 4-byte CRC (`_CfigChecksum`). Single CN_INFO instance; transport type selector at +0x00. Confirmed in [NETWORK.md](NETWORK.md) and [formats/CFG.md](formats/CFG.md).

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
