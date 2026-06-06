
/**    \file   test_stubs.cpp

Minimal implementations of MotionCodeLibrary logging symbols needed by the
test executable.

Why this file exists:
    Logger.h declares LOGE and LOGI as plain extern functions (not macros).
    Their definitions live in MotionCodeLibrary, which is a PRIVATE dependency
    of the manager_PostProcessing DLL.  PRIVATE linkage means the symbols are
    compiled into the DLL but not re-exported through its import library.

    Inline methods in shaderproperty_value.h (GetTexture, SetTexture, etc.)
    call LOGE directly, so those call sites are compiled into every translation
    unit that includes the header — including the test executable.  The linker
    therefore needs a definition of LOGE that is reachable from the test binary.

    This file provides that definition.  It intentionally routes all messages
    to stderr so failures in helper code are still visible in test output.
*/

#include "Logger.h"     // exact extern declarations — ensures signatures match

#include <cstdarg>
#include <cstdio>

void LOGE(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

void LOGI(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fputc('\n', stdout);
}
