import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;
import java.io.*;

public class DumpDLGFunctions extends GhidraScript {

    static final long[] ADDRESSES = {
        0x004897F0L, // _ChoosePreload
        0x00489AC0L, // _DrawText
        0x00489B90L, // _DrawAction
        0x0048A910L, // _DrawFormattedText
        0x0048ABF0L, // _DrawCampaignList
        0x0048B4E0L, // _DrawRocker
        0x0048C710L, // _DrawEditBox
    };

    @Override
    public void run() throws Exception {
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        File outFile = new File("C:\\Temp\\dlg_functions.txt");
        outFile.getParentFile().mkdirs();
        PrintWriter out = new PrintWriter(new FileWriter(outFile));

        for (long addr : ADDRESSES) {
            Address funcAddr = currentProgram.getAddressFactory()
                .getDefaultAddressSpace().getAddress(addr);

            Function func = getFunctionAt(funcAddr);
            if (func == null) {
                func = currentProgram.getFunctionManager().getFunctionContaining(funcAddr);
            }
            if (func == null) {
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
                out.println("// === NO FUNCTION AT: " + funcAddr + " ===\n");
            }
        }

        out.close();
        decompiler.dispose();
        println("Done. Output written to C:\\Temp\\dlg_functions.txt");
    }
}
