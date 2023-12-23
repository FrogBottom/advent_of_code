
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"
#define ABS(v) (((v) >= 0) ? (v) : -(v))
#define PART_TWO_GALAXY_SIZE 1000000

struct Galaxy {s32 x; s32 y;};

s32 DistanceBetween(Galaxy a, Galaxy b)
{
    return ABS(a.x - b.x) + ABS(a.y - b.y);
}

static s64 DoPartOne(Span<char> input)
{
    s32 cols = 0;
    while (input[cols] != '\n') ++cols;
    s32 rows = (s32)input.count / (cols + 1);
    if (input[input.count - 1] != '\n') ++rows; // If the last line isn't null terminated, we will have rounded incorrectly, so add one to the row count.

    // Create a list of row and column indices which are empty.
    TArray<s32> empty_cols = {};
    TArray<s32> empty_rows = {};

    // Scan each column, appending its index to the array if we find that it is empty.
    for (s32 col = 0; col < cols; ++col)
    {
        bool is_empty = true;
        for (s32 row = 0; row < rows; ++row) if (input[row * (cols + 1) + col] != '.') {is_empty = false; break;}
        if (is_empty) empty_cols.Append(col);
    }

    // Do the same, but for empty rows.
    for (s32 row = 0; row < rows; ++row)
    {
        bool is_empty = true;
        for (s32 col = 0; col < cols; ++col) if (input[row * (cols + 1) + col] != '.') {is_empty = false; break;}
        if (is_empty) empty_rows.Append(row);
    }

    // Scan for galaxies. We could have done all three of these scans in one pass, if we cared about parsing speed.
    TArray<Galaxy> galaxies = {};
    for (s32 row = 0; row < rows; ++row)
    {
        for (s32 col = 0; col < cols; ++col) if (input[row * (cols + 1) + col] == '#') galaxies.Append({col, row});
    }

    for (Galaxy& g : galaxies)
    {
        for (s32 i = empty_cols.Length() - 1; i >= 0; --i) if (g.x > empty_cols[i]) {g.x += i + 1; break;}
        for (s32 i = empty_rows.Length() - 1; i >= 0; --i) if (g.y > empty_rows[i]) {g.y += i + 1; break;}
    }

    s64 total_length = 0;

    for (s32 i = 0; i < galaxies.Length(); ++i)
    {
        for (s32 j = 0; j < galaxies.Length(); ++j)
        {
            if (j >= i) break;
            total_length += DistanceBetween(galaxies[i], galaxies[j]);
        }
    }

    return total_length;
}

static s64 DoPartTwo(Span<char> input)
{
    s32 cols = 0;
    while (input[cols] != '\n') ++cols;
    s32 rows = (s32)input.count / (cols + 1);
    if (input[input.count - 1] != '\n') ++rows; // If the last line isn't null terminated, we will have rounded incorrectly, so add one to the row count.

    // Create a list of row and column indices which are empty.
    TArray<s32> empty_cols = {};
    TArray<s32> empty_rows = {};

    // Scan each column, appending its index to the array if we find that it is empty.
    for (s32 col = 0; col < cols; ++col)
    {
        bool is_empty = true;
        for (s32 row = 0; row < rows; ++row) if (input[row * (cols + 1) + col] != '.') {is_empty = false; break;}
        if (is_empty) empty_cols.Append(col);
    }

    // Do the same, but for empty rows.
    for (s32 row = 0; row < rows; ++row)
    {
        bool is_empty = true;
        for (s32 col = 0; col < cols; ++col) if (input[row * (cols + 1) + col] != '.') {is_empty = false; break;}
        if (is_empty) empty_rows.Append(row);
    }

    // Scan for galaxies. We could have done all three of these scans in one pass, if we cared about parsing speed.
    TArray<Galaxy> galaxies = {};
    for (s32 row = 0; row < rows; ++row)
    {
        for (s32 col = 0; col < cols; ++col) if (input[row * (cols + 1) + col] == '#') galaxies.Append({col, row});
    }

    for (Galaxy& g : galaxies)
    {
        for (s32 i = empty_cols.Length() - 1; i >= 0; --i) if (g.x > empty_cols[i]) {g.x += ((i + 1) * PART_TWO_GALAXY_SIZE) - (i + 1); break;}
        for (s32 i = empty_rows.Length() - 1; i >= 0; --i) if (g.y > empty_rows[i]) {g.y += ((i + 1) * PART_TWO_GALAXY_SIZE) - (i + 1); break;}
    }

    s64 total_length = 0;

    for (s32 i = 0; i < galaxies.Length(); ++i)
    {
        for (s32 j = 0; j < galaxies.Length(); ++j)
        {
            if (j >= i) break;
            s32 len = DistanceBetween(galaxies[i], galaxies[j]);
            total_length += len;
        }
    }

    return total_length;
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
    s64 part1 = DoPartOne({(char*)input_file.ptr, input_file.count});
    u64 part1_counts = Platform::TimerMeasureCounts(&timer);

    s64 part2 = DoPartTwo({(char*)input_file.ptr, input_file.count});
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


