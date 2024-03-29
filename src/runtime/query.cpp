#include "query.h"
#define MODULE "Query"

#include "GlobalContext.h"
#include "log.h"
#include "process.h"
#include <cstdint>
#include <filesystem>
#include <stdio.h>
#include <string>
#include <vector>

#ifdef __linux__
#include <dlfcn.h>
#else
#error Only linux is currently supported
#endif

struct Symbol {
    std::string m_name;
    uint8_t *m_address = nullptr;
    bool fromNmLine(std::string line);
};

// get list of symbols in binary
std::vector<Symbol> getSymbolList(bool force) {
    static std::vector<Symbol> symbols;
    // cache the result
    if (!force && !symbols.empty()) {
        return symbols;
    }

    std::string exePath     = std::filesystem::canonical("/proc/self/exe").string();
    char *command           = (char *)"llvm-nm";
    const char *args[]      = {command, (char *)"--demangle", exePath.c_str(), nullptr};
    std::string symbols_str = execAndGet(command, (char *const *)args);

    size_t sym_table_pos = 0;
    for (;;) {
        size_t next_pos = symbols_str.find("\n", sym_table_pos);
        if (next_pos == std::string::npos) {
            break;
        }
        Symbol current_symbol;
        if (current_symbol.fromNmLine(symbols_str.substr(sym_table_pos, next_pos - sym_table_pos))) {
            symbols.push_back(current_symbol);
        }
        sym_table_pos = next_pos + 1;
    }

    return symbols;
}

struct MemMap {
    uint8_t *m_baseAddres = nullptr;
    std::string m_file; // file mapped to the memory
    bool fromProcMapsLine(std::string line);
};

std::vector<MemMap> getMapList(bool force) {
    static std::vector<MemMap> maps;
    // cache result
    if (!force && !maps.empty()) {
        return maps;
    }

    FILE *maps_file = fopen("/proc/self/maps", "r");
    if (maps_file) {
        char buffer[256] = {0};
        while (fgets(buffer, sizeof(buffer), maps_file)) {
            MemMap current_map;
            std::string line(buffer);
            if (current_map.fromProcMapsLine(line)) {
                maps.push_back(current_map);
            }
        }
    } else {
        LOGS("Couldn't open /proc/self/maps");
    }

    return maps;
}

void *querySymbol(void *handle, const char *name) {
    void *addr = dlsym(handle, name);
    if (addr) {
        return addr; // found dynamic symbol
    }
    // prevents infinite recursion when `query_symbol` needs a hooked function
    if (g_Context.m_isNative) {
        return addr;
    }
    MakeNativeGuard ng;

    // handle non-dynamic symbols

    std::vector symbols = getSymbolList(false);

    // get the base address of the process in the virtual address space
    uint8_t *base_address = nullptr;

    std::string exePath = std::filesystem::canonical("/proc/self/exe").string();
    std::vector maps    = getMapList(false);
    for (auto &map : maps) {
        if (map.m_file == exePath) {
            if (base_address == nullptr || map.m_baseAddres < base_address) {
                base_address = map.m_baseAddres;
            }
        }
    }

    if (base_address == nullptr) {
        LOGS("Coudln't determine process base address");
        return addr;
    }

    // look for symbol in the list
    for (Symbol &s : symbols) {
        if (s.m_name == name) {
            LOG("Info: found %p for non-dynamic `%s`", base_address + (uint64_t)s.m_address, name);
            return base_address + (uint64_t)s.m_address;
        }
    }

    return addr;
}

bool Symbol::fromNmLine(std::string line) {
    size_t after_address = line.find(" ");
    if (after_address == std::string::npos) {
        return false;
    }
    size_t after_type = line.find(" ", after_address + 1);
    if (after_type - after_address != 2) {
        // the type should only be one character wide
        return false;
    }
    size_t start_name = line.find_first_not_of(" ", after_type);

    m_name = line.substr(start_name);
    unsigned long long int address;
    try {
        address = std::stoull(line.substr(0, after_address), nullptr, 16);
    } catch (...) {
        return false;
    }
    m_address = (uint8_t *)address;

    return true;
}

bool MemMap::fromProcMapsLine(std::string line) {
    size_t after_base_address = line.find("-");
    size_t after_end_address  = line.find(" ", after_base_address + 1);
    size_t after_mode         = line.find(" ", after_end_address + 1);
    size_t after_offset       = line.find(" ", after_mode + 1);
    size_t after_device       = line.find(" ", after_offset + 1);
    size_t after_inode        = line.find(" ", after_device + 1);
    size_t start_map_name     = line.find_first_not_of(" ", after_inode);
    size_t after_map_name     = line.find_first_of(" \n", start_map_name);
    if (after_base_address == std::string::npos || after_end_address == std::string::npos ||
        after_mode == std::string::npos || after_offset == std::string::npos || after_device == std::string::npos ||
        after_inode == std::string::npos || start_map_name == std::string::npos) {
        return false;
    }
    m_file = line.substr(start_map_name, after_map_name - start_map_name);
    unsigned long long base_address;
    try {
        base_address = std::stoull(line.substr(0, after_base_address), nullptr, 16);
    } catch (...) {
        return false;
    }
    m_baseAddres = (uint8_t *)base_address;

    return true;
}
