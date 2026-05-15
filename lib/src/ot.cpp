#include "ft/ot.h"
#include <cstdio>
#include <cstring>

namespace ft {

// ---------------------------------------------------------------------------
// OT general section (struct_type 1..7 all share this header)
// Derived from OpenFA crates/asset/ot/src/lib.rs
// ---------------------------------------------------------------------------
const OtField OT_GENERAL_FIELDS[] = {
    // --- general info ---
    { "struct_type",        "1=OT 3=NT 5=PT 7=JT" },
    { "type_size",          "bytes" },
    { "instance_size",      "bytes" },
    { "names",              "ptr -> short, long, filename" },
    { "ot_flags",           "bitfield" },
    { "obj_class",          "$8000=Fighter $4000=Bomber $2000=Ship $1000=SAM $800=AAA $400=Tank $200=Vehicle $100=Struct $80=Projectile" },
    { "shape",              "ptr -> .SH filename" },
    { "shadow_shape",       "ptr -> .SH filename (or dword 0)" },
    { "dmg_debris_pos_x",   "i16 feet (V2+)" },
    { "dmg_debris_pos_y",   "i16 feet (V2+)" },
    { "dmg_debris_pos_z",   "i16 feet (V2+)" },
    { "dst_debris_pos_x",   "i16 feet (V2+)" },
    { "dst_debris_pos_y",   "i16 feet (V2+)" },
    { "dst_debris_pos_z",   "i16 feet (V2+)" },
    { "dmg_type",           "u32 (V2+)" },
    { "year_available",     "u32 year (V3+)" },
    { "max_vis_dist",       "i16 feet" },
    { "camera_dist",        "i16 feet" },
    { "unk_sig_22",         "u16" },
    { "unk_sig_laser",      "u16" },
    { "unk_sig_ir",         "u16" },
    { "unk_sig_radar",      "u16" },
    { "unk_sig_26",         "u16" },
    { "hit_points",         "u16" },
    { "dmg_planes",         "u16 damage vs planes" },
    { "dmg_ships",          "u16 damage vs ships" },
    { "dmg_structures",     "u16 damage vs structures" },
    { "dmg_armor",          "u16 damage vs armor" },
    { "dmg_other",          "u16 damage vs other" },
    { "explosion_type",     "u8 15=SmallGround 18=SmallAir 21=MedGround 26=LargeGround 27=Flak 30=LargeAir" },
    { "crater_size",        "u8 feet" },
    { "empty_weight",       "u32 lbs" },
    { "cmd_buf_size",       "u16" },
    // --- movement info ---
    { "turn_rate",          "u16" },
    { "bank_rate",          "u16" },
    { "max_climb",          "i16" },
    { "max_dive",           "i16" },
    { "max_bank",           "i16" },
    { "min_speed",          "u16 mph" },
    { "corner_speed",       "u16 mph" },
    { "max_speed",          "u16 mph" },
    { "acceleration",       "i32" },
    { "deceleration",       "i32" },
    { "min_altitude",       "i32 feet" },
    { "max_altitude",       "i32 feet" },
    { "util_proc",          "symbol" },
    // --- sound info ---
    { "loop_sound",         "ptr -> .11K filename" },
    { "second_sound",       "ptr -> .11K filename" },
    { "engine_on_sound",    "ptr -> .11K filename (V1+)" },
    { "engine_off_sound",   "ptr -> .11K filename (V1+)" },
    { "do_doppler",         "byte 0/1" },
    { "max_snd_dist",       "u16 feet" },
    { "doppler_pitch_lo",   "i16" },
    { "doppler_pitch_hi",   "i16" },
    { "doppler_speed_lo",   "i16" },
    { "doppler_speed_hi",   "i16" },
    { "view_offset_x",      "i16 feet" },
    { "view_offset_y",      "i16 feet" },
    { "view_offset_z",      "i16 feet" },
    { "hud_name",           "ptr -> .HUD filename (V2+)" },
};
const int OT_GENERAL_COUNT = (int)(sizeof(OT_GENERAL_FIELDS) / sizeof(OT_GENERAL_FIELDS[0]));

// ---------------------------------------------------------------------------
// NT (NpcType) extension -- follows OT_GENERAL when struct_type >= 3
// ---------------------------------------------------------------------------
const OtField NT_FIELDS[] = {
    { "npc_flags",          "u32" },
    { "ct_name",            "ptr -> pilot/crew type name" },
    { "crew_chief_skill",   "byte" },
    { "wingman_skill",      "byte" },
    { "leader_skill",       "byte" },
    { "max_target_dist",    "i16" },
    { "unk_nt_1",           "i16" },
    { "ai_aggressiveness",  "byte" },
    { "hards",              "ptr -> hardpoint table" },
};
const int NT_COUNT = (int)(sizeof(NT_FIELDS) / sizeof(NT_FIELDS[0]));

// ---------------------------------------------------------------------------
// PT (PlaneType) extension -- follows NT when struct_type >= 5
// Derived from OpenFA crates/asset/pt/src/lib.rs
// ---------------------------------------------------------------------------
const OtField PT_FIELDS[] = {
    { "pt_flags",           "$1=Jet $2=Hook $4=TwoSeat $8=Helo $10=Eject $20=VTOL $40=Carrier $80=Bay" },
    { "env",                "ptr -> flight envelope table" },
    { "env_min_g",          "i16 minimum G envelope index" },
    { "env_max_g",          "i16 maximum G envelope index" },
    { "max_speed_sea_lvl",  "u16 mph" },
    { "max_speed_36k",      "u16 mph at 36000 ft" },
    // bv_x (pitch velocity): min, max, acc, dacc
    { "bv_x_min",           "i16" }, { "bv_x_max", "i16" },
    { "bv_x_acc",           "i16" }, { "bv_x_dacc", "i16" },
    // bv_y (roll velocity)
    { "bv_y_min",           "i16" }, { "bv_y_max", "i16" },
    { "bv_y_acc",           "i16" }, { "bv_y_dacc", "i16" },
    // bv_z (yaw velocity)
    { "bv_z_min",           "i16" }, { "bv_z_max", "i16" },
    { "bv_z_acc",           "i16" }, { "bv_z_dacc", "i16" },
    // brv_x (roll rate)
    { "brv_x_min",          "i16" }, { "brv_x_max", "i16" },
    { "brv_x_acc",          "i16" }, { "brv_x_dacc", "i16" },
    // brv_y (pitch rate)
    { "brv_y_min",          "i16" }, { "brv_y_max", "i16" },
    { "brv_y_acc",          "i16" }, { "brv_y_dacc", "i16" },
    // brv_z (rudder/yaw rate)
    { "brv_z_min",          "i16" }, { "brv_z_max", "i16" },
    { "brv_z_acc",          "i16" }, { "brv_z_dacc", "i16" },
    { "gpull_aoa",          "i16 deg" },
    { "low_aoa_speed",      "i16 mph" },
    { "low_aoa_pitch",      "i16" },
    { "turbulence_pct",     "i16 (V1+)" },
    { "stall_warn_delay",   "i16" },
    { "stall_delay",        "i16" },
    { "stall_severity",     "i16" },
    { "stall_pitch_down",   "i16" },
    { "spin_entry",         "i16" }, { "spin_exit",      "i16" },
    { "spin_yaw_lo",        "i16" }, { "spin_yaw_hi",    "i16" },
    { "spin_aoa_lo",        "i16" }, { "spin_aoa_hi",    "i16" },
    { "spin_bank_lo",       "i16" }, { "spin_bank_hi",   "i16" },
    // rudder
    { "rudder_yaw_min",     "i16" }, { "rudder_yaw_max", "i16" },
    { "rudder_yaw_acc",     "i16" }, { "rudder_yaw_dacc","i16" },
    { "rudder_slip",        "i16" }, { "rudder_drag",    "i16" },
    { "rudder_bank",        "i16" },
    { "coef_drag",          "i16" },
    { "air_brakes_drag",    "i16" },
    { "wheel_brakes_drag",  "i16" },
    { "flaps_drag",         "i16" }, { "flaps_lift",     "i16" },
    { "gear_drag",          "i16" },
    { "bay_drag",           "i16" },
    { "loaded_drag",        "i16" }, { "loaded_gpull_drag","i16" },
    { "loaded_elevator",    "i16" }, { "loaded_aileron",  "i16" },
    { "loaded_rudder",      "i16" },
    { "struct_warn_limit",  "i16 G" },
    { "struct_limit",       "i16 G" },
    // system_damage[32]
    { "sys_dmg_0",  "u8" }, { "sys_dmg_1",  "u8" }, { "sys_dmg_2",  "u8" }, { "sys_dmg_3",  "u8" },
    { "sys_dmg_4",  "u8" }, { "sys_dmg_5",  "u8" }, { "sys_dmg_6",  "u8" }, { "sys_dmg_7",  "u8" },
    { "sys_dmg_8",  "u8" }, { "sys_dmg_9",  "u8" }, { "sys_dmg_10", "u8" }, { "sys_dmg_11", "u8" },
    { "sys_dmg_12", "u8" }, { "sys_dmg_13", "u8" }, { "sys_dmg_14", "u8" }, { "sys_dmg_15", "u8" },
    { "sys_dmg_16", "u8" }, { "sys_dmg_17", "u8" }, { "sys_dmg_18", "u8" }, { "sys_dmg_19", "u8" },
    { "sys_dmg_20", "u8" }, { "sys_dmg_21", "u8" }, { "sys_dmg_22", "u8" }, { "sys_dmg_23", "u8" },
    { "sys_dmg_24", "u8" }, { "sys_dmg_25", "u8" }, { "sys_dmg_26", "u8" }, { "sys_dmg_27", "u8" },
    { "sys_dmg_28", "u8" }, { "sys_dmg_29", "u8" }, { "sys_dmg_30", "u8" }, { "sys_dmg_31", "u8" },
    { "num_engines",        "u8" },
    { "neg_g_limit",        "i16" },
    { "thrust",             "u32 lbf" },
    { "aft_thrust",         "u32 lbf afterburner" },
    { "throttle_acc",       "i16" }, { "throttle_dacc",  "i16" },
    { "fuel_consumption",   "i16 lbs/sec" },
    { "aft_fuel_consumption","i16 lbs/sec" },
    { "internal_fuel",      "u32 lbs" },
    { "gear_pitch",         "i16" },
    { "crash_speed_fwd",    "i16 mph" },
    { "crash_speed_side",   "i16 mph" },
    { "crash_speed_vert",   "i16 mph" },
    { "crash_pitch",        "i16" }, { "crash_roll", "i16" },
    { "misc_per_flight",    "i16" },
    { "repair_multiplier",  "i16" },
    { "max_takeoff_weight", "u32 lbs" },
};
const int PT_COUNT = (int)(sizeof(PT_FIELDS) / sizeof(PT_FIELDS[0]));

// ---------------------------------------------------------------------------
// JT (ProjectileType) extension -- follows OT_GENERAL when struct_type == 7
// ---------------------------------------------------------------------------
const OtField JT_FIELDS[] = {
    { "jt_flags",           "u32" },
    { "warhead_type",       "u16" },
    { "guidance_type",      "u8" },
    { "si_names",           "ptr -> seeker info names" },
    { "lock_angle",         "u16 deg" },
    { "unk_jt_1",           "u8" },
    { "unk_jt_2",           "u8" },
    { "unk_jt_3",           "u8" },
    { "unk_jt_4",           "u8" },
    { "unk_jt_5",           "u8" },
    { "unk_jt_6",           "u8" },
    { "unk_jt_7",           "u8" },
    { "unk_jt_8",           "u8" },
    // (many more JT fields -- schema is partial)
};
const int JT_COUNT = (int)(sizeof(JT_FIELDS) / sizeof(JT_FIELDS[0]));

// ---------------------------------------------------------------------------
// Info printer
// ---------------------------------------------------------------------------

static void print_field(int idx, const BrfField& f, const OtField* schema, int schema_count,
                         const BrfDoc& doc) {
    const char* name = (schema && idx < schema_count && schema[idx].name[0])
                       ? schema[idx].name : "";
    const char* note = (schema && idx < schema_count) ? schema[idx].note : "";

    if (f.type == "ptr") {
        // Resolve the pointer to its string table
        const BrfTable* tbl = doc.find_table(f.value);
        if (tbl && !tbl->strings.empty()) {
            if (name[0]) printf("  [%3d] %-24s = %s -> \"%s\"",
                                idx, name, f.value.c_str(), tbl->strings[0].c_str());
            else         printf("  [%3d] %-24s   %s -> \"%s\"",
                                idx, "", f.value.c_str(), tbl->strings[0].c_str());
        } else {
            if (name[0]) printf("  [%3d] %-24s = ptr %s", idx, name, f.value.c_str());
            else         printf("  [%3d] %-24s   ptr %s", idx, "", f.value.c_str());
        }
    } else if (f.type == "symbol") {
        if (name[0]) printf("  [%3d] %-24s = %s", idx, name, f.value.c_str());
        else         printf("  [%3d] %-24s   %s", idx, "", f.value.c_str());
    } else {
        // Numeric: decode ^X and $HEX for display
        int64_t ival = brf_parse_int(f.value);
        if (name[0]) printf("  [%3d] %-24s = %-12s (%lld)", idx, name, f.value.c_str(), (long long)ival);
        else         printf("  [%3d] %-24s   %-12s (%lld)", idx, "", f.value.c_str(), (long long)ival);
    }

    if (note && note[0]) printf("  ; %s", note);
    printf("\n");
}

void brf_print_info(const BrfDoc& doc, const char* format) {
    bool is_pt  = strcmp(format, "pt") == 0;
    bool is_nt  = strcmp(format, "nt") == 0;
    bool is_jt  = strcmp(format, "jt") == 0;

    // Determine struct_type from first field
    int struct_type = 0;
    if (!doc.fields.empty() && doc.fields[0].type == "byte")
        struct_type = (int)brf_parse_int(doc.fields[0].value);

    printf("--- OT/General Section (struct_type=%d) ---\n", struct_type);
    int fi = 0; // field index into doc.fields[]
    int si = 0; // schema index into OT_GENERAL_FIELDS[]

    for (; fi < (int)doc.fields.size() && si < OT_GENERAL_COUNT; ++fi, ++si) {
        print_field(fi, doc.fields[fi], OT_GENERAL_FIELDS, OT_GENERAL_COUNT, doc);
    }

    if (struct_type == 7 || is_jt) {
        printf("\n--- JT/Projectile Extension (struct_type=7) ---\n");
        for (int ji = 0; fi < (int)doc.fields.size(); ++fi, ++ji) {
            print_field(fi, doc.fields[fi], JT_FIELDS, JT_COUNT, doc);
        }
    } else if (struct_type >= 3 || is_nt || is_pt) {
        printf("\n--- NT/Npc Extension (struct_type>=3) ---\n");
        for (int ni = 0; fi < (int)doc.fields.size() && ni < NT_COUNT; ++fi, ++ni) {
            print_field(fi, doc.fields[fi], NT_FIELDS, NT_COUNT, doc);
        }
        if (struct_type >= 5 || is_pt) {
            printf("\n--- PT/Plane Extension (struct_type>=5) ---\n");
            for (int pi = 0; fi < (int)doc.fields.size(); ++fi, ++pi) {
                print_field(fi, doc.fields[fi], PT_FIELDS, PT_COUNT, doc);
            }
        }
    } else {
        // Print any remaining unlabeled fields
        for (; fi < (int)doc.fields.size(); ++fi) {
            print_field(fi, doc.fields[fi], nullptr, 0, doc);
        }
    }

    if (!doc.tables.empty()) {
        printf("\n--- Pointer Tables ---\n");
        for (auto& t : doc.tables) {
            printf("  :%s\n", t.name.c_str());
            for (auto& s : t.strings)
                printf("    \"%s\"\n", s.c_str());
        }
    }
}

} // namespace ft
