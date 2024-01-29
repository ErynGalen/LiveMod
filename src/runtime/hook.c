#include "hook.h"
#include "log.h"
#define MODULE "Hook"

#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <capstone/capstone.h>

#define THIS_ARCH CS_ARCH_X86
#define THIS_MODE CS_MODE_64

int disasm_create(disasm_t *disasm) {
    if (cs_open(THIS_ARCH, THIS_MODE, &disasm->handle) != CS_ERR_OK) {
        return 1;
    }
    disasm->insn = cs_malloc(disasm->handle);
    if (disasm->insn == NULL) {
        return 2;
    }
    return 0;
}
void disasm_destroy(disasm_t *disasm) {
    cs_free(disasm->insn, 1);
    cs_close(&disasm->handle);
}


static uint8_t ABSOLUTE_JMP[] = {0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xE0
};
static size_t ABSOLUTE_JMP_ADDR_OFFSET = 2;

static uint8_t PUSH_RAX[] = {0x50};

static uint8_t POP_RAX[] = {0x58};
static uint8_t NOP_BYTE  = 0x90;



int hook(disasm_t *disasm, void *src, size_t max_size, void *dest, hook_t *result) {
    if (src == NULL || dest == NULL) {
        return 1;
    }
    uint8_t *source = src;
    uint8_t *destination = dest;
    result->source = source;
    result->destination = destination;
    LOG("hooking %p with %p", source, destination);

    const size_t min_hook_size = sizeof(ABSOLUTE_JMP) + sizeof(POP_RAX);
    size_t hook_size = 0;
    uint64_t source_addr = (uint64_t) source;
    const uint8_t *target = source; // this is gonna be modified by `cs_disasm_iter`
    while (hook_size < min_hook_size) {
        if (cs_disasm_iter(disasm->handle, &target, &max_size, &source_addr, disasm->insn)) {
            hook_size += disasm->insn->size;
            LOG("disassembled with op_str %s", disasm->insn->op_str);
            // TODO: check for program counter usage (%rip)
        } else {
            return 2;
        }
    } 
    result->hook_size = hook_size;

    size_t trampoline_size = hook_size + sizeof(PUSH_RAX) + sizeof(ABSOLUTE_JMP);
    result->trampoline_size = trampoline_size;
    result->trampoline = mmap(NULL, trampoline_size, PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // populate trampoline
    memcpy(result->trampoline, source, hook_size);
    memcpy(result->trampoline + hook_size, PUSH_RAX, sizeof(PUSH_RAX)); // preserve %rax
    memcpy(result->trampoline + hook_size + sizeof(PUSH_RAX), ABSOLUTE_JMP, sizeof(ABSOLUTE_JMP));
    // set jump address
    *(uint64_t *)(result->trampoline + hook_size + sizeof(PUSH_RAX) + ABSOLUTE_JMP_ADDR_OFFSET) =
        (uint64_t)source + sizeof(ABSOLUTE_JMP);

    // hook the function
    long page_size = sysconf(_SC_PAGESIZE);
    mprotect(source - ((long)source % page_size), page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(source, ABSOLUTE_JMP, sizeof(ABSOLUTE_JMP));
    // restore %rax saved in the trampoline
    memcpy(source + sizeof(ABSOLUTE_JMP), POP_RAX, sizeof(POP_RAX));
    // fill the gap with NOP
    for (uint8_t *insn = source + sizeof(ABSOLUTE_JMP) + sizeof(POP_RAX);
            insn < source + hook_size; insn++) {
        *insn = NOP_BYTE;
    }

    // set jump address
    *(uint64_t *)(source + ABSOLUTE_JMP_ADDR_OFFSET) = (uint64_t)destination;

    // restore default protection
    mprotect(source  - ((long)source % page_size), page_size, PROT_READ | PROT_EXEC);

    LOGS("hooked successfully");

    return 0;
}
