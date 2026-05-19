// Dumps decompiled C for every function in FA.EXE, grouped by SMS symbol namespace.
// This covers all VA ranges not targeted by the subsystem-specific Analyze*.java scripts.
// Invoke: run_ghidra.bat DumpAllFunctions.java
// Output: %FA_PROJECT%\output\DumpAllFunctions.txt

import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.util.*;

public class DumpAllFunctions extends FAScript {

    @Override
    public void run() throws Exception {
        openOutput("DumpAllFunctions");

        // Collect all functions sorted by entry point VA
        List<Function> allFunctions = new ArrayList<>();
        for (Function fn : currentProgram.getFunctionManager().getFunctions(true)) {
            allFunctions.add(fn);
        }
        allFunctions.sort((a, b) ->
            Long.compare(a.getEntryPoint().getOffset(), b.getEntryPoint().getOffset()));

        // Group by name prefix (SMS namespace) for organised output
        String currentGroup = "";
        int total = 0;
        int skipped = 0;

        for (Function fn : allFunctions) {
            long va = fn.getEntryPoint().getOffset();

            // Determine group from function name prefix
            String name = fn.getName();
            String group = deriveGroup(name, va);
            if (!group.equals(currentGroup)) {
                currentGroup = group;
                header("GROUP: " + currentGroup);
            }

            if (!dumped.contains(va)) {
                dumpAt(va);
                total++;
            } else {
                skipped++;
            }
        }

        out.println("// Total functions dumped: " + total);
        out.println("// Skipped (already dumped): " + skipped);

        closeOutput();
    }

    private String deriveGroup(String name, long va) {
        String n = name.toLowerCase();
        // SMS namespace prefixes
        if (n.startsWith("_mission") || n.startsWith("mission")) return "MISSION";
        if (n.startsWith("_proj") || n.startsWith("proj")) return "PROJ";
        if (n.startsWith("_plane") || n.startsWith("plane") || n.startsWith("_fm") || n.startsWith("fm")) return "PLANE_FM";
        if (n.startsWith("_gv") || n.startsWith("gvproc") || n.startsWith("_gvproc")) return "GV";
        if (n.startsWith("cn_") || n.startsWith("_cn_")) return "NET_CN";
        if (n.startsWith("mp") || n.startsWith("_mp") || n.startsWith("?mp")) return "MP";
        if (n.startsWith("_damage") || n.startsWith("damage")) return "DAMAGE";
        if (n.startsWith("_hard") || n.startsWith("hard") || n.startsWith("@hard")) return "HARD";
        if (n.startsWith("_ct") || n.startsWith("ct") || n.startsWith("_ctdo") || n.startsWith("_cteval")) return "AI_CT";
        if (n.startsWith("_mvr") || n.startsWith("mvr")) return "AI_MVR";
        if (n.startsWith("_seq") || n.startsWith("seq") || n.startsWith("?seq") || n.startsWith("?music") || n.startsWith("_ail")) return "AUDIO_SEQ";
        if (n.startsWith("_dialog") || n.startsWith("dialog") || n.startsWith("_draw") || n.startsWith("_choose")) return "UI_DLG";
        if (n.startsWith("_hud") || n.startsWith("hud") || n.startsWith("?hud")) return "HUD";
        if (n.startsWith("_lib") || n.startsWith("lib") || n.startsWith("_loadfile") || n.startsWith("loadfile")) return "LIB";
        if (n.startsWith("_t_") || n.startsWith("t_") || n.startsWith("@t_")) return "TERRAIN";
        if (n.startsWith("_g_") || n.startsWith("g_tile") || n.startsWith("@g_")) return "RENDER_TILE";
        if (n.startsWith("_map") || n.startsWith("map") || n.startsWith("?map")) return "MAP";
        if (n.startsWith("_see") || n.startsWith("see")) return "SEE";
        if (n.startsWith("_ecm") || n.startsWith("ecm")) return "ECM";
        if (n.startsWith("_obj") || n.startsWith("obj") || n.startsWith("@obj")) return "OBJ";
        if (n.startsWith("_pilot") || n.startsWith("pilot") || n.startsWith("_plt")) return "PILOT";
        if (n.startsWith("_input") || n.startsWith("input") || n.startsWith("_joy") || n.startsWith("joy")) return "INPUT";
        if (n.startsWith("_net") || n.startsWith("net") || n.startsWith("_ipx") || n.startsWith("ipx") || n.startsWith("_tcp") || n.startsWith("tcp")) return "NET";
        if (n.startsWith("_wr") || n.startsWith("wr")) return "RENDER_WR";
        if (n.startsWith("_lay") || n.startsWith("lay")) return "LAY";
        if (n.startsWith("_brg") || n.startsWith("brg") || n.startsWith("_brf") || n.startsWith("brf")) return "BRF";
        // VA range fallback groups for functions with no SMS name (FUN_*)
        if (va >= 0x401000L && va < 0x410000L) return "VA_401000_INIT";
        if (va >= 0x410000L && va < 0x420000L) return "VA_410000_HUD_STATE";
        if (va >= 0x420000L && va < 0x430000L) return "VA_420000_WORLD_INPUT";
        if (va >= 0x430000L && va < 0x446000L) return "VA_430000_CAMPAIGN_ASSET";
        if (va >= 0x446000L && va < 0x460000L) return "VA_446000_AUDIO_WEAPON";
        if (va >= 0x460000L && va < 0x470000L) return "VA_460000_AI_BI";
        if (va >= 0x470000L && va < 0x482000L) return "VA_470000_GV_MM";
        if (va >= 0x482000L && va < 0x4AB000L) return "VA_482000_MP_MISSION";
        if (va >= 0x4AB000L && va < 0x4B3000L) return "VA_4AB000_AI_MOV";
        if (va >= 0x4B3000L && va < 0x4C0000L) return "VA_4B3000_LAY_RENDER";
        if (va >= 0x4C0000L && va < 0x4D0000L) return "VA_4C0000_PROJ_SEE";
        if (va >= 0x4D0000L && va < 0x4F0000L) return "VA_4D0000_PHYSICS_TERRAIN";
        if (va >= 0x4F0000L && va < 0x540000L) return "VA_4F0000_DATA_TABLES";
        return "VA_OTHER";
    }
}
