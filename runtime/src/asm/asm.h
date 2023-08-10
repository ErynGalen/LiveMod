#include <cstdint>
#include <stddef.h>
#include <stdint.h>

int instruction_length(const unsigned char *func);

#if defined(__x86_64__)
// mov rax, 0xyyyyyyyyyyyyyyyy
// jmp rax
static constexpr uint8_t ABSOLUTE_JMP[] = {0x48, 0xB8, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0};
static constexpr size_t ABSOLUTE_JMP_ADDR_OFFSET = 2;

static constexpr uint8_t PUSH_RAX[] = {0x50};

static constexpr uint8_t POP_RAX[] = {0x58};
static constexpr uint8_t NOP_BYTE = 0x90;
#else
#error Only x86-64 is supported at the moment
#endif
