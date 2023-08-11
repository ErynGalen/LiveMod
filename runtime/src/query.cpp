#include "query.h"

#include "GlobalContext.h"
#include "dlfcn.h"
#include "process.h"
#include <cstdint>
#include <filesystem>
#include <stdio.h>
#include <string>
#include <vector>

struct Symbol {
    std::string m_name;
    uint8_t *m_address = nullptr;
    bool fromObjdumpLine(std::string line);
};

struct MemMap {
    uint8_t *m_baseAddres = nullptr;
    std::string m_file; // file mapped to the memory
    bool fromProcMapsLine(std::string line);
};

void *query_symbol(void *handle, const char *name) {
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

    // get list of symbols in binary
    std::vector<Symbol> symbols;

    std::string exePath     = std::filesystem::canonical("/proc/self/exe").string();
    char *command           = (char *)"objdump";
    const char *args[]      = {command, (char *)"--demangle", "--syms", exePath.c_str(), nullptr};
    std::string symbols_str = execAndGet(command, (char *const *)args);

    size_t sym_table_pos = 0;
    for (int i = 0; i < 4; i++) { // skip 4 first lines
        sym_table_pos = symbols_str.find("\n", sym_table_pos) + 1;
        if (sym_table_pos == std::string::npos) {
            printf("Unknown symbols list format:\n%s\n", symbols_str.c_str());
            break;
        }
    }
    for (;;) {
        size_t next_pos = symbols_str.find("\n", sym_table_pos);
        if (next_pos == std::string::npos) {
            break;
        }
        Symbol current_symbol;
        if (current_symbol.fromObjdumpLine(symbols_str.substr(sym_table_pos, next_pos - sym_table_pos))) {
            symbols.push_back(current_symbol);
        }
        sym_table_pos = next_pos + 1;
    }

    // get the base address of the process in the virtual address space
    uint8_t *base_address = nullptr;

    FILE *maps_file = fopen("/proc/self/maps", "r");
    if (maps_file) {
        char buffer[256] = {0};
        while (fgets(buffer, sizeof(buffer), maps_file)) {
            MemMap current_map;
            std::string line(buffer);
            if (current_map.fromProcMapsLine(line)) {
                if (current_map.m_file == exePath) {
                    if (base_address == nullptr || current_map.m_baseAddres < base_address) {
                        base_address = current_map.m_baseAddres;
                    }
                }
            }
        }
    } else {
        printf("Couldn't open /proc/self/maps\n");
        return addr;
    }

    if (base_address == nullptr) {
        printf("Couldn't determine process base address\n");
        return addr;
    }

    // look for symbol in the list
    for (Symbol &s : symbols) {
        if (s.m_name == name) {
            printf("found static %p for %s\n", base_address + (uint64_t)s.m_address, name);
            return base_address + (uint64_t)s.m_address;
        }
    }

    return addr;
}

bool Symbol::fromObjdumpLine(std::string line) {
    size_t after_address      = line.find(" ");
    size_t after_flags        = after_address + 8;
    size_t after_section_name = line.find(" ", after_flags + 1);
    size_t after_symbol_size  = line.find(" ", after_section_name + 1);
    size_t start_symbol_name  = line.find_first_not_of(" ", after_symbol_size);
    size_t after_symbol_name  = line.find_first_of(" \n", start_symbol_name);
    if (after_address == std::string::npos || after_flags == std::string::npos ||
        after_section_name == std::string::npos || after_symbol_size == std::string::npos ||
        start_symbol_name == std::string::npos) {
        return false;
    }
    m_name                         = line.substr(start_symbol_name);
    unsigned long long int address = std::stoull(line.substr(0, after_address), nullptr, 16);
    m_address                      = (uint8_t *)address;

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
    m_file                          = line.substr(start_map_name, after_map_name - start_map_name);
    unsigned long long base_address = std::stoull(line.substr(0, after_base_address), nullptr, 16);
    m_baseAddres                    = (uint8_t *)base_address;

    return true;
}
