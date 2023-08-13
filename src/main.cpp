#include <iostream>
#include <string>
#include <vector>

void printUsage(const char *name) {
    std::cout << "Usage:\n";
    std::cout << name << " <library> <command> [args]\n";
    std::cout << "    <library>: library to preload/to inject\n";
    std::cout << "    <command>: command to launch" << std::endl;
}

/// example: launch("runtime/libruntime.so", ["echo", "a", "b"], 3);
int launch(const char *library, char *commandParts[], int partCount);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        exit(1);
    }
    const char *library = argv[1];
    const int partCount = argc - 2;
    char **arguments    = (char **)malloc(partCount * sizeof(const char *));

    arguments[0] = argv[2];
    for (int partN = 1; partN <= partCount; partN++) {
        arguments[partN] = argv[partN + 2];
    }

    return launch(library, arguments, partCount);
}

#ifdef __linux__
#include <cstdlib>
#include <filesystem>
#include <unistd.h>

int launch(const char *library, char *commandParts[], int partCount) {
    const char *_old_ld_preload  = getenv("LD_PRELOAD");
    std::string libraryAbsolute = std::filesystem::canonical(library).string();
    std::cout << "[LiveMod] Launching with library `" << libraryAbsolute << "`" << std::endl;

    setenv("LD_PRELOAD", libraryAbsolute.c_str(), 1);

    commandParts            = (char **)realloc((void *)commandParts, partCount + sizeof(char *));
    commandParts[partCount] = nullptr;

    execvp(commandParts[0], commandParts);
    exit(42); // shouldn't happen
}
#else
#error Only linux is supported at the moment
#endif
