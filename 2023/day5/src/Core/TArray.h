#ifndef TARRAY_H

// ========================================================================== //
// Dynamic array type. Use as basically a drop-in replacement for C arrays.
// Allows implicit conversion to pointer type. Uses asserts for bounds checks,
// which will usually happen in debug but not release builds.
// You can initialize basically any way you want:
// TArray<int> arr = {};
// TArray<int> arr = TArray<int>();
// TArray<int> arr = TArray<int>(16);
//
// @Todo(Frog): Sorting, maybe? QSort style API? That or require comparison
// operators be defined.
// @Todo(Frog): Support a custom allocator, so we aren't just slapping stuff
// onto the heap all the time.
// @Todo(Frog): Disable Move/Copy constructors.
// ========================================================================== //

typedef int tarray_int;

// If you define TARRAY_MALLOC, TARRAY_REALLOC, TARRAY_FREE, and
// TARRAY_ZEROMEMORY, the standard library versions won't be included.
#if !defined TARRAY_MALLOC || !defined TARRAY_REALLOC || !defined TARRAY_FREE || !defined TARRAY_ZEROMEMORY
#include <cstdlib>
#endif

// If you define your own assert, the standard library version isn't used.
#ifndef TARRAY_ASSERT
#include <cassert>
#define TARRAY_ASSERT assert
#endif

// If no custom malloc is defined, use the stdlib version.
#ifndef TARRAY_MALLOC
#define TARRAY_MALLOC(size) malloc(size)
#endif

// If no custom free is defined, use the stdlib version.
#ifndef TARRAY_REALLOC
#define TARRAY_REALLOC(old_ptr, size) realloc(old_ptr, size)
#endif

// If no custom zero is defined, use the stdlib version.
#ifndef TARRAY_ZEROMEMORY
#define TARRAY_ZEROMEMORY(ptr, size) memset(ptr, 0, size)
#endif

// If no custom free is defined, use the stdlib version.
#ifndef TARRAY_FREE
#define TARRAY_FREE(ptr) free(ptr)
#endif

// By default, the first allocation will make space for TARRAY_INITIAL_CAPACITY
// elements. You can define this value differently if you like.
#ifndef TARRAY_INITIAL_CAPACITY
#define TARRAY_INITIAL_CAPACITY 4
#endif

template <typename T>
struct TArray
{
    // Constructors.
    TArray() = default; // Default initialization is allowed.
    TArray(tarray_int length); // Constructor from length.
    TArray(const TArray<T>& other); // Copy constructor.

    // Operator overloads.
    inline operator T*() const {return ptr;} // Implicit pointer conversion.
    inline T& operator[](tarray_int i); // Array access.
    inline const T& operator[](tarray_int i) const; // Const array access.
    inline TArray<T>& operator=(const TArray<T>& other); // Copy assignment.

    // Gets and sets length/capacity.
    inline tarray_int Length() const {return length;}
    inline tarray_int Capacity() const {return capacity;}
    inline size_t ByteSize() const {return length * sizeof(T);}
    inline void SetLength(tarray_int length);
    inline void SetCapacity(tarray_int length); // Can grow or shrink.

    // Inserts new elements and returns the new size.
    inline tarray_int Append(const T& element);
    inline tarray_int Append(const TArray<T>& other);
    inline tarray_int Insert(const T& element, tarray_int i);

    // Removes elements.
    inline T Remove(tarray_int i); // Shifts subsequent elements to maintain ordering.
    inline T RemoveAndSwap(tarray_int i); // Swaps with the back array element.

    // Frees the array memory.
    inline void Free();
    ~TArray<T>() {Free();}

    // Checks if an item (or all items) are present. Requires == be defined.
    inline bool Contains(const T& element) const;
    inline bool Contains(const TArray<T>& other) const; // Checks if all are present.
    inline tarray_int IndexOf(const T& element) const; // Earliest index, or -1.

    T* begin() const { return ptr; }
    T* end() const { return ptr + length; }

    private:
    T* ptr; // Heap allocated base pointer.
    tarray_int length; // Number of currently stored elements.
    tarray_int capacity; // Total number of elements that could be stored.
};
#define TARRAY_H
#endif

// ========================================================================== //
// End of header. Implementation below.
// ========================================================================== //

#ifdef TARRAY_IMPLEMENTATION
template <typename T>
TArray<T>::TArray(const TArray<T>& other)
{
    ptr = nullptr;
    length = 0;
    capacity = 0;
    *this = other;
}

template <typename T>
TArray<T>::TArray(tarray_int length) : length(length)
{
    TARRAY_ASSERT(length >= 0);
    if (length > 0)
    {
        capacity = (length > TARRAY_INITIAL_CAPACITY) ? length : TARRAY_INITIAL_CAPACITY;
        size_t size = sizeof(T) * capacity;
        ptr = (T*)TARRAY_MALLOC(size);
        TARRAY_ZEROMEMORY(ptr, size);
    }
    else
    {
        capacity = 0;
        ptr = nullptr;
    }
}

template <typename T>
T& TArray<T>::operator[](tarray_int i)
{
    TARRAY_ASSERT(i >= 0 && i < length);
    return ptr[i];
}

template <typename T>
const T& TArray<T>::operator[](tarray_int i) const
{
    TARRAY_ASSERT(i >= 0 && i < length);
    return ptr[i];
}

template <typename T>
TArray<T>& TArray<T>::operator=(const TArray<T>& other)
{
    if (this != &other)
    {
        Free();
        SetCapacity(other.capacity);
        SetLength(other.length);
        for (tarray_int i = 0; i < length; ++i) ptr[i] = other[i];
    }
    return *this;
}

template <typename T>
void TArray<T>::SetLength(tarray_int length)
{
    this->length = length;
    if (length > capacity) SetCapacity(length);
}

template <typename T>
void TArray<T>::SetCapacity(tarray_int capacity)
{
    if (this->capacity == capacity) return;
    tarray_int old_capacity = this->capacity;
    if (length > capacity) length = capacity;
    size_t size = capacity * sizeof(T);
    this->capacity = capacity;
    ptr = (ptr) ? (T*)TARRAY_REALLOC(ptr, size) : (T*)TARRAY_MALLOC(size);
    if (capacity > old_capacity)
    {
        size_t new_size = (capacity - old_capacity) * sizeof(T);
        TARRAY_ZEROMEMORY(ptr + old_capacity, new_size);
    }
}

template <typename T>
tarray_int TArray<T>::Append(const T& element)
{
    if (capacity == 0) SetCapacity(TARRAY_INITIAL_CAPACITY);
    else if (length == capacity) SetCapacity(capacity * 2);
    ptr[length] = element;
    return ++length;
}

template <typename T>
tarray_int TArray<T>::Append(const TArray<T>& other)
{
    for (tarray_int i = 0; i < other.length; ++i) Append(other[i]);
    return length;
}

template <typename T>
tarray_int TArray<T>::Insert(const T& element, tarray_int i)
{
    TARRAY_ASSERT(i >= 0 && i <= length);
    if (capacity == 0) SetCapacity(TARRAY_INITIAL_CAPACITY);
    else if (length > capacity) SetCapacity(capacity * 2);
    for (tarray_int j = length; j > i; --j) ptr[j] = ptr[j - 1];
    ptr[i] = element;
    return ++length;
}

template <typename T>
T TArray<T>::Remove(tarray_int i)
{
    TARRAY_ASSERT(i >= 0 && i < length);
    length--;
    T result = ptr[i];
    for (tarray_int j = i; j < length; ++j) ptr[j] = ptr[j + 1];
    return result;
}

template <typename T>
T TArray<T>::RemoveAndSwap(tarray_int i)
{
    TARRAY_ASSERT(i >= 0 && i < length);
    T result = ptr[i];
    ptr[i] = ptr[--length];
    return result;
}

template <typename T>
void TArray<T>::Free()
{
    if (ptr != nullptr) TARRAY_FREE(ptr);
    length = 0;
    capacity = 0;
    ptr = nullptr;
}

template <typename T>
bool TArray<T>::Contains(const T& element) const
{
    for (tarray_int i = 0; i < length; ++i) if (ptr[i] == element) return true;
    return false;
}

template <typename T>
bool TArray<T>::Contains(const TArray<T>& other) const
{
    if (length < other.length) return false;
    for (tarray_int i = 0; i < other.length; ++i) if (!Contains(other[i])) return false;
    return true;
}

template <typename T>
tarray_int TArray<T>::IndexOf(const T& element) const
{
    for (tarray_int i = 0; i < length; ++i) if (ptr[i] == element) return i;
    return -1;
}
#endif