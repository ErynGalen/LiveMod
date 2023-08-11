#include "query.h"

#include "GlobalContext.h"
#include "dlfcn.h"
#include "process.h"
#include <filesystem>
#include <stdio.h>

void *query_symbol(void *handle, const char *name) {
    void *addr = dlsym(handle, name);

    // prevents infinite recursion when `query_symbol` needs a hooked function
    if (g_Context.m_isNative) {
        return addr;
    }
    MakeNativeGuard ng;

    // handle non-dynamic symbols

    // get list of symbols in binary
    const char *exePath = std::filesystem::canonical("/proc/self/exe").c_str();
    char *command       = (char *)"nm";
    const char *args[]  = {command, (char *)"--demangle", "-j", exePath, nullptr};
    std::string symbols = execAndGet(command, (char *const *)args);

    printf("!!!%s!!!\n", symbols.c_str());
    // get symbol address?

    return addr;
}
