import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.*;

public class DumpHUDFunctions extends GhidraScript {

    static final long[] ADDRESSES = {
        0x00407B60L, // HUDDrawHeading
        0x00407EE0L, // HUDDrawSpeed
        0x00408420L, // HUDDrawAlt
        0x00408E20L, // HUDDrawHVel
        0x00409030L, // HUDDrawWeaponInfo
        0x004092D0L, // HUDDrawRangeInfo
        0x0040ACE0L, // HUDDrawStability
        0x0040ABB0L, // HUDDrawDisrupt
    };

    @Override
    public void run() throws Exception {
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        File outFile = new File("C:\\Temp\\hud_functions.txt");
        outFile.getParentFile().mkdirs();
        PrintWriter out = new PrintWriter(new FileWriter(outFile));

        for (long addr : ADDRESSES) {
            Address funcAddr = currentProgram.getAddressFactory()
                .getDefaultAddressSpace().getAddress(addr);
            Function func = getFunctionAt(funcAddr);
            if (func == null) {
                func = currentProgram.getFunctionManager().getFunctionContaining(funcAddr);
            }
            if (func != null) {
                DecompileResults results = decompiler.decompileFunction(func, 120, monitor);
                if (results.decompileCompleted()) {
                    out.println("// === " + func.getName() + " @ " + func.getEntryPoint() + " ===");
                    out.println(results.getDecompiledFunction().getC());
                    out.println();
                } else {
                    out.println("// === DECOMPILE FAILED: " + funcAddr + " ===");
                    out.println();
                }
            } else {
                out.println("// === NO FUNCTION AT: " + funcAddr + " ===");
                out.println();
            }
        }

        out.close();
        decompiler.dispose();
        println("Done. Output written to C:\\Temp\\hud_functions.txt");
    }
}
