#ifndef PLATFORM_COMPAT_H
#define PLATFORM_COMPAT_H

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>

static inline void Sleep(unsigned int ms) {
    usleep(ms * 1000);
}
#endif

#endif
