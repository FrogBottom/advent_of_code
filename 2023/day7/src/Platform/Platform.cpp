#include "Platform/Platform.h"

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 2048
#endif

namespace Win32 {
s32 ConvertPath(IString path, Span<WCHAR>* out_buffer)
{
    Assert(path.Ptr()); // A null path is not valid (although an empty one is).
    Assert(out_buffer);

	// Figure out the required length.
	s32 wide_length = 0;
	if (path.Length() > 0) wide_length = MultiByteToWideChar(CP_UTF8, 0, path, (int)path.Length(), 0, 0);

	// Choose whether we need to allocate a bigger buffer, and set up the result accordingly.
    Assert(out_buffer->count > wide_length);
	// Convert and fix up the path (null terminate and replace path separators).
	if (path.Length()) MultiByteToWideChar(CP_UTF8, 0, path, (int)path.Length(), (LPWSTR)out_buffer->ptr, wide_length);
	out_buffer->ptr[wide_length] = L'\0';
	for (s32 i = 0; i < out_buffer->count; ++ i) if (out_buffer->ptr[i] == L'/') out_buffer->ptr[i] = L'\\';
	return wide_length;
}

// Sets up a standard stream (such as stdout or stderr).
static void* GetStandardStream(u32 stream_type)
{
    // If we don't have our own stream and can't find a parent console, allocate a new console.
    void* result = GetStdHandle(stream_type);
    if (!result || result == INVALID_HANDLE_VALUE)
    {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) AllocConsole();
        result = GetStdHandle(stream_type);
    }

    // Set console output mode to UTF-8.
    SetConsoleOutputCP(CP_UTF8);

    // Try to enable VT code parsing.
    u32 mode = 0;
    GetConsoleMode(result, (LPDWORD)&mode);
    SetConsoleMode(result, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    return result;
}

// Prints a message to a platform stream. Always assumes we are writing UTF-8.
// If we are attached to a debugger, calls OutputDebugString instead.
// Note that this is true even if the stream is redirected to a file!
static void PrintToStream(const char* message, void* stream)
{
    u32 bytes_written = 0;
    if (IsDebuggerPresent()) OutputDebugStringA(message);
    else WriteFile(stream, message, (DWORD)StrLen(message), (LPDWORD)&bytes_written, 0);
}
} // namespace Win32

struct Win32StandardStream
{
    HANDLE handle; // Stream handle (STD_OUTPUT_HANDLE or STD_ERROR_HANDLE).
    bool is_redirected; // True if redirected to file.
    bool is_wide; // True if appending to a UTF-16 file.
    bool is_little_endian; // True if file is UTF-16 little endian.
};

void Platform::TimerStart(Timer* timer)
{
        LARGE_INTEGER frequency, start;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start);
        timer->frequency = frequency.QuadPart;
        timer->start_count = start.QuadPart;
}

u64 Platform::TimerMeasureCounts(Timer* timer)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (u64)now.QuadPart - timer->start_count;
}

u64 Platform::TimerCountsToMicroseconds(Timer* timer, u64 counts)
{
    return (counts * 1000000) / timer->frequency;
}

s64 Platform::GetFileSize(IString path)
{
	Assert(path.Ptr() && path.Length()); // No null or empty paths allowed.

	s64 result = -1;

	WCHAR stack_buffer[MAX_PATH];
    Span<WCHAR> wide_path = {stack_buffer, MAX_PATH};
	s32 wide_length = Win32::ConvertPath(path, &wide_path);
	HANDLE handle = CreateFileW(wide_path.ptr, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);

	if (handle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(handle, &file_size)) result = file_size.QuadPart;
		CloseHandle(handle);
	}
	return result;
}

Span<u8> Platform::ReadFileToBuffer(IString path)
{
	Assert(path.Ptr() && path.Length()); // No null or empty paths allowed.
	WCHAR stack_buffer[MAX_PATH];
    Span<WCHAR> wide_path = {stack_buffer, MAX_PATH};
	s32 wide_length = Win32::ConvertPath(path, &wide_path);
	HANDLE handle = CreateFileW(wide_path.ptr, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);

	Span<u8> result = {};
    if (handle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(handle, &file_size))
		{
			result = {(u8*)malloc(file_size.QuadPart), file_size.QuadPart};
			DWORD dummy;
			bool success = ReadFile(handle, result.ptr, (DWORD)result.count, &dummy, 0);
			CloseHandle(handle);
			if (!success)
			{
				free(result.ptr);
				result = {};
			}
		}
	}
	return result;
}

bool Platform::ReadFileToBuffer(IString path, Span<u8> buffer)
{
	Assert(buffer.ptr && buffer.count && path.Ptr());
	WCHAR stack_buffer[MAX_PATH];
    Span<WCHAR> wide_path = {stack_buffer, MAX_PATH};
	s32 wide_length = Win32::ConvertPath(path, &wide_path);

	// s32 wide_length = MultiByteToWideChar(CP_UTF8, 0, path, path.Length(), 0, 0) + 1; // Add one for the null terminator.
	// WCHAR* wide_buffer = (WCHAR*)malloc(wide_length * sizeof(WCHAR));
	// MultiByteToWideChar(CP_UTF8, 0, path.Ptr(), path.Length(), wide_buffer, wide_length);
	HANDLE handle = CreateFileW(wide_path.ptr, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);

	bool result = false;
    if (handle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(handle, &file_size))
		{
			Assert(buffer.count >= file_size.QuadPart);
			DWORD dummy;
			result = ReadFile(handle, buffer.ptr, (DWORD)buffer.count, &dummy, 0);
			CloseHandle(handle);
		}
	}
	return result;
}
bool Platform::IsConsoleVTEnabled()
{
    void* std_out = Win32::GetStandardStream(STD_OUTPUT_HANDLE);

    u32 mode = 0;
    GetConsoleMode(std_out, (LPDWORD)&mode);
    return (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void Platform::PrintMessage(const char* message)
{
    static void* stream = Win32::GetStandardStream(STD_OUTPUT_HANDLE);
    Win32::PrintToStream(message, stream);
}

void Platform::PrintError(const char* message)
{
    static void* stream = Win32::GetStandardStream(STD_ERROR_HANDLE);
    Win32::PrintToStream(message, stream);
}

bool Platform::ShowAssertDialog(const char* message)
{
    // @Todo(Frog): This malloc's the assert string to convert from UTF-8 to UTF-16. Not ideal.
    s32 buffer_size = MultiByteToWideChar(CP_UTF8, 0, message, -1, 0, 0);
    char16_t* wide_string = (char16_t*)malloc(sizeof(char16_t) * buffer_size); // @malloc
    MultiByteToWideChar(CP_UTF8, 0, message, -1, (LPWSTR)wide_string, buffer_size);
    int result = MessageBoxW(0, (LPCWSTR)wide_string, L"Assertion Failed!", MB_YESNO | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
    free(wide_string); // @malloc
    return (result == IDYES);
}