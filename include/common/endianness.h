// This file aims to provide a cross-platform method to find the endianness of the target system.
// It has not been fully tested and may contain errors. If it is unable to determine the endianness,
// the ENDIANNESS macro can be set manually to BIG_E or LITTLE_E. Most systems will be little endian.

#ifndef COMMON__ENDIANNESS_H
#define COMMON__ENDIANNESS_H

#define BIG_E 0
#define LITTLE_E 1

#if defined(__linux__) || defined(__CYGWIN__)
    #include <endian.h>
#elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
    #include <winsock2.h>
    #include <sys/param.h>
#elif defined(__APPLE__)
    #include <machine/endian.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
    #include <sys/endian.h>
#endif

#if defined(ENDIANNESS) && ENDIANNESS != BIG_E && ENDIANNESS != LITTLE_E
    #error "Invalid ENDIANNESS macro!"
#endif

#if !defined(ENDIANNESS)
    #if (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) || \
        (defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN) || \
        defined(__BIG_ENDIAN__) || \
        defined(__ARMEB__) || \
        defined(__THUMBEB__) || \
        defined(__AARCH64EB__) || \
        defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
        // Big endian target
        #define ENDIANNESS BIG_E
    #elif (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
        (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) || \
        defined(__LITTLE_ENDIAN__) || \
        defined(__ARMEL__) || \
        defined(__THUMBEL__) || \
        defined(__AARCH64EL__) || \
        defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
        // Little endian target
        #define ENDIANNESS LITTLE_E
    #else
        #error "Unable to determine endianness! Set ENDIANNESS macro manually."
    #endif
#endif

#endif