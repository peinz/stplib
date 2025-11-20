#pragma once

#include <stdio.h>
#include <signal.h>
#include <assert.h>

#define breakpoint() raise(SIGTRAP);

// [Logging]
// Log levels: 0=debug, 1=info, 2=warning, 3=error
enum Log_Level {
  LOG_LEVEl_DEBUG   = 0,
  LOG_LEVEl_INFO    = 1,
  LOG_LEVEl_WARNING = 2,
  LOG_LEVEl_ERROR   = 3,
};

#ifndef LOG_LEVEL
#define LOG_LEVEL 1  // Default to info level
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define log_v(...) ({ printf("[DEBUG] "); printf(__VA_ARGS__); printf("\n"); })
#else
#define log_v(...) ({})
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define log_i(...) ({ printf("[INFO] "); printf(__VA_ARGS__); printf("\n"); })
#else
#define log_i(...) ({})
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define log_w(...) ({ printf("[ALERT] "); printf(__VA_ARGS__); printf("\n"); })
#else
#define log_w(...) ({})
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define log_e(...) ({ printf("[ERROR] %s:%d ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); })
#else
#define log_e(...) ({})
#endif
#define unreachable(...)  ({ printf("[ASSERT] %s:%d ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); assert(!"unreachable"); })

