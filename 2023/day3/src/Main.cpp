
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Parses a number out of the input starting at offset, updating the offset to the end of the parsed number.
// Doesn't parse negative numbers.
static s32 ParseNumber(char* input, s32* offset)
{
    s32 result = 0;
    while (input[*offset] >= '0' && input[*offset] <= '9')
    {
        s32 digit = input[*offset] - '0';
        result = result * 10 + digit;
        *offset += 1;
    }

    return result;
}

bool IsDigit(char c) {return ((c >= '0' && c <= '9'));}
bool IsSymbol(char c) {return ((c < '0' || c > '9') && c != '.');}

static char* PadBuffer(IString input, s32 original_line_length, s32 original_line_count, s32* out_line_stride, s32* out_offset)
{
    s32 line_length = original_line_length + 2;
    s32 out_line_count = original_line_count + 2;

    *out_line_stride = line_length + 1;
    *out_offset = (*out_line_stride) + 1;

    s32 out_size = (line_length + 1) * (original_line_count + 2);
    char* buf = (char*)malloc(out_size);

    for (s32 i = 0; i < line_length; ++i) buf[i] = '.';
    buf[line_length] = '\n';
    for (s32 line = 1; line < (out_line_count - 1); ++line)
    {
        char* offset = buf + ((*out_line_stride) * line);
        const char* original_offset = input.Ptr() + (original_line_length + 1) * (line - 1);
        offset[0] = '.';
        offset[line_length - 1] = '.';
        offset[line_length] = '\n';
        memcpy(&offset[1], original_offset, original_line_length);
    }

    for (s32 i = *out_line_stride * (out_line_count - 1); i < out_size; ++i) buf[i] = '.';
    buf[out_size - 1] = '\0';

    return buf;
}

static s32 DoPartOne(IString input)
{
    s32 line_length = 0;
    while (input[line_length] != '\n') ++line_length;
    s32 line_count = ((s32)input.Length() / (line_length + 1)) + 1; // Add one, since the last line doesn't have a newline.

    s32 line_stride = 0;
    s32 start_offset = 0;
    char* buf = PadBuffer(input, line_length, line_count, &line_stride, &start_offset);


    s32 result = 0;
    // Iterate through all lines.
    for (s32 line_idx = 0; line_idx < line_count; ++line_idx)
    {
        char* line = buf + start_offset + (line_stride *  line_idx);

        // Iterate through characters, looking for symbols.
        for (s32 i = 0; i < line_length; ++i)
        {
            if (IsSymbol(line[i]))
            {
                // Scan for nearby numbers.
                for (s32 y = -1; y <= 1; ++y)
                {
                    for (s32 x = -1; x <= 1; ++x)
                    {
                        s32 j = i + x + (line_stride * y);

                        if (IsDigit(line[j]))
                        {
                            // Scan left to find the start of the number.
                            s32 start_idx = j;
                            while (IsDigit(line[start_idx])) --start_idx;
                            ++start_idx;

                            s32 number = ParseNumber(line, &start_idx);
                            result += number;
                            // Scan right to skip the remainder of the number.
                            while (IsDigit(line[j]))
                            {
                                ++j;
                                ++x;
                            }
                        }
                    }
                }
            }
        }
    }

    free(buf);
    return result;
}

static s32 DoPartTwo(IString input)
{
    s32 line_length = 0;
    while (input[line_length] != '\n') ++line_length;
    s32 line_count = ((s32)input.Length() / (line_length + 1)) + 1; // Add one, since the last line doesn't have a newline.

    s32 line_stride = 0;
    s32 start_offset = 0;
    char* buf = PadBuffer(input, line_length, line_count, &line_stride, &start_offset);


    s32 result = 0;
    // Iterate through all lines.
    for (s32 line_idx = 0; line_idx < line_count; ++line_idx)
    {
        char* line = buf + start_offset + (line_stride *  line_idx);

        // Iterate through characters, looking for symbols.
        for (s32 i = 0; i < line_length; ++i)
        {
            if (IsSymbol(line[i]))
            {
                s32 accumulator = 1;
                s32 counter = 0;
                // Scan for nearby numbers.
                for (s32 y = -1; y <= 1; ++y)
                {
                    for (s32 x = -1; x <= 1; ++x)
                    {
                        s32 j = i + x + (line_stride * y);

                        if (IsDigit(line[j]))
                        {
                            // Scan left to find the start of the number.
                            s32 start_idx = j;
                            while (IsDigit(line[start_idx])) --start_idx;
                            ++start_idx;

                            s32 number = ParseNumber(line, &start_idx);

                            accumulator *= number;
                            ++counter;
                            // Scan right to skip the remainder of the number.
                            while (IsDigit(line[j])) {++j;++x;}
                        }
                    }
                }

                if (counter == 2) result += accumulator;
            }
        }
    }

    free(buf);
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


