
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

#define MAX_RED 12
#define MAX_GREEN 13
#define MAX_BLUE 14

// Parses a number out of the input starting at offset, updating the offset to the end of the parsed number.
// Doesn't parse negative numbers.
static s32 ParseNumber(IString input, s32* offset)
{
    s32 result = 0;
    while (*offset < input.Length() && input[*offset] >= '0' && input[*offset] <= '9')
    {
        s32 digit = input[*offset] - '0';
        result = result * 10 + digit;
        *offset += 1;
    }

    return result;
}


static s32 DoPartOne(IString input)
{
    s32 result = 0;
    s32 last_id = 0;

    // Iterate through lines.
    for (s32 file_offset = 0; file_offset < input.Length(); ++file_offset)
    {
        // We will check this at the end of the line to see if this game was possible.
        bool was_game_possible = true;

        // Parse ID. We end on the index of the colon.
        s32 id = last_id + 1;
        ++last_id;
        while (input[file_offset] != ':') ++file_offset;

        // Iterate through hands in a line.
        while (file_offset < input.Length() && input[file_offset] != '\n')
        {
            Assert(input[file_offset] == ';' || input[file_offset] == ':');

            // Iterate through colors in a hand.
            do
            {
                file_offset += 2; // Skip to the start of the next number.
                s32 number = ParseNumber(input, &file_offset);
                file_offset += 1; // Skip the space to get to the start of the color.

                switch (input[file_offset])
                {
                    case 'r':
                    {
                        file_offset += 3;
                        if (number > MAX_RED) was_game_possible = false;
                        break;
                    }
                    case 'g':
                    {
                        file_offset += 5;
                        if (number > MAX_GREEN) was_game_possible = false;
                        break;
                    }
                    case 'b':
                    {
                        file_offset += 4;
                        if (number > MAX_BLUE) was_game_possible = false;
                        break;
                    }
                    default: Assert(false);
                }
            } while (file_offset < input.Length() && input[file_offset] == ',');
        }
        ++file_offset;

        if (was_game_possible) result += id;
    }
    return result;
}

static s32 DoPartTwo(IString input)
{
    s32 result = 0;
    s32 last_id = 0;

    // Iterate through lines.
    for (s32 file_offset = 0; file_offset < input.Length(); ++file_offset)
    {
        // We will check this at the end of the line to see if this game was possible.
        s32 min_red = 0;
        s32 min_green = 0;
        s32 min_blue = 0;

        // Parse ID. We end on the index of the colon.
        s32 id = last_id + 1;
        ++last_id;
        while (input[file_offset] != ':') ++file_offset;

        // Iterate through hands in a line.
        while (file_offset < input.Length() && input[file_offset] != '\n')
        {
            Assert(input[file_offset] == ';' || input[file_offset] == ':');

            // Iterate through colors in a hand.
            do
            {
                file_offset += 2; // Skip to the start of the next number.
                s32 number = ParseNumber(input, &file_offset);
                file_offset += 1; // Skip the space to get to the start of the color.

                switch (input[file_offset])
                {
                    case 'r':
                    {
                        file_offset += 3;
                        if (number > min_red) min_red = number;
                        break;
                    }
                    case 'g':
                    {
                        file_offset += 5;
                        if (number > min_green) min_green = number;
                        break;
                    }
                    case 'b':
                    {
                        file_offset += 4;
                        if (number > min_blue) min_blue = number;
                        break;
                    }
                    default: Assert(false);
                }
            } while (file_offset < input.Length() && input[file_offset] == ',');
        }
        ++file_offset;

        result += (min_red * min_green * min_blue);
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


