#ifndef PLATFORM_H
#define PLATFORM_H

#if ENV_PLATFORM == ESP32C6
    /* ESP32C6 */
    #define ENV_USEFILESYSTEM   1
    #define ENV_USELCD          1
    #define ENV_USECONFIGMODE   1
#elif ENV_PLATFORM == LILYGO
    /* LILYGO ESP32S3 */
    #define ENV_USEFILESYSTEM   0
    #define ENV_USELCD          0
    #define ENV_USECONFIGMODE   0
#else
#error "Missing Platform"

#endif

#endif /* PLATFORM_H */