#include "Hook.h"
#include "asm/asm.h"
#include "dlhook.h"
#include "query.h"

#include <cstdint>
#include <dlfcn.h>
#include <memory_resource>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <llvm/MC/MCAssembler.h>

int loop() {
    while (true) {
        // do stuff
    }
    return 0;
}

bool Hook::hook() {
    if (!m_pSource || !m_pDestination) {
        return false;
    }

    constexpr size_t MIN_HOOK_SIZE = sizeof(ABSOLUTE_JMP) + sizeof(POP_RAX);
    size_t hookSize = 0;
    while (hookSize < MIN_HOOK_SIZE) {
        hookSize += instruction_length((uint8_t *)m_pSource + hookSize);
        printf("%d, ", (int)hookSize);
    }
    printf("\n");
    hookSize += 4; // margin because `instruction_length` isn't correct
    size_t trampolineSize = hookSize + sizeof(PUSH_RAX) + sizeof(ABSOLUTE_JMP);
    m_pTrampoline =
        mmap(NULL, trampolineSize, PROT_READ | PROT_WRITE | PROT_EXEC,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // populate trampoline
    memcpy(m_pTrampoline, m_pSource, hookSize); // original function
    memcpy((uint8_t *)m_pTrampoline + hookSize, PUSH_RAX,
           sizeof(PUSH_RAX)); // preserve rax
    memcpy((uint8_t *)m_pTrampoline + hookSize + sizeof(PUSH_RAX), ABSOLUTE_JMP,
           sizeof(ABSOLUTE_JMP)); // jump
    // set jump address
    *(uint64_t *)((uint8_t *)m_pTrampoline + hookSize + sizeof(PUSH_RAX) +
                  ABSOLUTE_JMP_ADDR_OFFSET) =
        (uint64_t)(uint8_t *)m_pSource + sizeof(ABSOLUTE_JMP);

    // hook function
    long page_size = sysconf(_SC_PAGESIZE);
    mprotect((uint8_t *)m_pSource - ((long)m_pSource % page_size), page_size,
             PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(m_pSource, ABSOLUTE_JMP, sizeof(ABSOLUTE_JMP));
    // restore RAX saved in the trampoline
    memcpy((uint8_t *)m_pSource + sizeof(ABSOLUTE_JMP), POP_RAX,
           sizeof(POP_RAX));
    // fill the gap with NOP
    for (uint8_t *instruction =
             (uint8_t *)m_pSource + sizeof(ABSOLUTE_JMP) + sizeof(POP_RAX);
         instruction < (uint8_t *)m_pSource + hookSize; ++instruction) {
        *instruction = NOP_BYTE;
    }

    // set jump address
    *(uint64_t *)((uint8_t *)m_pSource + ABSOLUTE_JMP_ADDR_OFFSET) =
        (uint64_t)m_pDestination;

    // restore default protection
    mprotect((uint8_t *)m_pSource - ((long)m_pSource % page_size), page_size,
             PROT_READ | PROT_EXEC);

    m_isHooked = true;
    m_hookSize = hookSize;
    m_trampolineSize = trampolineSize;

    return true;
}

bool Hook::unhook() {
    if (!m_isHooked) {
        return true;
    }
    return false;
}
