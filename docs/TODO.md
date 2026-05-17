# Research Backlog

Outstanding RE and documentation tasks, grouped by effort.

---

## Binary Analysis (no disassembly tool required)

- **PLT field gap (0xB0–campaign block start)**: Field layout from offset `0xB0` to the campaign block start is unmapped. Method: diff two pilot saves with known differences (aircraft, loadout) byte-by-byte. See [formats/PLT.md](formats/PLT.md). *(Requires gameplay — 4-pass methodology documented in PLT.md)*

- **FA_3.LIB PIC naming pattern**: Confirm whether the `<AC>_<N>.PIC` N suffix encodes LOD level, paint scheme, or texture region. Requires disc 2 (FA_3.LIB not on disc 1 or the hard drive install). Method: extract a full aircraft skin set, load each into GUI PIC viewer, compare against SH UV coordinates. See [formats/PIC.md](formats/PIC.md).

- **RGN**: Decode `POSTER.RGN` (324 bytes) and `BUTTONS.RGN` binary region record structures. Confirm count word at offset 0, parse all 32 region records, explain size difference between the two files. See [formats/RGN.md](formats/RGN.md).

- **SSF grammar**: Extract the complete file copy manifest from both SSF files; identify all keywords, operators, and conditionals in the SSF grammar. See [formats/SSF.md](formats/SSF.md).

---

## Loose File Differential Mapping (toggle in-game + diff)

- **EA.CFG**: Map all fields by toggling settings in-game and diffing. Cross-reference `CN_ReadConfig` symbol in FA.SMS. See [formats/CFG.md](formats/CFG.md).

- **NET.DAT**: Map multiplayer network config fields — callsign, session name, transport config. Cross-reference `CN_INFO` struct via FA.SMS. See [formats/NET.md](formats/NET.md).

---

## Win32 PE DLL Disassembly

For each item: load in Ghidra, import FA.SMS symbols via `scripts/ghidra/ImportFASms.java`, trace from the DLL entry point or a known FA.EXE caller.

- **FNT encoding details**: Confirm `cFont[0]` font height value by reading the pointer table header at raw file offset `0x200`. Verify whether `00`/`01`/`11` suffix encodes locale, resolution, or style. Resolve count discrepancy: 15 files inventoried vs 13 in FA_1.LIB. See [formats/FNT.md](formats/FNT.md).

- **HUD gauge completeness**: Confirm exact struct byte offsets by hex-diffing A7.HUD vs F22.HUD. Identify the four unresolved coordinate pairs at offsets 0x265–0x26B. Confirm per-aircraft anchor point source (hardcoded in FA.EXE vs stored elsewhere). See [formats/HUD.md](formats/HUD.md).

- **DLG field gaps**: Fill in unknown fields at +0x02..+0x09 (common header gap present in all record types). Decode `_ChoosePreload` record params (bounding-box vs dialog-type ID — `FUN_004897f0` body is minimal). Map all 92 DLG filenames to their in-game screens. See [formats/DLG.md](formats/DLG.md).

- **LAY gradient header**: Decode the `10 10` bytes in the gradient sub-block header (meaning of both u8 values unknown — channel count? sub-table count?). Confirm whether `0x31` is entries-per-sub-table or a type ID. Explain CLOUD1B = CLOUD1 (byte-for-byte identical). Map `layer <name>.LAY <index>` slot indices from `.MM` files to rendering layers. See [formats/LAY.md](formats/LAY.md).

- **MUS sub-opcode semantics**: Decode `FA`/`FB`/`FC`/`FD`/`FE` sub-opcode meanings — these are Miles Sound System XMIDI extensions processed by AIL natively; requires MSS documentation or Miles SDK headers. See [formats/MUS.md](formats/MUS.md).

- **HGR layout table**: Identify the second `.HGR` filename (likely a carrier or alternate airbase variant). Disassemble to identify the hangar layout table — aircraft slot positions, icon placement, camera angle. See [formats/HGR.md](formats/HGR.md).

- **PTS asset references**: Extract all 37 filenames and cross-reference with `.PT` aircraft. Determine whether `.PTS` also references `.HUD` or `.FNT` files for the aircraft's cockpit display. See [formats/PTS.md](formats/PTS.md).

---

## Format Deep Dives (BRF Numeric Fields)

- **SEE dual-lobe switch trigger**: Primary=search, secondary=track confirmed by range/angle comparison; the engine condition that switches lobes needs Ghidra. Also confirm sentinel values `$80000000`/`$7fffffff` (heading-error limits vs no-limit flags). Confirm F15R APG-63 implied range (~150 nm from raw value `^911400`). See [formats/SEE.md](formats/SEE.md).

- **JT agility and hit-probability bytes**: Map the byte sequence after seeker lobe data — turn rate, g-limit, fuze delay, Pk values. See [formats/JT.md](formats/JT.md).

- **JT warhead flags bits 0–15**: Bits 16–17 (AA/AG capability) confirmed; lower bits control fuze/warhead type. Needs Ghidra weapon evaluation function. See [formats/JT.md](formats/JT.md).

- **ECM effectiveness byte roles**: Variable bytes at positions 1, 5, 9 identified; whether they map to radar jamming / chaff / flare effectiveness needs Ghidra. Confirm whether `$1f0` is a bitmask (five frequency bands) or an enumerated power level. Cross-reference five fixed constants (35, 95, 24, 159, 31) against known RWR band frequencies. See [formats/ECM.md](formats/ECM.md).

- **GAS capacity word**: The `word` field (108/198/248/315) does not map linearly to US gallons. The `dword` mass is confirmed as fuel weight in lbs (6.6× gallon count). The capacity `word` requires FA.EXE fuel-system disassembly to decode. See [formats/GAS.md](formats/GAS.md).

---

## Mission / Campaign Formats

- **AI script semantics**: Locate the `.AI` script parser/interpreter in FA.EXE (xref to `.AI` filename loading). Confirm full `move` / `jink` argument semantics — heading reference points, speed modes. Confirm relationship between `.AI` script and companion `.BI` overlay. See [formats/AI.md](formats/AI.md).

- **CAM binary layout**: Disassemble `UKRAINE.CAM` to confirm binary layout of the mission state and weapon tables. Identify which `.MC` files correspond to which campaigns and missions. Determine how `.CAM` references theater `.MM` files. See [formats/CAM.md](formats/CAM.md).

- **MC condition logic**: Disassemble `UKR01.MC` to trace the complete condition-check logic. Determine how the `.CAM` file references or loads `.MC` files at mission start. Clarify `FOO.MC` and `EXTRA01.MC` — developer test missions or FA multiplayer extras. See [formats/MC.md](formats/MC.md).

- **MM field gaps**: Confirm `sides` entry count semantics. Determine world-space coordinate scale and origin for `pos`/`view` values. Document all `flags` bit assignments for `obj` blocks. Clarify `tmap_named` second and third argument semantics. Survey all `w_goal` values to enumerate waypoint goal types. Confirm `tdic id=256` meaning (tile type index into T2?). See [formats/MM.md](formats/MM.md).

---

## Terrain / Environment

- **T2 sub-header and surface class**: Decode sub-header bytes 4–17 (likely encodes world-space origin, scale, or min/max height bounds). Clarify header unknowns at 0x40 (20 bytes) and 0x60, 0x68. Determine the exact mapping from surface class byte to PIC atlas texture row/column. Confirm meaning of tile-summary record 0. Check OpenFA `crates/asset/mmm/` for any T2 parsing code. Determine whether FA_3.LIB (Disk 2) contains additional `.T2` files. See [formats/T2.md](formats/T2.md).

---

## Flag / Enum Mapping (Ghidra)

- **OT/NT `ot_flags` bits (5, 8, 10, 11, 22+)**: Bit patterns catalogued from full OT/NT survey; specific bit meanings need Ghidra confirmation of the damage/targeting/collision evaluation functions. Bit 22 (`$400000`) appears only on `~`-prefixed COLTWR variants — likely an override flag. See [formats/OT.md](formats/OT.md) and [formats/NT.md](formats/NT.md).

- **NT hardpoint bit 1 (`$2`)**: Bit 0 (hardpoint active) confirmed; bit 1 meaning unknown. Needs Ghidra weapon evaluation function trace. See [formats/NT.md](formats/NT.md).

- **SMS symbol map verification**: Cross-reference selected VAs against FA.EXE to confirm the symbol map is from a matching build. Identify build configuration / PDB origin. See [formats/SMS.md](formats/SMS.md).
