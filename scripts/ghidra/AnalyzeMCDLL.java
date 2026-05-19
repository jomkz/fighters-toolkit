// Mission condition DLL logic analysis.
// Runs against %FA_PROJECT%\overlay_projects\mc\
// For each MC DLL: dumps the exported entry point, traces all condition-check
// paths, and identifies: which object aliases are tested, which conditions
// trigger mission success or failure, and what time thresholds are used.
// Also traces FUN_00495e80 to clarify its role in the dispatch chain.
//
// Invoke via run_overlays.bat --analyze MC
// Output: %FA_PROJECT%\output\AnalyzeMCDLL.txt

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class AnalyzeMCDLL extends GhidraScript {

    private DecompInterface decompiler;
    private PrintWriter out;
    private Set<Long> dumped;

    @Override
    public void run() throws Exception {
        String projectDir = System.getenv("FA_PROJECT");
        if (projectDir == null || projectDir.isEmpty())
            projectDir = System.getProperty("java.io.tmpdir");
        File outDir = new File(projectDir, "output");
        outDir.mkdirs();
        File outFile = new File(outDir, "AnalyzeMCDLL.txt");

        decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        dumped = new LinkedHashSet<>();
        out = new PrintWriter(new FileWriter(outFile, true));  // append: one file per project run

        out.println("// ============================================================");
        out.println("// AnalyzeMCDLL -- mission condition DLL logic");
        out.println("// Program: " + currentProgram.getName());
        out.println("// Image base: 0x" + Long.toHexString(
                currentProgram.getImageBase().getOffset()));
        out.println("// ============================================================");
        out.println();

        // -----------------------------------------------------------------------
        // Exported entry point (condition evaluator)
        // -----------------------------------------------------------------------
        header("MC -- exported entry point(s)");
        SymbolTable st = currentProgram.getSymbolTable();
        FunctionManager fm = currentProgram.getFunctionManager();
        List<Long> exportVas = new ArrayList<>();
        for (Symbol sym : st.getAllSymbols(false)) {
            if (sym.isExternalEntryPoint()) {
                Function fn = fm.getFunctionAt(sym.getAddress());
                if (fn != null) {
                    out.println("// EXPORT: " + sym.getName() + " @ " + sym.getAddress());
                    exportVas.add(fn.getEntryPoint().getOffset());
                    dumpAt(fn.getEntryPoint().getOffset());
                }
            }
        }

        // -----------------------------------------------------------------------
        // Condition-check call trace from each export
        // -----------------------------------------------------------------------
        header("MC -- condition-check paths (callees of exported entry)");
        for (long exportVa : exportVas) {
            Function exportFn = fm.getFunctionAt(toAddr(exportVa));
            if (exportFn == null) continue;
            out.println("// Callees of " + exportFn.getName() + ":");
            for (Function callee : exportFn.getCalledFunctions(monitor)) {
                out.println("//   " + callee.getName() + " @ " + callee.getEntryPoint());
                dumpAt(callee.getEntryPoint().getOffset());
            }
        }

        // -----------------------------------------------------------------------
        // Dispatch chain function (from FA.EXE -- may appear as import thunk)
        // -----------------------------------------------------------------------
        header("MC -- dispatch chain FUN_00495e80 (if present as thunk)");
        long dispatchVa = 0x00495e80L;
        Function dispatch = fm.getFunctionAt(toAddr(dispatchVa));
        if (dispatch != null) {
            dumpAt(dispatchVa);
        } else {
            out.println("// FUN_00495e80 not present in this overlay (it lives in FA.EXE)");
            out.println("// Look for it as an import reference / external call in the export body.");
        }

        // -----------------------------------------------------------------------
        // Scan for known mission-outcome call patterns
        // -----------------------------------------------------------------------
        header("MC -- mission outcome markers (string scan)");
        searchStringsLocal(new String[]{
            "SUCCESS", "FAILED", "Mission", "success", "failed",
            "MISSIONSuccess", "campaignFailed", "OBJAlias",
            "_currentTime", "currentTime", "time", "Time"
        });

        // -----------------------------------------------------------------------
        // All remaining functions (VA order)
        // -----------------------------------------------------------------------
        header("MC -- all functions (VA order)");
        List<Function> allFns = new ArrayList<>();
        for (Function fn : fm.getFunctions(true)) allFns.add(fn);
        allFns.sort((a, b) -> Long.compare(
                a.getEntryPoint().getOffset(), b.getEntryPoint().getOffset()));
        for (Function fn : allFns) {
            dumpAt(fn.getEntryPoint().getOffset());
        }

        out.println("\n// Total functions dumped: " + dumped.size());
        out.close();
        decompiler.dispose();
        println("AnalyzeMCDLL complete -> " + outFile.getAbsolutePath());
    }

    private void searchStringsLocal(String[] patterns) throws Exception {
        Memory mem = currentProgram.getMemory();
        ReferenceManager rm = currentProgram.getReferenceManager();
        FunctionManager fm = currentProgram.getFunctionManager();
        Address searchStart = currentProgram.getMinAddress();
        Address searchEnd   = currentProgram.getMaxAddress();

        for (String kw : patterns) {
            byte[] kwBytes = kw.getBytes("US-ASCII");
            Address found = mem.findBytes(searchStart, searchEnd, kwBytes, null, true, monitor);
            while (found != null) {
                out.println("// string '" + kw + "' at " + found);
                for (Reference ref : rm.getReferencesTo(found)) {
                    Function f = fm.getFunctionContaining(ref.getFromAddress());
                    if (f != null && dumped.add(f.getEntryPoint().getOffset())) {
                        out.println("//   ref from " + ref.getFromAddress()
                                + " in " + f.getName());
                        dumpAt(f.getEntryPoint().getOffset());
                    }
                }
                found = mem.findBytes(found.add(1), searchEnd, kwBytes, null, true, monitor);
            }
        }
    }

    private void header(String title) {
        out.println("\n// === " + title + " ===");
    }

    private void dumpAt(long va) throws Exception {
        if (dumped.contains(va)) return;
        dumped.add(va);
        FunctionManager fm = currentProgram.getFunctionManager();
        Function fn = fm.getFunctionAt(toAddr(va));
        if (fn == null) fn = fm.getFunctionContaining(toAddr(va));
        if (fn == null) {
            out.println("// NOT FOUND at 0x" + Long.toHexString(va));
            return;
        }
        out.println("// --- " + fn.getName() + " @ " + fn.getEntryPoint() + " ---");
        DecompileResults res = decompiler.decompileFunction(fn, 120, monitor);
        if (res != null && res.getDecompiledFunction() != null) {
            out.println(res.getDecompiledFunction().getC());
        } else {
            out.println("// decompile failed: " + fn.getName());
        }
        out.println();
    }

}
