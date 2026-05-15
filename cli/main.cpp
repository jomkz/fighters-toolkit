#include <cstdio>
#include <cstring>
#include <string>

// Forward declarations from command modules
int cmd_lib(int argc, char** argv);
int cmd_pic(int argc, char** argv);

static void print_usage() {
    puts("ft -- Fighters Toolkit\n");
    puts("Usage:  ft <command> <subcommand> [options]\n");
    puts("Library commands:");
    puts("  ft lib ls     <file.LIB>");
    puts("  ft lib unpack <file.LIB> [output_dir]");
    puts("  ft lib pack   <dir>      <output.LIB>");
    puts("  ft lib patch  <src.LIB>  <name> <file> <output.LIB>");
    puts("");
    puts("Picture commands:");
    puts("  ft pic info   <file.PIC>");
    puts("  ft pic unpack <file.PIC> [-p PALETTE.PAL] [-o output.png]");
    puts("  ft pic pack   <file.png> [-p PALETTE.PAL] [-o output.PIC]");
}

int main(int argc, char** argv) {
    if (argc < 2) { print_usage(); return 1; }

    const char* cmd = argv[1];
    if (strcmp(cmd, "lib") == 0) return cmd_lib(argc - 1, argv + 1);
    if (strcmp(cmd, "pic") == 0) return cmd_pic(argc - 1, argv + 1);

    fprintf(stderr, "Unknown command: %s\n", cmd);
    print_usage();
    return 1;
}
