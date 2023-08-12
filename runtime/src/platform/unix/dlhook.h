#ifndef H_DLHOOK_H
#define H_DLHOOK_H

// replace dynamically loaded symbols
// overriding functions are preloaded throught the use of LD_PRELOAD
// statically linked symbols cannot be replaced by this method

#include <dlfcn.h>

// mark functions overriding dynamically linked symbols with `DYNHOOK`
#ifdef __cplusplus
#define DL_HOOK extern "C"
#else
#define DL_HOOK
#endif

// set `orig` to the adddress of dynamic symbol `name`
// 0 on success
bool linkOriginal(void **orig, const char *name);

/// link the original `func` that has the specified signature
#define DL_ORIG(ret_type, func, ...)                                                                                   \
    static ret_type (*orig_##func)(__VA_ARGS__) = NULL;                                                                \
    linkOriginal((void **)&orig_##func, #func);

#endif
