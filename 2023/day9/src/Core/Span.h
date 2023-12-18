#pragma once

#include "EngineCore.h"
// @Todo(Frog): Auto-cast to underlying pointer type, maybe?

/**
 * Basic wrapper around a pointer and count. You can use these to pass around contiguous groups of things,
 * like an array of objects, without needing to pass the pointer and count separately. A span does not
 * own referenced memory and will not allocate or free it.
 *
 * You can construct a span empty, from a pointer and count, or from a static array of elements.
 * The latter uses a template parameter to determine the count, so don't go crazy with it.
 *
 * You can also index a span the same way you would an array, and you can create a sub-span of the
 * first or last N elements, or a group of elements in the middle.
 *
 * A basic begin() and end() implementation are provided so that range-based for loops work in the same way
 * as for static arrays.
 *
 * Note that NO bounds checking or null checking is performed, to keep this wrapper as thin as possible.
 * Use at your own risk.
 */
template <typename T> struct Span
{
    T* ptr;
    s64 count;

    constexpr Span() = default;
    constexpr Span(T* first, s64 count) : ptr(first), count(count) {}
    template<s64 N> constexpr Span(T(&arr)[N]) : ptr(arr), count(N) {} // Initialize from a static array.

    constexpr Span<T> First(s64 n)              { return {ptr, n}; }             // First N elements.
    constexpr Span<T> Last(s64 n)               { return {&ptr[count - n], n}; } // Last N elements.
    constexpr Span<T> SubSpan(s64 first, s64 n) { return {ptr + offset, n}; }    // N elements starting at first.
    constexpr s64 ByteSize() {return count * sizeof(T);}

    constexpr T& operator[](s64 i) const { return ptr[i]; };

    constexpr T* begin() const { return ptr; }
    constexpr T* end() const { return ptr + count; }
};