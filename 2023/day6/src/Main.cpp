
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"


static s64 ComputeDistance(s64 alloted_time, s64 held_time) {return held_time * alloted_time - (held_time * held_time);}

static s64 DoPartOne(IString input)
{
    s64 times[4] = {};
    s64 distances[4] = {};

    sscanf(input.Ptr(), "Time: %lld %lld %lld %lld\nDistance: %lld%lld%lld%lld", times, times+1, times+2, times+3, distances, distances+1, distances+2, distances+3);

    s64 result = 1;
    for (s64 race = 0; race < ARRAYCOUNT(times); ++race)
    {
        s64 alloted_time = times[race];
        s64 distance_record = distances[race];

        s64 attempts_which_succeed = 0;
        for (s64 attempt = 1; attempt < alloted_time; ++attempt)
        {
            s64 distance = ComputeDistance(alloted_time, attempt);
            if (distance > distance_record) ++attempts_which_succeed;
        }

        result *= attempts_which_succeed;
    }
    return result;
}

static s64 DoPartTwo(IString input)
{
    s64 alloted_time = 46807866;
    s64 distance_record = 214117714021024;

    s64 attempts_which_succeed = 0;
    for (s64 attempt = 1; attempt < alloted_time; ++attempt)
    {
        s64 distance = ComputeDistance(alloted_time, attempt);
        if (distance > distance_record) ++attempts_which_succeed;
    }

    return attempts_which_succeed;
}

int main(int argc, char* argv[])
{
    // Read the input file into a buffer.
    IString path = (argc > 1) ? argv[1] : DEFAULT_INPUT_PATH;
    Span<u8> input_file = Platform::ReadFileToBuffer(path);

    // Start timing.
    Platform::Timer timer = {};
    Platform::TimerStart(&timer);

    // Do the actual work.
    s64 part1 = DoPartOne({(char*)input_file.ptr, (u32)input_file.count});
    u64 part1_counts = Platform::TimerMeasureCounts(&timer);

    s64 part2 = DoPartTwo({(char*)input_file.ptr, (u32)input_file.count});
    u64 part2_counts = Platform::TimerMeasureCounts(&timer);

    // Stop timing.
    u64 part1_us = Platform::TimerCountsToMicroseconds(&timer, part1_counts);
    u64 part2_us = Platform::TimerCountsToMicroseconds(&timer, part2_counts - part1_counts);

    // Print results.
    PrintF("Part 1: %lld (Computed in %lldus)\nPart 2: %lld (Computed in %lldus)\n", part1, part1_us, part2, part2_us);
    // Free the input file and exit.
    free(input_file.ptr);
    return 0;
}


