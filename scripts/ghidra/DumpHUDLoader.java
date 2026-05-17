import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class DumpHUDLoader extends GhidraScript {

    // Gauge globals found from HUD draw functions
    static final long[] GAUGE_GLOBALS = {
        0x00521541L, // heading width
        0x00521545L, // heading dy
        0x00521557L, // speed dx
        0x0052155bL, // speed dy
        0x0052155fL, // speed height
        0x00521574L, // alt dx
        0x00521578L, // alt dy
        0x0052157cL, // alt height
        0x00521591L, // hvel/FPM dx
        0x00521593L, // hvel/FPM dy
        0x00521595L, // hvel box half-w
        0x00521597L, // hvel box half-h
        0x005215cdL, // weapon info dx
        0x005215cfL, // weapon info dy
        0x005215d1L, // range info dx
        0x005215d3L, // range info dy
    };

    @Override
    public void run() throws Exception {
        File outFile = new File("C:\\Temp\\hud_loader.txt");
        outFile.getParentFile().mkdirs();
        PrintWriter out = new PrintWriter(new FileWriter(outFile));

        // Find all functions that write to the heading width global (0x521541)
        // as the entry point to finding the HUD loader
        Address targetAddr = currentProgram.getAddressFactory()
            .getDefaultAddressSpace().getAddress(0x00521541L);

        out.println("=== References to DAT_00521541 (heading width) ===");
        ReferenceIterator refs = currentProgram.getReferenceManager()
            .getReferencesTo(targetAddr);
        Set<Function> loaderCandidates = new HashSet<>();
        while (refs.hasNext()) {
            Reference ref = refs.next();
            Address fromAddr = ref.getFromAddress();
            Function func = currentProgram.getFunctionManager()
                .getFunctionContaining(fromAddr);
            String funcName = (func != null) ? func.getName() + " @ " + func.getEntryPoint() : "?";
            out.println("  " + fromAddr + " (" + ref.getReferenceType() + ") in " + funcName);
            if (func != null && ref.getReferenceType().isWrite()) {
                loaderCandidates.add(func);
            }
        }
        out.println();

        // Decompile each candidate loader function
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        for (Function func : loaderCandidates) {
            out.println("// === LOADER CANDIDATE: " + func.getName() + " @ " + func.getEntryPoint() + " ===");
            DecompileResults results = decompiler.decompileFunction(func, 120, monitor);
            if (results.decompileCompleted()) {
                out.println(results.getDecompiledFunction().getC());
            } else {
                out.println("// DECOMPILE FAILED");
            }
            out.println();
        }

        out.close();
        decompiler.dispose();
        println("Done. Output written to C:\\Temp\\hud_loader.txt");
    }
}
