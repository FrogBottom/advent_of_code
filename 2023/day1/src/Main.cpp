
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

// Returns a digit at the current offset if we were able to parse one, or -1 otherwise.
char ParseDigit(IString input, s32 offset)
{
    // Check for literal digits.
    if (input[offset] >= '0' && input[offset] <= '9') return input[offset] - '0';

    // Check for digit substrings.
    static IString lut[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    for (s32 i = 0; i < ARRAYCOUNT(lut); ++i)
    {
        if (offset + lut[i].Length() <= input.Length())
        {
            // If we found a substring, use it and return.
            if (memcmp(&input[offset], lut[i].Ptr(), lut[i].Length()) == 0) return i;
        }
    }

    // No digit found.
    return -1;
}

static s32 DoPartOne(IString input)
{
    s32 result = 0;
    // Scan each line.
    s32 file_offset = 0;
    for (s32 file_offset = 0; file_offset < input.Length(); ++file_offset)
    {
        s32 first_digit = -1; // First digit, or -1 if we haven't found a digit yet.
        s32 last_digit = 0;

        char c = input[file_offset];
        while (c != '\n' && file_offset < input.Length())
        {
            char digit = c - '0';
            if (digit >= 0 && digit <= 9)
            {
                if (first_digit < 0) first_digit = digit;
                last_digit = digit;
            }

            ++file_offset;
            if (file_offset < input.Length()) c = input[file_offset];
        }

        result += (first_digit * 10) + last_digit;
    }
    return result;
}

static s32 DoPartTwo(IString input)
{
    IString lut[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

    s32 result = 0;
    // Scan each line.
    s32 file_offset = 0;
    for (s32 file_offset = 0; file_offset < input.Length(); ++file_offset)
    {
        s32 first_digit = -1; // First digit, or -1 if we haven't found a digit yet.
        s32 last_digit = 0;

        while (input[file_offset] != '\n' && file_offset < input.Length())
        {
            char digit = ParseDigit(input, file_offset);

            if (digit >= 0)
            {
                if (first_digit < 0) first_digit = digit;
                last_digit = digit;
            }
            ++file_offset;
        }

        result += (first_digit * 10) + last_digit;
    }
    return result;
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
    s32 part1 = DoPartOne({(char*)input_file.ptr, (u32)input_file.count});
    u64 part1_counts = Platform::TimerMeasureCounts(&timer);

    s32 part2 = DoPartTwo({(char*)input_file.ptr, (u32)input_file.count});
    u64 part2_counts = Platform::TimerMeasureCounts(&timer);

    // Stop timing.
    u64 part1_us = Platform::TimerCountsToMicroseconds(&timer, part1_counts);
    u64 part2_us = Platform::TimerCountsToMicroseconds(&timer, part2_counts - part1_counts);

    // Print results.
    PrintF("Part 1: %d (Computed in %lldus)\nPart 2: %d (Computed in %lldus)\n", part1, part1_us, part2, part2_us);
    // Free the input file and exit.
    free(input_file.ptr);
    return 0;
}
