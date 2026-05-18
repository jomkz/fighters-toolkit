# Mission Condition Script (.MC)

FA_2.LIB contains 21 `.MC` files. Each implements the runtime condition checks for a specific mission event — trigger conditions, completion logic, and failure detection. Each is a **Win32 PE DLL** loaded at runtime.

## File Inventory

All 21 filenames:

`CATFAIL.MC`, `EXTRA01.MC`, `FOO.MC`, `K16.MC`, `K17.MC`, `TRAIN01.MC`, `U01.MC`, `U07.MC`, `U08.MC`, `U11.MC`, `U12.MC`, `U15.MC`, `U22.MC`, `U23.MC`, `U24.MC`, `U25.MC`, `U29.MC`, `U34.MC`, `UKR01.MC`, `UKR02.MC`, `VIET03.MC`

- `U*.MC` — Ukraine campaign mission conditions (not all 50 missions have a dedicated `.MC`; only those with non-trivial trigger logic)
- `K*.MC` — Kurile campaign missions
- `VIET03.MC` — Vietnam campaign mission 3
- `CATFAIL.MC` — carrier takeoff failure condition
- `TRAIN01.MC` — training mission condition
- `UKR01.MC`, `UKR02.MC` — Ukraine campaign-level events
- `EXTRA01.MC` — legitimate mission condition (uses `@OBJAlias@8`, `@OBJGet@4`, `_MISSIONSuccess@0`; no test strings)
- `FOO.MC` — developer timing test: embeds debug string `"The time is now >= 10 seconds!"`, uses `_currentTime` and `_MSGSendChatter@24` with `RADIOBP.5K` audio; name is a classic programmer placeholder

## Content

String analysis of all `.MC` files reveals the mission condition API:

| Import | Description |
|--------|-------------|
| `@OBJAlias@8` | Look up a game object by its alias ID |
| `@OBJGet@4` | Get a game object by index (EXTRA01.MC) |
| `_Dist@8` | Compute distance between two objects |
| `_MISSIONSuccess@0` | Trigger mission success outcome |
| `_MSGSendChatter@24` | Send a radio chatter message to the player |
| `_OnTheGround@0` | Test whether an object is on the ground |
| `_PopCurObj@0` | Pop the current object from the evaluation stack |
| `_PushCurObj@4` | Push an object onto the evaluation stack |
| `_currentTime` | Global: current game time in ticks |
| `_playerId` | Global: the player's object ID |

These are physics/world-state query functions — the `.MC` DLL polls game state each tick to detect mission trigger conditions (e.g. player landed, target destroyed, distance threshold crossed).

## Format

Win32 PE DLL. All observed `.MC` files decompressed to **4608 bytes**.

## Location

| LIB | Count |
|-----|-------|
| FA_2.LIB | 21 |

## Campaign Condition Text Files (.mc_M, .mc_nato_M)

These are **distinct** from the `.MC` PE DLL files above. The campaign engine (`FUN_00428412`) loads campaign-wide condition scripts as plain-text files with suffix `.mc_M` (standard campaign) or `.mc_nato_M` (NATO campaign variant). Loaded via `FUN_00481940` → `FUN_00481c10` (text parser using `FUN_00483c90` as tokenizer + `__strlwr`).

Confirmed keywords parsed by `FUN_00481c10`:

| Keyword | Effect |
|---------|--------|
| `textformat` | Sets file format version/type flag |
| `briefmap` | Sets `DAT_005516e0 = 1` (briefing map active) |
| `selectplane` | Sets `DAT_0054e498 = 1` (plane selection screen) |
| `armplane` | Sets `DAT_00552820 = 1` (arming screen) |
| `layer` | Reads layer name and integer parameter |
| (startup coords) | Reads 3 fixed-point world-space coordinates (× 256) |

These files are stored in the LIB archive with `.mc_M` / `.mc_nato_M` suffixes, not `.MC`. They are text keyword files, not Win32 PE DLLs.

## Dispatch Chain (Confirmed)

The full MC dispatch chain from game entry:

```
?usnfmain@@YAXXZ (0x403700)          -- main loop
FUN_00428412 (0x428412)              -- campaign/mission loader
  └─ _CallMissionProc_8 (0x481940)   -- central DLL dispatcher
       ├─ _MISSIONInit2_0 (0x480b50) -- post-load init; assigns _eventFilterProc
       ├─ _MISSIONTextProc@16 (0x481c10)   -- re-entry: text condition parse
       └─ _MISSIONCheckSuccess@0 (0x486860) -- re-entry: success test
```

`_MISSIONInit2_0` (0x480b50) iterates all `_objPtrs`, calls `_MAPSetSide_4`, `_OBJFindHumans_0`, `_OBJAliasAll_12`, `_OBJAliasForMulti_0`, `_TIMEInit_12`, `_G_SetScaleMax_8`, then loads the mission DLL via `_RMAccess_8(&_missionDLLName__3PADA, 0x8000)`, storing the result in `_eventFilterProc`.

Confirmed condition keyword consumers:

| Keyword | Consumer functions |
|---------|-------------------|
| `DESTROY` | `FUN_0043a5c0`, `FUN_00431ab0`, `FUN_0044fe10` |
| `FAIL` | `FUN_004a2a41` |
| `tmap` | `_MISSIONTextProc@16` (string at `0x4fc228`); `FUN_00495e80` (`.MC` handler at `0x495e80`, string at `0x5010f4`) |

The `cond` keyword appears at 8 locations in FA.EXE but no function references were found — its parse handler and internal dispatch table in `_MISSIONTextProc` remain unmapped.

## TODO — Deep Dive

- Disassemble `UKR01.MC` to trace the complete condition check logic and identify all object aliases it monitors
- Map remaining `.mc_M` keyword handlers beyond those confirmed above (`cond` keyword handler not found)
- Disassemble `FUN_00495e80` (`.MC` string handler at 0x495e80) to identify its role in the condition pipeline
- Clarify `EXTRA01.MC` purpose (multiplayer extra, bonus mission, or other)

## Related

- [CAM.md](CAM.md) — campaign engine that loads `.MC` files to evaluate mission state
- [M.md](M.md) — `.M` mission files whose events `.MC` evaluates
