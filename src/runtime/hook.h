#ifndef H_RUNTIME_HOOK_H
#define H_RUNTIME_HOOK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <capstone/capstone.h>

typedef struct {
    csh handle;
    cs_insn *insn;
} disasm_t;

int disasm_create(disasm_t *disasm);
void disasm_destroy(disasm_t *disasm);

typedef struct {
    uint8_t *source;
    uint8_t *destination;
    uint8_t *trampoline;

    size_t hook_size;
    size_t trampoline_size;
} hook_t;

int hook(disasm_t *disasm, void *source, size_t max_size, void *dest, hook_t *result);

/// retrieve `orig_func` from the specified hook, whose arguments have the types specified in `...`
#define SET_ORIGINAL(hook, ret_type, func, ...) \
    ret_type (*orig_##func)(__VA_ARGS__) = (ret_type(*)(__VA_ARGS__))hook.trampoline

#ifdef __cplusplus
}
#endif

#endif
