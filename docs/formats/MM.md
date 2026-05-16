# Theater / Map Layout (.MM)

FA_2.LIB contains 75 `.MM` files. Each defines a theater or campaign map — the geographic region, terrain tile layout, and associated mission context. Pilot save files (`.P`) reference the active campaign by its `.CAM` filename, which in turn points to one or more `.MM` theater maps.

## Status

Structure unknown. The OpenFA project references this format in `crates/asset/mmm/` but no format specification has been extracted from that work yet.

## Location

| LIB | Count |
|-----|-------|
| FA_2.LIB | 75 |

## Related

- [CAM.md](CAM.md) — campaign definitions that reference `.MM` files
- [MISSION.md](MISSION.md) — `.M` individual missions that are placed within a theater
- [T2.md](T2.md) — terrain height/color/type maps, likely referenced by `.MM`
