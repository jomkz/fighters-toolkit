// Generic overlay DLL dump -- runs against any overlay Ghidra project.
// Iterates all functions, groups by export name (if available) then by VA order.
// When zero functions are found (data-only DLL), falls back to raw memory block
// scan: hex+ASCII dump of every PE section plus null-terminated string index.
// Accepts the DLL short name as a script argument for the output filename.
//
// Usage (headless):
//   analyzeHeadless.bat %FA_PROJECT%\overlay_projects\<format> <ProjectName>
//       -process <DLL> -scriptPath scripts\ghidra -postScript DumpOverlayDLL.java <DllName>
//
// Invoke via run_overlays.bat --analyze <DllName>
// Output: %FA_PROJECT%\output\Overlay_<DllName>.txt

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class DumpOverlayDLL extends GhidraScript {

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        String dllName = (args != null && args.length > 0 && !args[0].isEmpty())
                ? args[0] : currentProgram.getName();

        String projectDir = System.getenv("FA_PROJECT");
        if (projectDir == null || projectDir.isEmpty())
            projectDir = System.getProperty("java.io.tmpdir");
        File outDir = new File(projectDir, "output");
        outDir.mkdirs();
        File outFile = new File(outDir, "Overlay_" + dllName + ".txt");

        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        // Append mode: one file accumulates all DLLs in the project
        PrintWriter out = new PrintWriter(new FileWriter(outFile, true));
        out.println("// ============================================================");
        out.println("// DLL: " + currentProgram.getName()
                + "  [format: " + dllName + "]");
        out.println("// Image base: 0x"
                + Long.toHexString(currentProgram.getImageBase().getOffset()));
        out.println("// ============================================================");
        out.println();

        FunctionManager fm = currentProgram.getFunctionManager();
        SymbolTable st = currentProgram.getSymbolTable();

        // Collect exported symbols
        Set<Long> exported = new LinkedHashSet<>();
        for (Symbol sym : st.getAllSymbols(false)) {
            if (sym.isExternalEntryPoint()) {
                Function fn = fm.getFunctionAt(sym.getAddress());
                if (fn != null) exported.add(fn.getEntryPoint().getOffset());
            }
        }

        Set<Long> dumped = new LinkedHashSet<>();

        // Dump exports first
        if (!exported.isEmpty()) {
            out.println("// === EXPORTS ===");
            for (long va : exported) {
                if (dumped.contains(va)) continue;
                dumped.add(va);
                dumpFunction(currentProgram, fm, decompiler, out, va);
            }
        }

        // Dump all functions in VA order
        List<Function> allFns = new ArrayList<>();
        for (Function fn : fm.getFunctions(true)) allFns.add(fn);
        allFns.sort((a, b) -> Long.compare(
                a.getEntryPoint().getOffset(), b.getEntryPoint().getOffset()));

        if (!allFns.isEmpty()) {
            out.println("// === ALL FUNCTIONS (VA order) ===");
            for (Function fn : allFns) {
                long va = fn.getEntryPoint().getOffset();
                if (dumped.contains(va)) continue;
                dumped.add(va);
                dumpFunction(currentProgram, fm, decompiler, out, va);
            }
            out.println("// Total functions: " + dumped.size());
        } else {
            // Data-only DLL: dump all memory blocks as hex + string index
            out.println("// === DATA SECTIONS (no executable code found) ===");
            dumpDataSections(out);
        }

        out.println();
        out.close();
        decompiler.dispose();
        println("DumpOverlayDLL [" + dllName + "]: " + dumped.size()
                + " functions, program=" + currentProgram.getName()
                + " -> " + outFile.getAbsolutePath());
    }

    private void dumpFunction(Program prog, FunctionManager fm,
                               DecompInterface decompiler, PrintWriter out,
                               long va) throws Exception {
        Function fn = fm.getFunctionAt(prog.getAddressFactory()
                .getDefaultAddressSpace().getAddress(va));
        if (fn == null) return;
        out.println("// --- " + fn.getName() + " @ 0x" + Long.toHexString(va) + " ---");
        DecompileResults res = decompiler.decompileFunction(fn, 120, monitor);
        if (res != null && res.getDecompiledFunction() != null) {
            out.println(res.getDecompiledFunction().getC());
        } else {
            out.println("// decompile failed: " + fn.getName());
        }
        out.println();
    }

    private void dumpDataSections(PrintWriter out) throws Exception {
        Memory mem = currentProgram.getMemory();
        for (MemoryBlock block : mem.getBlocks()) {
            if (!block.isInitialized()) continue;
            long start = block.getStart().getOffset();
            long end   = block.getEnd().getOffset();
            int  size  = (int)(end - start + 1);
            out.println("// --- section: " + block.getName()
                    + "  offset=0x" + Long.toHexString(start)
                    + "  size=0x" + Integer.toHexString(size)
                    + "  flags=" + sectionFlags(block) + " ---");

            byte[] data = new byte[size];
            block.getBytes(block.getStart(), data);

            // Hex + ASCII dump (16 bytes per row)
            StringBuilder hex = new StringBuilder();
            StringBuilder asc = new StringBuilder();
            for (int i = 0; i < data.length; i++) {
                if (i % 16 == 0) {
                    if (i > 0) {
                        out.println(String.format("  %04x: %-48s  %s",
                                i - 16, hex.toString(), asc.toString()));
                    }
                    hex.setLength(0);
                    asc.setLength(0);
                }
                int b = data[i] & 0xFF;
                hex.append(String.format("%02x ", b));
                asc.append((b >= 0x20 && b < 0x7f) ? (char)b : '.');
            }
            // Last row
            if (hex.length() > 0) {
                int rowStart = (data.length / 16) * 16;
                out.println(String.format("  %04x: %-48s  %s",
                        rowStart, hex.toString(), asc.toString()));
            }

            // String index: null-terminated ASCII strings >= 4 chars
            out.println();
            out.println("  // Strings in " + block.getName() + ":");
            int slen = 0;
            int sstart = 0;
            boolean anyString = false;
            for (int i = 0; i <= data.length; i++) {
                byte b = (i < data.length) ? data[i] : 0;
                if (b >= 0x20 && b < 0x7f) {
                    if (slen == 0) sstart = i;
                    slen++;
                } else {
                    if (slen >= 4) {
                        String s = new String(data, sstart, slen, "ASCII");
                        out.println(String.format("    0x%04x  \"%s\"",
                                start + sstart, s));
                        anyString = true;
                    }
                    slen = 0;
                }
            }
            if (!anyString) out.println("    (none)");
            out.println();
        }
    }

    private static String sectionFlags(MemoryBlock b) {
        StringBuilder sb = new StringBuilder();
        if (b.isRead())    sb.append('R');
        if (b.isWrite())   sb.append('W');
        if (b.isExecute()) sb.append('X');
        return sb.length() > 0 ? sb.toString() : "-";
    }
}
