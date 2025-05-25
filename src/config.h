#ifndef CONFIG_H
#define CONFIG_H

#define HAVE_CONFIG_H 1        // Enable config.h inclusion
#define OS_SUPPORT_CUSTOM 1    // Enable os_support_custom.h
#define FLOATING_POINT 1       // Use floating-point arithmetic
#define USE_KISS_FFT 1         // Enable Kiss FFT
#define EXPORT                 // Empty EXPORT for no DLL exports

// Optional ESP32-specific options
#define USE_PSRAM 1            // Use PSRAM for allocations if available
//#define USE_FREERTOS_HEAP 1    // Use FreeRTOS heap (pvPortMalloc/vPortFree)
#define ESP_PLATFORM 1

#endif /* CONFIG_H */