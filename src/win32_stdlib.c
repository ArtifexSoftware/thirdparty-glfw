//========================================================================
// GLFW 3.3 Win32 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2016 Camilla Berglund <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// NOTE: THESE FUNCTIONS ARE NOT STANDARD-COMPLIANT; THIS IS NOT A LIBC
//       They do only what GLFW needs them to do
//       Use them only if you understand the difference

void _glfw_memset(void* ptr, int c, size_t size)
{
    char* p = ptr;
    while (size--)
        *p++ = (char) c;
}

void* _glfw_memcpy(void* d, const void* s, size_t size)
{
    char* dp = d;
    const char* sp = s;
    while (size--)
        *dp++ = *sp++;
    return d;
}

void* _glfw_calloc(size_t count, size_t size)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, count * size);
}

void* _glfw_realloc(void* ptr, size_t size)
{
    if (ptr)
        return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
    else
        return HeapAlloc(GetProcessHeap(), 0, size);
}

void _glfw_free(void* ptr)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}

char* _glfw_strdup(const char* str)
{
    char* dup = _glfw_calloc(_glfw_strlen(str) + 1, 1);
    _glfw_strcpy(dup, str);
    return dup;
}

size_t _glfw_strlen(const char* str)
{
    return lstrlenA(str);
}

int _glfw_strcmp(const char* a, const char* b)
{
    return lstrcmpA(a, b);
}

int _glfw_strncmp(const char* a, const char* b, size_t l)
{
    while (l--)
    {
        if (*a != *b)
            return *a - *b;
        if (!*a)
            break;
        a++;
        b++;
    }

    return 0;
}

char* _glfw_strstr(const char* a, const char* b)
{
    do
    {
        size_t i = 0;
        while (a[i] && a[i] == b[i])
            i++;
        if (!b[i])
            return (char*) a;
    }
    while (*a++);

    return NULL;
}

char* _glfw_strcpy(char* d, const char* s)
{
    return lstrcpyA(d, s);
}

int _glfw_vsnprintf(char* str, size_t size, const char* fmt, va_list list)
{
    // TODO: Something that includes bounds checking
    return wvsprintfA(str, fmt, list);
}

void _glfw_qsort(void* ptr, size_t count, size_t size, int (*comp)(const void*, const void*))
{
    // TODO: The code
}

double _glfw_pow(double x, double y)
{
    // TODO: Find suitably licensed version
    return 0.0;
}

int _glfw_abs(int x)
{
    if (x < 0)
        return -x;
    return x;
}

