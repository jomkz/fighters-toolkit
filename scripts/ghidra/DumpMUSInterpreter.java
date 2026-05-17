import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.*;

public class DumpMUSInterpreter extends GhidraScript {

    static final long[] ADDRESSES = {
        0x00446B70L, // _SEQmusic          — MUS bytecode interpreter
        0x004329E0L, // ?MusicOn           — play music by name + flag
        0x00432F80L, // ?ShellMusicUpdate  — per-frame music state update
        0x00433170L, // ?ShellMusic        — music state dispatcher
        0x00432BD0L, // ?DMusicOff
        0x00432C00L, // ?MusicOff
        0x00446890L, // _SEQfadein
        0x00446910L, // _SEQfadeout
    };

    @Override
    public void run() throws Exception {
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        File outFile = new File("C:\\Temp\\mus_interpreter.txt");
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
                    out.println("// === DECOMPILE FAILED: " + funcAddr + " ===\n");
                }
            } else {
                out.println("// === NO FUNCTION AT: " + funcAddr + " ===\n");
            }
        }

        out.close();
        decompiler.dispose();
        println("Done. Output written to C:\\Temp\\mus_interpreter.txt");
    }
}
