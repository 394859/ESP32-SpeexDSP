#ifndef OS_SUPPORT_CUSTOM_H
#define OS_SUPPORT_CUSTOM_H

#include "config.h"  // Include config.h for HAVE_CONFIG_H, USE_PSRAM, etc.
#include <stdlib.h>  // For calloc, realloc, free (fallback)
#include <string.h>  // For memcpy, memmove, memset
#include <stdio.h>   // For fprintf

// Include ESP32-specific headers if using PSRAM or FreeRTOS
#ifdef USE_PSRAM
#include <esp_heap_caps.h>  // For heap_caps_malloc, heap_caps_realloc, etc.
#endif
#ifdef USE_FREERTOS_HEAP
#include <freertos/FreeRTOS.h>  // For pvPortMalloc, vPortFree
#include <freertos/portable.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Override speex_alloc (clears memory)
#define OVERRIDE_SPEEX_ALLOC
static inline void* speex_alloc(int size) {
   if (size <= 0) return NULL;
#ifdef USE_PSRAM
   // Allocate in PSRAM with clearing (like calloc)
   void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
   if (ptr) memset(ptr, 0, size);
   return ptr;
#elif defined(USE_FREERTOS_HEAP)
   // Use FreeRTOS heap with manual clearing
   void* ptr = pvPortMalloc(size);
   if (ptr) memset(ptr, 0, size);
   return ptr;
#else
   // Fallback to standard calloc (clears memory)
   return calloc(size, 1);
#endif
}

// Override speex_alloc_scratch (no clearing needed)
#define OVERRIDE_SPEEX_ALLOC_SCRATCH
static inline void* speex_alloc_scratch(int size) {
   if (size <= 0) return NULL;
#ifdef USE_PSRAM
   // Allocate in PSRAM without clearing
   return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#elif defined(USE_FREERTOS_HEAP)
   // Use FreeRTOS heap without clearing
   return pvPortMalloc(size);
#else
   // Fallback to standard malloc (no clearing)
   return malloc(size);
#endif
}

// Override speex_free (moved up to be defined before speex_realloc)
#define OVERRIDE_SPEEX_FREE
static inline void speex_free(void* ptr) {
   if (!ptr) return;
#ifdef USE_PSRAM
   heap_caps_free(ptr);
#elif defined(USE_FREERTOS_HEAP)
   vPortFree(ptr);
#else
   free(ptr);
#endif
}

// Override speex_realloc (now uses speex_free after its definition)
#define OVERRIDE_SPEEX_REALLOC
static inline void* speex_realloc(void* ptr, int size) {
   if (size <= 0) {
      speex_free(ptr);
      return NULL;
   }
#ifdef USE_PSRAM
   // Realloc in PSRAM (note: may fail if moving between heaps)
   return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#elif defined(USE_FREERTOS_HEAP)
   // FreeRTOS doesn’t have realloc, so manual realloc
   void* new_ptr = pvPortMalloc(size);
   if (new_ptr && ptr) {
      memcpy(new_ptr, ptr, size);  // Copy old data (assumes new size >= old)
      speex_free(ptr);
   }
   return new_ptr;
#else
   // Fallback to standard realloc
   return realloc(ptr, size);
#endif
}

// Override speex_free_scratch (same as speex_free)
#define OVERRIDE_SPEEX_FREE_SCRATCH
static inline void speex_free_scratch(void* ptr) {
   if (!ptr) return;
#ifdef USE_PSRAM
   heap_caps_free(ptr);
#elif defined(USE_FREERTOS_HEAP)
   vPortFree(ptr);
#else
   free(ptr);
#endif
}

// Optional: Override fatal error handling with ESP32 logging
#define OVERRIDE_SPEEX_FATAL
static inline void _speex_fatal(const char *str, const char *file, int line) {
   fprintf(stderr, "Fatal error in %s, line %d: %s\n", file, line, str);
   exit(1);  // Could use esp_restart() if <esp_system.h> is included
}

// Optional: Customize warnings
#define OVERRIDE_SPEEX_WARNING
static inline void speex_warning(const char *str) {
#ifndef DISABLE_WARNINGS
   fprintf(stderr, "warning: %s\n", str);
#endif
}

// Optional: Customize integer warnings
#define OVERRIDE_SPEEX_WARNING_INT
static inline void speex_warning_int(const char *str, int val) {
#ifndef DISABLE_WARNINGS
   fprintf(stderr, "warning: %s %d\n", str, val);
#endif
}

// Optional: Customize notifications
#define OVERRIDE_SPEEX_NOTIFY
static inline void speex_notify(const char *str) {
#ifndef DISABLE_NOTIFICATIONS
   fprintf(stderr, "notification: %s\n", str);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* OS_SUPPORT_CUSTOM_H */