#ifndef H_LOG_H
#define H_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define LOG(msg, ...) printf("[" MODULE "] " msg "\n", __VA_ARGS__)
#define LOGS(msg) printf("[" MODULE "] " msg "\n")

#ifdef __cplusplus
}
#endif
#endif
