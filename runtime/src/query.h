#ifndef H_QUERY_H
#define H_QUERY_H

#ifdef __linux__
#include <dlfcn.h>
#define QUERY_NEXT RTLD_NEXT
#else
#define QUERY_NEXT (-1)
#endif

// Get the addres of a dynamic or static symbol
void *querySymbol(void *handle, const char *name);

#endif
