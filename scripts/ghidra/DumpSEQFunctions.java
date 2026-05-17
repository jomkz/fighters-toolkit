import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;
import java.io.*;

public class DumpSEQFunctions extends GhidraScript {

    static final long[] ADDRESSES = {
        0x00446B70L, // _SEQmusic
        0x00446890L, // _SEQfadein
        0x00446910L, // _SEQfadeout
        0x00446990L, // ?SeqFadeOut
        0x004469F0L, // ?SeqFadeIn
        0x004328B0L, // ?InitMusic
        0x00432920L, // ?ShutDownMidi
        0x00432A80L, // ?DMusicVolume
        0x00432B40L, // ?MusicVolume
    };

    @Override
    public void run() throws Exception {
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        File outFile = new File("C:\\Temp\\seq_functions.txt");
        outFile.getParentFile().mkdirs();
        PrintWriter out = new PrintWriter(new FileWriter(outFile));

        for (long addr : ADDRESSES) {
            Address funcAddr = currentProgram.getAddressFactory()
                .getDefaultAddressSpace().getAddress(addr);

            Function func = getFunctionAt(funcAddr);
            if (func == null) {
                // force disassemble + create function
                DisassembleCommand disCmd = new DisassembleCommand(funcAddr, null, true);
                disCmd.applyTo(currentProgram, monitor);
                CreateFunctionCmd createCmd = new CreateFunctionCmd(funcAddr);
                createCmd.applyTo(currentProgram, monitor);
                func = getFunctionAt(funcAddr);
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
                out.println("// === COULD NOT CREATE FUNCTION AT: " + funcAddr + " ===\n");
            }
        }

        out.close();
        decompiler.dispose();
        println("Done. Output written to C:\\Temp\\seq_functions.txt");
    }
}
