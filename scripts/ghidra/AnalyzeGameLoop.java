// Game loop, per-frame object update dispatcher, and initialization sequence.
// Dark zone: 0x468000-0x479DFF (66 KB) -- zero prior coverage.
// Invoke: run_ghidra.bat AnalyzeGameLoop.java
// Output: %FA_PROJECT%\output\AnalyzeGameLoop.txt

public class AnalyzeGameLoop extends FAScript {

    @Override
    public void run() throws Exception {
        openOutput("AnalyzeGameLoop");

        // -----------------------------------------------------------------------
        // Entry point and initialization chain
        // -----------------------------------------------------------------------
        header("INIT -- WinMain / _main entry point");
        dumpSymbolsMatching("winmain", "_main", "maincrtstartup", "wwinmain");
        searchStrings(new String[]{"WinMain", "GameLoop", "JANES", "Fighters Anthology"});

        header("INIT -- game initialization sequence");
        dumpSymbolsMatching("initgame", "_initgame", "gameinit", "_gameinit", "init_game",
                "startgame", "_startgame", "gamestart", "_gamestart");

        // -----------------------------------------------------------------------
        // Main game loop
        // -----------------------------------------------------------------------
        header("LOOP -- main game loop body");
        dumpSymbolsMatching("gameloop", "_gameloop", "mainloop", "_mainloop",
                "frameupdate", "_frameupdate", "gametick", "_gametick");

        header("LOOP -- frame counter and timing");
        dumpSymbolsMatching("framecounter", "frametimer", "gametimer", "_gametimer",
                "timestep", "_timestep", "tickcount", "_tickcount");
        searchStrings(new String[]{"fps", "FPS", "frame"});

        // -----------------------------------------------------------------------
        // Per-frame object update dispatcher (dark zone 0x468000-0x479DFF)
        // -----------------------------------------------------------------------
        header("LOOP -- per-frame object iterator / dispatcher range 0x468000-0x479DFF");
        dumpRange(0x00468000L, 0x00479DFFL);

        // -----------------------------------------------------------------------
        // Object pool / entity manager
        // -----------------------------------------------------------------------
        header("OBJ -- object add / remove / iterate");
        dumpSymbolsMatching("_objadd", "objadd", "_objremove", "objremove",
                "_objinit", "objinit", "_objproc", "_objupdate", "objupdate",
                "_objlist", "objlist", "_entityadd", "entityadd");

        header("OBJ -- callers of _GVProc (vehicle per-frame dispatch)");
        dumpCallers(0x00473db0L);

        header("OBJ -- callers of _PROJProc (missile per-frame dispatch)");
        dumpCallers(0x004c1f50L);

        // -----------------------------------------------------------------------
        // Init / shutdown range 0x401000-0x406000
        // -----------------------------------------------------------------------
        header("INIT -- range 0x401000-0x406000 (entry point cluster)");
        dumpRange(0x00401000L, 0x00406000L);

        // -----------------------------------------------------------------------
        // Campaign / mission state init
        // -----------------------------------------------------------------------
        header("INIT -- campaign and mission init");
        dumpSymbolsMatching("_missioninit", "missioninit", "_campaigninit", "campaigninit",
                "_missionstart", "missionstart", "_levelload", "levelload");

        // -----------------------------------------------------------------------
        // Shutdown / cleanup
        // -----------------------------------------------------------------------
        header("SHUTDOWN -- game shutdown and cleanup");
        dumpSymbolsMatching("shutdown", "_shutdown", "gameshutdown", "_gameshutdown",
                "cleanup", "_cleanup", "exitgame", "_exitgame");

        closeOutput();
    }
}
