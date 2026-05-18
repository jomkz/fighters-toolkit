# Compiled AI Runtime (.BI)

FA_2.LIB contains 9 `.BI` files — exactly one per `.AI` script file (e.g. `AC130.BI` paired with `AC130.AI`). Each is a **Win32 PE DLL** that implements the runtime operations referenced by the paired `.AI` script.

## Role in the AI System

The FA engine runs a two-part AI system:

- **`.AI`** — plain-text source script compiled to bytecode at build time; defines the logic (conditions, branches, actions) in a goto-based language
- **`.BI`** — Win32 PE DLL that contains both the compiled bytecode of the paired `.AI` script (in its CODE section) and the native x86 implementations of every condition and action the bytecode calls (as `_CT*` exports)

At runtime the engine loads the `.BI` and calls `_CTExecProgram@4`, which reads bytecode from the BI CODE section and dispatches to the `_CTDo_*` and `_CTEval_*` exports via `CALL_BY_NAME`/`CALL_DIRECT` opcodes.

## Exported Functions

All `.BI` files export two families of functions, prefixed `_CT`:

### Action functions (`_CTDo_*`)

Called by the bytecode interpreter when executing an action instruction:

| Export | AI instruction |
|--------|---------------|
| `_CTDo_btoh` | `btoh` |
| `_CTDo_circle` | `circle` |
| `_CTDo_exit` | `exit` |
| `_CTDo_homeangle` | `homeAngle` |
| `_CTDo_homepos` | `homePos` |
| `_CTDo_immelman` | `immelman` |
| `_CTDo_invert` | `invert` |
| `_CTDo_jink` | `jink` |
| `_CTDo_maneuver` | `maneuver` |
| `_CTDo_move` | `move` |
| `_CTDo_movetoalt` | `moveToAlt` |
| `_CTDo_restart` | `restart` |
| `_CTDo_wm_approach` | `wm_approach` |
| `_CTDo_wm_break` | `wm_break` |
| `_CTDo_wm_hspacing` | (internal wingman spacing) |
| `_CTDo_yoyo` | `yoyo` |

Simpler `.BI` files (AC130.BI) export only the subset of actions used by that aircraft class.

### Condition functions (`_CTEval_*`)

Called by the bytecode interpreter when evaluating a condition:

| Export | AI attribute / condition |
|--------|--------------------------|
| `_CTEval_alt` | `alt` |
| `_CTEval_altdiff` | `altDiff` |
| `_CTEval_b` | `b` (internal counter) |
| `_CTEval_betterspeed` | `betterSpeed` |
| `_CTEval_bettertwr` | `betterTwr` |
| `_CTEval_canclimb` | `canClimb` |
| `_CTEval_corner` | `corner` / `cornerSpeed` |
| `_CTEval_disttotgt` | `distToTgt` |
| `_CTEval_do_attack` | `do_attack` |
| `_CTEval_do_evade` | `do_evade` |
| `_CTEval_do_hit` | `do_hit` |
| `_CTEval_do_ir_launch` | `do_ir_launch` |
| `_CTEval_do_nothing` | `do_nothing` |
| `_CTEval_do_radar_launch` | `do_radar_launch` |
| `_CTEval_engagep` | `engageP` |
| `_CTEval_h` | heading |
| `_CTEval_hdiff` | `hdiff` |
| `_CTEval_hrzdisttotgt` | `hrzDistToTgt` |
| `_CTEval_htotgt` | heading-to-target |
| `_CTEval_maxspeed` | (max speed attribute) |
| `_CTEval_minspeed` | `minSpeed` |
| `_CTEval_p` | `p` |
| `_CTEval_pdiff` | `pdiff` |
| `_CTEval_skill` | `skill` |
| `_CTEval_speed` | `speed` |
| `_CTEval_speeddiff` | `speedDiff` |
| `_CTEval_tgt` | `tgt` |
| `_CTEval_tgtahead` | `tgtAhead` |
| `_CTEval_tgtfacing` | `tgtFacing` |
| `_CTEval_tgthumancontrol` | `tgtHumanControl` |
| `_CTEval_tgtisfighter` | `tgtIsFighter` |
| `_CTEval_tgtisplane` | `tgtIsPlane` |
| `_CTEval_tgtoffbeam` | `tgtOffBeam` |
| `_CTEval_turnradius` | `turnRadius` |
| `_CTEval_wingapproach` | `wingApproach` |
| `_CTEval_wingcombat` | `wingCombat` |
| `_CTEval_wm_hspacing_is` | (wingman horizontal spacing) |

### Maneuver name strings

The trilingual maneuver name strings (e.g. `"GND ATTACK;BODENANGRIFF;ATTAQUE AU SOL"`) documented in [AI.md](AI.md) are embedded as data inside `F.BI` (the primary fighter AI runtime). `_CTDo_maneuver` reads these strings and passes the locale-appropriate segment to the UI.

## Bytecode Interpreter

### Overview

The interpreter is `_CTExecProgram@4` (`FUN_00466970`). It executes at most 5000 opcodes per call, then forcibly invokes `CTDo_exit` to prevent infinite loops.

**Runtime state globals:**

| Global | Role |
|--------|------|
| `DAT_00546bea` | Instruction pointer — `char*` into the loaded BI CODE section |
| `DAT_00546bf0` | Current script priority level (compared against `param_1` passed by caller) |
| `DAT_00546c94` | Pointer to the current actor's live object record |
| `DAT_00546c88` | Actor type flag: 1 if actor type is 2 or 4 (fighter/bomber) |
| `DAT_00546c90` | Execution result returned to caller (non-zero = script performed an action) |
| `DAT_00546c98` | Halt flag — set non-zero to stop execution early |
| `DAT_0050cf6e` | Current actor slot index (0 = player) |
| `DAT_0050d312` | CT system enable flag — interpreter is a no-op when this is zero |

**End-of-program marker:** `'%'` (0x25) — the main loop checks `*ip != '%'` as its loop condition.

**State save/restore:** `FUN_004668f0` restores a 32-dword CT state block from `DAT_0050cf90` (or zeroes it if no saved state). `FUN_00466920` saves the current state back. This enables preemptible script execution with re-entry.

**Opcode dispatch:** `FUN_00466a80` (0x466a80) reads one opcode byte from `*DAT_00546bea` and dispatches. Full opcode table below.

**Evaluation stack:** `FUN_00466290` = push; `FUN_00465ad0` = pop. Stack base = `DAT_00546bf2`; depth = `DAT_00546c42`. Max depth = 32 dwords. `FUN_00466820` reports error codes (1=syntax error, 4=stack underflow, 5=stack overflow, 0xa=unknown opcode, 0xb=call by name to unknown proc, 0xc=stack imbalance).

**Base address:** `DAT_00546be6` is the base pointer for the loaded BI CODE section; all jump offsets are relative to this base.

### Opcode Table (Confirmed)

| Opcode | IP advance | Name | Description |
|--------|-----------|------|-------------|
| `0x00` | 1 | NOP | No operation |
| `0x25` ('%') | — | END | End of program (also the main-loop terminator) |
| `0x01` | 5 | PUSH_DWORD | Push *(int32*)(IP+1) |
| `0x02` | 3 | PUSH_WORD | Push *(int16*)(IP+1) sign-extended |
| `0x03` | 2 | PUSH_BYTE | Push *(int8*)(IP+1) sign-extended |
| `0x04` | 1 | EVAL | Call FUN_00465ad0 (pop eval-stack top) |
| `0x05` | 2 | STORE_VAR | Pop → var[*(byte*)(IP+1)] via FUN_004670e0 |
| `0x06` | 2 | LOAD_VAR | Push var[*(byte*)(IP+1)] via FUN_004670e0 |
| `0x07` | varies | PUSH_ADDR | Push (IP+1 − base); advance IP past null-terminated string name |
| `0x08` | 1 | MUL | Pop a, b; push b×a |
| `0x09` | 1 | DIV | Pop a, b; push b/a (returns 0 if a=0) |
| `0x0A` | 1 | MOD | Pop a, b; push b%a (returns 0 if a=0) |
| `0x0B` | 1 | ADD | Pop a, b; push b+a |
| `0x0C` | 1 | SUB | Pop a, b; push b−a |
| `0x0D` | 1 | AND | Pop a, b; push b&a (bitwise) |
| `0x0E` | 1 | OR | Pop a, b; push b\|a (bitwise) |
| `0x0F` | 1 | XOR | Pop a, b; push b^a |
| `0x10` | 1 | SHL | Pop a, b; push b<<a |
| `0x11` | 1 | SHR | Pop a, b; push b>>a (arithmetic) |
| `0x12` | 1 | LT | Pop a, b; push (b < a) |
| `0x13` | 1 | LE | Pop a, b; push (b ≤ a) |
| `0x14` | 1 | GE | Pop a, b; push (b ≥ a) |
| `0x15` | 1 | GT | Pop a, b; push (b > a) |
| `0x16` | 1 | EQ | Pop a, b; push (b == a) |
| `0x17` | 1 | NE | Pop a, b; push (b ≠ a) |
| `0x18` | 1 | LAND | Pop a, b; push (b≠0 && a≠0) |
| `0x19` | 1 | LOR | Pop a, b; push (b≠0 \|\| a≠0) |
| `0x1A` | 1 | ABS | Pop a; push abs(a) |
| `0x1B` | 1 | NEG | Pop a; push −a |
| `0x1C` | 1 | NOT | Pop a; push (a == 0) |
| `0x1D` | 1 | RANDOM | Pop N (0–65535); push random(0..N−1) via engine RNG |
| `0x1E` | 1 | PERCENT | Pop N; push (random_100 < N) |
| `0x1F` | 1 | CHANCE | Pop N; scale by skill level (÷100 per level > 2); push (random_100 < scaled_N) |
| `0x20` | 3 | GOTO | Read s16 offset; jump to base + offset |
| `0x21` | 3 | PUSH_GOTO | Push (IP+1 − base), then execute GOTO with following s16 offset |
| `0x22` | 1 | JUMP | Pop addr; jump to base + addr |
| `0x23` | 3 | IF_FALSE | Pop cond; if cond==0: jump to base + s16 offset; else skip 2 bytes |
| `0x24` | varies | SWITCH | Pop idx; if 0 ≤ idx < N: jump to indexed table (1+N×2 bytes); else skip table |
| `0x26` | 5 | CALL_DIRECT | IP += 5; call *(code**)(IP+1); push return value |
| `0x27` | varies | CALL_BY_NAME | Look up null-terminated name, call; push return value; **self-patches to CALL_DIRECT** for subsequent calls (JIT optimization) |
| `0x28` | 5 | FRAME | Read 2 s16 values into `DAT_00546c44`/`DAT_00546c46`; IP += 4 |

### Argument Readers

`FUN_00465ad0` (0x465ad0) is the raw stack-pop function — pops one dword from the 32-entry eval stack at `DAT_00546bf2[DAT_00546c42 - 1]`. The `_CTDo_` handlers pop their arguments by calling higher-level wrappers that additionally validate and convert units:

| Address | Name (inferred) | Return value | Converts from |
|---------|----------------|--------------|---------------|
| `FUN_00465ad0` | `read_raw` | raw dword from eval stack | raw value |
| `FUN_00465d40` | `read_heading` | normalized heading in binary degrees (0–359° × 182) | normalizes to [0, 359], then × 182 |
| `FUN_00465c90` | `read_angle` | angle in binary degrees (clamped ±90° × 182) | clamps to [−90, 90], then × 182 |
| `FUN_00465da0` | `read_alt` | angle in binary degrees (clamped ±180° × 182) or `0x7FFFFFFF` (`any`) — used for roll in `CTDo_move` | clamps to [−180, 180], then × 182; passthrough if = 0x7FFFFFFF |
| `FUN_00465de0` | `read_duration` | unsigned int 0–15 (capped) | clamps to [0, 15] |
| `FUN_00465e00` | `read_speed` | speed in binary degrees, clamped to aircraft [min_speed, max_speed] | reads aircraft speed bounds at runtime |

### CTDo_move (`FUN_00465cc0`) — confirmed arg sequence

Calls `FUN_004ac510(heading, alt, roll_or_any, alt_is_any, vel_x, vel_y, speed, duration)`:
1. `heading` (binary degrees, 0–359° normalized) — from `read_heading`
2. `alt` (binary degrees, ±90°) — from `read_angle`
3. `roll` (binary degrees, ±180°, or `0x7FFFFFFF` = `any`) — from `read_alt`
4. `alt_is_any` (bool) — derived from altitude arg being 0x7FFFFFFF (the `any` sentinel)
5–6. velocity carry-over from previous command (`_DAT_00546c9c`, `_DAT_00546ca0`, zeroed after use)
7. `speed` (binary degrees, clamped to aircraft speed range) — from `read_speed`
8. `duration` (0–15 ticks) — from `read_duration`

`FUN_004ac510` (_MVRMove): clamps `alt` to ±0x3FFC (±90°); when `alt_is_any` = true → maneuver type 6 (any altitude) / roll target = 0; when false → type 1, roll target = `roll` arg.

### CTDo_turn (`FUN_00465ea0`) — confirmed arg sequence

1. min heading (degrees, clamped to current turn rate via `FUN_004780d0`)
2. max heading (clamped similarly)
3. type/mode (5 = timed, 6 = `any`-time/unconditional)
4. target heading in binary degrees (`arg * 182`, i.e. `arg * 65536/360`)
5. ctrl
6. duration

## Size

| BI file | Size | AI source size |
|---------|------|----------------|
| AC130.BI, B.BI, HYDRO.BI, LARGE.BI, LINER.BI | 4,608 B | 960–3,970 B |
| H.BI | 8,704 B | 12,412 B |
| F.BI, F117.BI, MOTH.BI | 12,800 B | 18,423–20,616 B |

For complex scripts (F, H) the bytecode is more compact than the source text. For simple scripts (LINER, LARGE) the PE overhead — headers, export table, native function bodies — exceeds the bytecode size, so the BI is larger than its `.AI` source. The compiler is internal to FA's toolchain and not distributed.

## Location

| LIB | Count |
|-----|-------|
| FA_2.LIB | 9 |

## TODO

- **Opcode 0x28 (`FRAME`)**: confirmed write path — `case 0x28:` sets `DAT_00546bea = pbVar1`, conditionally calls `FUN_00466820(0xc)` when both `DAT_00546c8c != '\0'` and `DAT_00546c42 != 0` (stack/mode guard), then reads two consecutive `uint16` values from the stream into `_DAT_00546c44` and `_DAT_00546c46`, advancing the pointer by 4. No function reads these globals by direct address; they are accessed via a pointer to the surrounding state block. Candidate consumers identified by offset scan: `_INFO2Draw`, `_FMFlight@0`, `_MANAdd@24`, `_GVDoCurrentWaypoint`, `?MPStatusSet@@YIXJ@Z`, `FUN_0048e740`. First s16 = sequential per-block ID (IDs 1–6 reserved; scripts start at 7); second s16 increases monotonically — likely profiling or priority scheduling data.

- **AI→BI compiler**: The AI language and all 40 bytecode opcodes are fully documented, making a compiler from `.AI` text to bytecode straightforward in principle. The blocker is the BI PE layout: it is not yet known which section holds the bytecode versus the native x86 `_CTDo_*`/`_CTEval_*` implementations, so it is unclear how to embed new bytecode in a valid DLL. The  approach once the layout is mapped: write the _CTDo_*/_CTEval_* implementations from scratch in C and link them into a new DLL alongside the compiled bytecode. This produces a clean, self-contained result.

## Related

- [AI.md](AI.md) — plain-text AI script; uses the exports of the paired `.BI` as its instruction set
