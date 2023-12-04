#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Integer typedefs.
#define U8_MAX UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX
#define S8_MAX INT8_MAX
#define S16_MAX INT16_MAX
#define S32_MAX INT32_MAX
#define S64_MAX INT64_MAX
#define OUTPUT_BUFFER_SIZE 65536

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

// Technically KiB, MiB, and GiB, but who's counting?
#define KB(size) ((uint64_t) 1024 * (size))
#define MB(size) ((uint64_t) 1024 * KB(size))
#define GB(size) ((uint64_t) 1024 * MB(size))

#define ARRAYCOUNT(x) (sizeof(x) / sizeof(x[0]))

// @Todo(Frog): Do these without punting to cstdlib.
#define StrLen(string) strlen((string))
#define StrPrintF(buffer, size, format, ...) snprintf((buffer), (size), (format), __VA_ARGS__)

// Static buffer for printf calls.
static char OUTPUT_BUFFER[OUTPUT_BUFFER_SIZE];

// Print a string to stdout.
#define PrintLog(string) Platform::PrintMessage((string))

// @Todo(Frog): This should call Print multiple times if the input is longer than our output buffer.
// Formatted print to stdout, limited to OUTPUT_BUFFER_SIZE in length.
#define PrintF(format, ...)                                        \
{                                                                  \
StrPrintF(OUTPUT_BUFFER, OUTPUT_BUFFER_SIZE, format, __VA_ARGS__); \
PrintLog(OUTPUT_BUFFER);                                              \
}

// These do the same as Print and PrintF, they just output to stderr instead.
#define ErrPrint(string) Platform::PrintError((string))
#define ErrPrintF(format, ...)                                     \
{                                                                  \
StrPrintF(OUTPUT_BUFFER, OUTPUT_BUFFER_SIZE, format, __VA_ARGS__); \
ErrPrint(OUTPUT_BUFFER);                                           \
}

// Assert macros.
#ifndef NDEBUG
#define Assert(x)                                                                                                      \
{                                                                                                                      \
if (!(x))                                                                                                              \
{                                                                                                                      \
StrPrintF(OUTPUT_BUFFER, OUTPUT_BUFFER_SIZE, "Assertion Failed (%s, line %d):\nAssert(%s)\n", __FILE__, __LINE__, #x); \
ErrPrint(OUTPUT_BUFFER);                                                                                               \
if (Platform::ShowAssertDialog(OUTPUT_BUFFER)) __debugbreak();                                                         \
}                                                                                                                      \
}
#else
#define Assert(x)
#endif // NDEBUG

#ifndef NDEBUG
#define AssertCustom(x, message)                                                                                                    \
{                                                                                                                                   \
if (!(x))                                                                                                                           \
{                                                                                                                                   \
StrPrintF(OUTPUT_BUFFER, OUTPUT_BUFFER_SIZE, "Assertion Failed (%s, line %d):\n%s\nAssert(%s)\n", __FILE__, __LINE__, #x, message); \
ErrPrint(OUTPUT_BUFFER);                                                                                                            \
if (Platform::ShowAssertDialog(OUTPUT_BUFFER)) __debugbreak();                                                                      \
}                                                                                                                                   \
}
#else
#define AssertCustom(x, message)
#endif // NDEBUG

#include "MString.h"
#include "TArray.h"


#include "Span.h"