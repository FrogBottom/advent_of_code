#pragma once

#include "Core/EngineCore.h"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#else
#error Sorry, only Win32 platform is supported right now. Maybe you're building on a different platform, or _WIN32 is not defined by your compiler.
#endif

namespace Platform
{
    struct Timer
    {
        u64 frequency; // Timer frequency, in counts/second.
        u64 start_count; // Count when the timer was started.
    };
    void TimerStart(Timer* timer);
    u64 TimerMeasureCounts(Timer* timer);
    u64 TimerCountsToMicroseconds(Timer* timer, u64 counts);

    bool IsConsoleVTEnabled();
    void PrintMessage(const char* message);
    void PrintError(const char* message);
	bool ShowAssertDialog(const char* message);

	s64 GetFileSize(IString path);
	Span<u8> ReadFileToBuffer(IString path);
    bool ReadFileToBuffer(IString path, Span<u8> buffer);
};
