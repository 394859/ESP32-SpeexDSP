#ifndef OS_SUPPORT_CUSTOM_H
#define OS_SUPPORT_CUSTOM_H

#include "config.h"  // Include config.h for HAVE_CONFIG_H, etc.
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OVERRIDE_SPEEX_ALLOC
static inline void* speex_alloc(int size) {
  // Must clear memory (like calloc), as required by SpeexDSP
  return (size > 0) ? static_cast<void*>(calloc(size, 1)) : NULL;
}

#define OVERRIDE_SPEEX_REALLOC
static inline void* speex_realloc(void* ptr, int size) {
  return static_cast<void*>(realloc(ptr, size));
}

#define OVERRIDE_SPEEX_FREE
static inline void speex_free(void* ptr) {
  if (ptr) free(ptr);
}

// Optional: Override scratch functions if needed (not currently required)
#define OVERRIDE_SPEEX_ALLOC_SCRATCH
static inline void* speex_alloc_scratch(int size) {
  // Scratch doesn’t need to be cleared, but we’ll match speex_alloc for simplicity
  return (size > 0) ? static_cast<void*>(calloc(size, 1)) : NULL;
}

#define OVERRIDE_SPEEX_FREE_SCRATCH
static inline void speex_free_scratch(void* ptr) {
  if (ptr) free(ptr);
}

#ifdef __cplusplus
}
#endif

#endif /* OS_SUPPORT_CUSTOM_H */STOM_H */