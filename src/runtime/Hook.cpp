#include "Hook.h"
#include "asm/asm.h"
#include "query.h"

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#else
#error Only Linux is currently supported
#endif

bool Hook::hook() {
    if (!m_pSource || !m_pDestination) {
        return false;
    }
    if (m_isHooked) {
        return true;
    }

    constexpr size_t MIN_HOOK_SIZE = sizeof(ABSOLUTE_JMP) + sizeof(POP_RAX);
    size_t hookSize                = 0;
    while (hookSize < MIN_HOOK_SIZE) {
        InstructionInfo info;
        bool success = info.atAddr((uint8_t *)m_pSource + hookSize, 16); // arbitrary max length
        if (!success) {
            printf("[Hook] Couldn't get instruction info at %p.\n", (uint8_t *)m_pSource + hookSize);
        }
        if (info.m_usesProgramCounter) {
            printf("[Hook] Detected program counter usage in hook at %p.\n", (uint8_t *)m_pSource + hookSize);
            printf("[Hook] Warning: this could cause undefined behaviour when calling the original function.\n");
        }
        hookSize += info.m_length;
    }

    size_t trampolineSize = hookSize + sizeof(PUSH_RAX) + sizeof(ABSOLUTE_JMP);
    m_pTrampoline = mmap(NULL, trampolineSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // populate trampoline
    memcpy(m_pTrampoline, m_pSource, hookSize); // original function
    memcpy((uint8_t *)m_pTrampoline + hookSize, PUSH_RAX,
           sizeof(PUSH_RAX)); // preserve rax
    memcpy((uint8_t *)m_pTrampoline + hookSize + sizeof(PUSH_RAX), ABSOLUTE_JMP,
           sizeof(ABSOLUTE_JMP)); // jump
    // set jump address
    *(uint64_t *)((uint8_t *)m_pTrampoline + hookSize + sizeof(PUSH_RAX) + ABSOLUTE_JMP_ADDR_OFFSET) =
        (uint64_t)(uint8_t *)m_pSource + sizeof(ABSOLUTE_JMP);

    // hook function
    long page_size = sysconf(_SC_PAGESIZE);
    mprotect((uint8_t *)m_pSource - ((long)m_pSource % page_size), page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(m_pSource, ABSOLUTE_JMP, sizeof(ABSOLUTE_JMP));
    // restore RAX saved in the trampoline
    memcpy((uint8_t *)m_pSource + sizeof(ABSOLUTE_JMP), POP_RAX, sizeof(POP_RAX));
    // fill the gap with NOP
    for (uint8_t *instruction = (uint8_t *)m_pSource + sizeof(ABSOLUTE_JMP) + sizeof(POP_RAX);
         instruction < (uint8_t *)m_pSource + hookSize; ++instruction) {
        *instruction = NOP_BYTE;
    }

    // set jump address
    *(uint64_t *)((uint8_t *)m_pSource + ABSOLUTE_JMP_ADDR_OFFSET) = (uint64_t)m_pDestination;

    // restore default protection
    mprotect((uint8_t *)m_pSource - ((long)m_pSource % page_size), page_size, PROT_READ | PROT_EXEC);

    m_isHooked       = true;
    m_hookSize       = hookSize;
    m_trampolineSize = trampolineSize;

    return true;
}

bool Hook::unhook() {
    if (!m_isHooked) {
        return true;
    }
    // TODO: implement unhooking
    return false;
}
