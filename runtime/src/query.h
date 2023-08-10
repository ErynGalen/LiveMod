#ifndef H_QUERY_H
#define H_QUERY_H

#include <dlfcn.h>

#define QUERY_NEXT RTLD_NEXT

// Get the addres of a dynamic or static symbol
void *query_symbol(void *handle, const char *name);

#endif
