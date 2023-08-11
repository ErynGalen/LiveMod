#ifndef H_PROCESS_H
#define H_PROCESS_H

#include <string>

// the last element of args[] must be NULL
std::string execAndGet(char *command, char *const args[]);

#endif
