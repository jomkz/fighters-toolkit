// Multiplayer networking protocol, CN_INFO struct mapping, and IP.EXE interface.
// Dark zone: 0x482200-0x4AACEF (169 KB, shared with mission eval) -- zero prior coverage.
// Invoke: run_ghidra.bat AnalyzeNetwork.java
// Output: %FA_PROJECT%\output\AnalyzeNetwork.txt

public class AnalyzeNetwork extends FAScript {

    @Override
    public void run() throws Exception {
        openOutput("AnalyzeNetwork");

        // -----------------------------------------------------------------------
        // CN_INFO struct -- config read/write (EA.CFG, NET.DAT)
        // -----------------------------------------------------------------------
        header("CN -- CN_ReadConfig / CN_WriteConfig (CN_INFO struct mapping)");
        dumpSymbolsMatching("cn_readconfig", "_cn_readconfig", "cn_writeconfig", "_cn_writeconfig",
                "cnreadconfig", "cnwriteconfig", "cn_init", "_cn_init", "cninit");

        header("CN -- CN_INFO struct field scan (offsets 0x00-0x200)");
        findFunctionsReadingOffsets(0x00400000L, 0x00540000L, 0x00, 0x200);

        // -----------------------------------------------------------------------
        // Multiplayer frame sync and receive loop
        // -----------------------------------------------------------------------
        header("MP -- ?MPReceive@@YGDXZ (0x46C980) and callers");
        dumpAt(0x0046c980L);
        dumpCallers(0x0046c980L);

        header("MP -- multiplayer state and symbols");
        dumpSymbolsMatching("mpreceive", "mpsend", "mpupdate", "mpinit", "mpshutdown",
                "mpframe", "mpsync", "mpstatus", "mpstatus", "mpsession",
                "mpjoin", "mphost", "mpstart", "mpend", "mpkill",
                "?mpset", "?mpget", "?mpstatus", "?mprecv", "?mpsend",
                "_mprecv", "_mpsend", "_mpupdate", "_mpinit");

        header("MP -- range 0x482200-0x4AACEF (MP / mission eval dark zone)");
        dumpRange(0x00482200L, 0x004aacefL);

        // -----------------------------------------------------------------------
        // Transport layer (IPX, TCP, serial, modem)
        // -----------------------------------------------------------------------
        header("NET -- IPX transport");
        dumpSymbolsMatching("_ipxsend", "ipxsend", "_ipxrecv", "ipxrecv",
                "_ipxinit", "ipxinit", "_ipxopen", "ipxopen",
                "_ipxclose", "ipxclose", "_ipxpoll", "ipxpoll");
        searchStrings(new String[]{"IPX", "ipx", "Novell", "novell"});

        header("NET -- TCP/IP transport");
        dumpSymbolsMatching("_tcpsend", "tcpsend", "_tcprecv", "tcprecv",
                "_tcpinit", "tcpinit", "_tcpopen", "tcpopen",
                "_tcpclose", "tcpclose", "_tcpconnect", "tcpconnect");
        searchStrings(new String[]{"TCP", "tcp", "socket", "Socket", "winsock", "WinSock"});

        header("NET -- serial / modem transport");
        dumpSymbolsMatching("_serialsend", "serialsend", "_serialrecv", "serialrecv",
                "_modemsend", "modemsend", "_modemrecv", "modemrecv",
                "_comminit", "comminit", "_commopen", "commopen",
                "_commsend", "commsend", "_commrecv", "commrecv");
        searchStrings(new String[]{"COM", "modem", "Modem", "serial", "Serial"});

        // -----------------------------------------------------------------------
        // Packet format and protocol state machine
        // -----------------------------------------------------------------------
        header("NET -- packet encode / decode");
        dumpSymbolsMatching("_packetinit", "packetinit", "_packencode", "packencode",
                "_packdecode", "packdecode", "_packetsend", "packetsend",
                "_packetrecv", "packetrecv", "_netpacket", "netpacket");

        header("NET -- network session management");
        dumpSymbolsMatching("_netsession", "netsession", "_sessioninit", "sessioninit",
                "_sessionjoin", "sessionjoin", "_sessionhost", "sessionhost",
                "_sessionlist", "sessionlist", "_sessionclose", "sessionclose");
        searchStrings(new String[]{"session", "Session", "lobby", "Lobby", "host", "join"});

        // -----------------------------------------------------------------------
        // Player sync (entity state broadcast)
        // -----------------------------------------------------------------------
        header("MP -- player/entity sync over network");
        dumpSymbolsMatching("mpsetfuel", "?mpsetfuel", "mpsetpos", "mpsetstate",
                "mpsetweapon", "mpsetdamage", "mpentityupdate", "mpplayerupdate");

        closeOutput();
    }
}
