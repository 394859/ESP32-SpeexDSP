#ifndef OS_SUPPORT_CUSTOM_H
#define OS_SUPPORT_CUSTOM_H

#include "config.h"  // For HAVE_CONFIG_H, USE_PSRAM, etc.
#include <string.h>  // For memcpy, memmove, memset
#include <stdio.h>   // For fprintf (fallback)
#include <stdlib.h>  // For malloc, calloc, realloc, free (fallback)

// ESP32-specific headers
#ifdef ESP_PLATFORM
#include <esp_heap_caps.h>     // For heap_caps_malloc, heap_caps_free, etc.
#include <esp_log.h>           // For ESP_LOG macros
#include <esp_system.h>        // For esp_restart
#include <freertos/FreeRTOS.h> // For pvPortMalloc, vPortFree
#include <freertos/portable.h> // For FreeRTOS portability
#define TAG "Speex"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Core memory allocation functions
#define OVERRIDE_SPEEX_ALLOC
static inline void* speex_alloc(int size) {
   if (size <= 0) return NULL;
#ifdef USE_PSRAM
   void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
   if (ptr) memset(ptr, 0, size);
   return ptr;
#elif defined(USE_FREERTOS_HEAP)
   void* ptr = pvPortMalloc(size);
   if (ptr) memset(ptr, 0, size);
   return ptr;
#else
   return calloc(size, 1);  // Fallback: calloc clears memory
#endif
}

#define OVERRIDE_SPEEX_ALLOC_SCRATCH
static inline void* speex_alloc_scratch(int size) {
   if (size <= 0) return NULL;
#ifdef USE_PSRAM
   return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#elif defined(USE_FREERTOS_HEAP)
   return pvPortMalloc(size);
#else
   return malloc(size);  // Scratch doesn’t need clearing
#endif
}

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

#define OVERRIDE_SPEEX_REALLOC
static inline void* speex_realloc(void* ptr, int size) {
   if (size <= 0) {
      speex_free(ptr);
      return NULL;
   }
#ifdef USE_PSRAM
   return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#elif defined(USE_FREERTOS_HEAP)
   if (!ptr) return pvPortMalloc(size);
#ifdef ESP_PLATFORM
   size_t old_size = heap_caps_get_allocated_size(ptr);
   if (old_size >= (size_t)size) return ptr;  // Reuse if shrinking or same size
#endif
   void* new_ptr = pvPortMalloc(size);
   if (new_ptr && ptr) {
      memcpy(new_ptr, ptr, size);  // Copy old data (assumes new size >= old)
      vPortFree(ptr);
   }
   return new_ptr;
#else
   return realloc(ptr, size);  // Fallback to standard realloc
#endif
}

// Memory operations (unchanged from original unless needed)
#define OVERRIDE_SPEEX_COPY
#define SPEEX_COPY(dst, src, n) (memcpy((dst), (src), (n)*sizeof(*(dst)) + 0*((dst)-(src))))

#define OVERRIDE_SPEEX_MOVE
#define SPEEX_MOVE(dst, src, n) (memmove((dst), (src), (n)*sizeof(*(dst)) + 0*((dst)-(src))))

#define OVERRIDE_SPEEX_MEMSET
#define SPEEX_MEMSET(dst, c, n) (memset((dst), (c), (n)*sizeof(*(dst))))

// Error handling and logging
#ifdef ESP_PLATFORM
#define OVERRIDE_SPEEX_FATAL
static inline void _speex_fatal(const char *str, const char *file, int line) {
   ESP_LOGE(TAG, "Fatal error in %s, line %d: %s", file, line, str);
   esp_restart();
}

#define OVERRIDE_SPEEX_WARNING
static inline void speex_warning(const char *str) {
#ifndef DISABLE_WARNINGS
   ESP_LOGW(TAG, "%s", str);
#endif
}

#define OVERRIDE_SPEEX_WARNING_INT
static inline void speex_warning_int(const char *str, int val) {
#ifndef DISABLE_WARNINGS
   ESP_LOGW(TAG, "%s %d", str, val);
#endif
}

#define OVERRIDE_SPEEX_NOTIFY
static inline void speex_notify(const char *str) {
#ifndef DISABLE_NOTIFICATIONS
   ESP_LOGI(TAG, "%s", str);
#endif
}
#else
#define OVERRIDE_SPEEX_FATAL
static inline void _speex_fatal(const char *str, const char *file, int line) {
   fprintf(stderr, "Fatal error in %s, line %d: %s\n", file, line, str);
   exit(1);
}

#define OVERRIDE_SPEEX_WARNING
static inline void speex_warning(const char *str) {
#ifndef DISABLE_WARNINGS
   fprintf(stderr, "warning: %s\n", str);
#endif
}

#define OVERRIDE_SPEEX_WARNING_INT
static inline void speex_warning_int(const char *str, int val) {
#ifndef DISABLE_WARNINGS
   fprintf(stderr, "warning: %s %d\n", str, val);
#endif
}

#define OVERRIDE_SPEEX_NOTIFY
static inline void speex_notify(const char *str) {
#ifndef DISABLE_NOTIFICATIONS
   fprintf(stderr, "notification: %s\n", str);
#endif
}
#endif

#define OVERRIDE_SPEEX_PUTC
static inline void _speex_putc(int ch, void *file) {
#ifdef ESP_PLATFORM
   ESP_LOGI(TAG, "%c", (char)ch);  // Log to ESP32 console
#else
   FILE *f = (FILE *)file;
   fprintf(f, "%c", ch);  // Fallback to file output
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* OS_SUPPORT_CUSTOM_H */