#ifndef H_DLHOOK_H
#define H_DLHOOK_H

// replace dynamically loaded symbols
// overriding functions are preloaded throught the use of LD_PRELOAD
// statically linked symbols cannot be replaced by this method

#include <dlfcn.h>

// mark functions overriding dynamically linked symbols with `DYNHOOK`
#ifdef __cplusplus
#define DYNHOOK extern "C"
#else
#define DYNHOOK
#endif

// set `orig` to the adddress of dynamic symbol `name`
// 0 on success
bool link_original(void **orig, const char *name);

#endif
