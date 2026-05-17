# Research Backlog

Outstanding RE and documentation tasks, grouped by effort.

---

## Binary Analysis (no disassembly tool required)

- **PLT field gap (0xB0–campaign block start)**: Field layout of the pilot save file from offset `0xB0` to the campaign block start is unmapped. Method: diff two pilot saves with known differences (different aircraft, different loadout) byte-by-byte. See [formats/PLT.md](formats/PLT.md).

---

## Win32 PE DLL Disassembly

For each item: load the overlay DLL in Ghidra, import the FA.SMS symbol list to auto-name engine imports, trace from the DLL's exported entry point.

- **FNT glyph format**: Pixel format (1/4/8-bpp?), grid stride, character metrics, baseline offset. See [formats/FNT.md](formats/FNT.md).
- **HUD element layout**: Screen-space coordinate table, gauge types, indicator IDs. See [formats/HUD.md](formats/HUD.md).
- **MUS playlist logic**: Which XMI tracks it sequences, trigger conditions (game state machine), transition logic. See [formats/MUS.md](formats/MUS.md).
- **DLG control positions**: Per-dialog control coordinate table, control type encoding. See [formats/DLG.md](formats/DLG.md).
- **LAY rendering algorithm**: Sky/cloud/ocean draw calls, atmosphere parameters, horizon math. See [formats/LAY.md](formats/LAY.md).

---

## Format Deep Dives (BRF numeric fields)

These fields are documented structurally but their numeric semantics need calibration.

- ~~**SEE range encoding**~~: **Resolved** — 1 unit = 1 foot (6076 units/nm). See [formats/SEE.md](formats/SEE.md).
- ~~**SEE seeker type byte**~~: **Resolved** — 0=visual, 1=laser, 2=IR/EO, 3=radar. See [formats/SEE.md](formats/SEE.md).
- **SEE dual-lobe meaning**: Primary lobe = search mode, secondary = track/lock (partially confirmed from range/angle comparison; switch trigger still needs Ghidra). See [formats/SEE.md](formats/SEE.md).
- **JT PROJ_TYPE fields**: Bits 16–17 of warhead flags confirmed (AA/AG capability); seeker byte enum confirmed; agility/hit-probability bytes still unmapped. See [formats/JT.md](formats/JT.md).
- **ECM effectiveness bytes**: Three variable positions identified (bytes 1, 5, 9 in 9-byte block); five fixed constants identified; roles still need Ghidra. See [formats/ECM.md](formats/ECM.md).
- **GAS capacity word**: Remains undecoded — does not map cleanly to gallons, liters, or known units. Cross-reference `.PT` internal fuel field to derive scale factor. See [formats/GAS.md](formats/GAS.md).
- **OT/NT capability flags**: `obj_class` partially confirmed ($40=scenery, $100=ground structure, $2000=naval). `ot_flags` bit 0 = targetable confirmed; full bit map needs Ghidra. See [formats/OT.md](formats/OT.md), [formats/NT.md](formats/NT.md).

---

## Undocumented Loose Files

- **EA.CFG**: Map all fields by toggling settings in-game and diffing. Cross-reference `CN_ReadConfig` symbol in FA.SMS. See [formats/CFG.md](formats/CFG.md).
- **NET.DAT**: Map multiplayer network config fields. Cross-reference `CN_INFO` struct via FA.SMS. See [formats/NET.md](formats/NET.md).
- **RGN**: Decode `POSTER.RGN` (324 bytes) binary region record structure. See [formats/RGN.md](formats/RGN.md).

---

## Future Inventory

- **FA_3.LIB PIC naming pattern**: Confirm whether the `<AC>_<N>.PIC` N suffix encodes LOD level, paint scheme, or texture region. Method: extract a full aircraft skin set (e.g. all `f22_*.PIC`) using `ft lib unpack`, load each into the GUI PIC viewer, and compare against `f22.SH` UV coordinates. If textures tile differently at distance, suffix = LOD; if they show different colour schemes, suffix = paint scheme.
- **`~` prefix files**: Confirm semantics of `~`-prefixed entries in FA_2.LIB (e.g. `~BNK5.OT`, `~MOOSE.JT`). Method: (1) unpack both `BNK5.OT` and `~BNK5.OT` and diff their BRF content — if only stats differ, they are campaign/theater overrides; if the shape or name differs, they may be runtime aliases for scripted events. (2) Search FA.SMS for symbol names containing `override`, `variant`, or `theater` for engine-side context.
