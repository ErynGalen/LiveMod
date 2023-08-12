#ifndef H_ASM_ASM_H
#define H_ASM_ASM_H

#include "GlobalLLVM.h"
#include <cstdint>
#include <stddef.h>

struct InstructionInfo {
    size_t m_length = 0;
    bool m_usesProgramCounter = false;
    /// Info of the instruction at `addr`,  limited by the specified max length
    bool atAddr(uint8_t *addr, size_t maxLength);
};

#if defined(__x86_64__)
// mov rax, 0xyyyyyyyyyyyyyyyy
// jmp rax
static constexpr uint8_t ABSOLUTE_JMP[] = {0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0};
static constexpr size_t ABSOLUTE_JMP_ADDR_OFFSET = 2;

static constexpr uint8_t PUSH_RAX[] = {0x50};

static constexpr uint8_t POP_RAX[] = {0x58};
static constexpr uint8_t NOP_BYTE  = 0x90;
#else
#error Only x86-64 is supported at the moment
#endif

#endif
