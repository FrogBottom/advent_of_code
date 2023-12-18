
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

static s32 ParseNumber(const char* input, s32* offset)
{
    s32 result = 0;
    bool is_negative = (input[*offset] == '-');
    if (is_negative) *offset += 1;
    while (input[*offset] >= '0' && input[*offset] <= '9')
    {
        s32 digit = input[*offset] - '0';
        result = result * 10 + digit;
        *offset += 1;
    }
    if (is_negative) result *= -1;
    return result;
}

struct Sequence
{
    TArray<s32> sequence;
    s32 smallest_length; // Length of the final substring containing all zeros.
    s32 count; // Number of total sub-sequences, including the initial one.
};

bool GenerateSubSequence(Sequence* s, s32 parent_start)
{
    bool is_all_zeros = true;

    s32 old_length = s->sequence.Length();
    if (old_length - parent_start < 2) Assert(false);

    for (s32 idx = parent_start + 1; idx < old_length; ++idx)
    {
        s32 diff = s->sequence[idx] - s->sequence[idx - 1];
        s->sequence.Append(diff);
        if (diff) is_all_zeros = false;
    }

    s->count += 1;
    s->smallest_length -= 1;

    return is_all_zeros;
}

s32 PredictNext(Sequence* s)
{
    s32 length = s->smallest_length;
    s32 idx = s->sequence.Length() - length - 1;
    s32 current = s->sequence[idx + length];
    while (idx >= 0)
    {
        current = s->sequence[idx] + current;
        length += 1;
        idx -= length;
    }

    return current;
}

s32 PredictPrevious(Sequence* s)
{
    s32 length = s->smallest_length;
    s32 idx = s->sequence.Length() - 1 - length - length;
    length += 1;
    s32 current = s->sequence[idx + length];
    while (idx >= 0)
    {
        current = s->sequence[idx] - current;
        length += 1;
        idx -= length;
    }

    return current;
}

static s64 DoPartOne(Span<char> input)
{
    s32 offset = 0;
    TArray<Sequence> sequences = {};
    while (offset < (s32)input.count)
    {
        Sequence s = {};
        bool end_of_line = false;
        while (!end_of_line && offset < (s32)input.count)
        {
            s32 num = ParseNumber(input.ptr, &offset);
            s.sequence.Append(num);
            end_of_line = (input[offset] == '\n');
            offset += 1; // Skip past the space (or newline, if we just encountered it).
        }
        s.count = 1;
        s.smallest_length = s.sequence.Length();
        sequences.Append(s);
    }

    for (Sequence& s : sequences)
    {
        s32 last_start = 0;
        s32 next_start = s.sequence.Length();
        while (!GenerateSubSequence(&s, last_start))
        {
            last_start = next_start;
            next_start = s.sequence.Length();
        }
    }

    s64 result = 0;

    for (Sequence& s : sequences) result += (s64)PredictNext(&s);

    // Sequence* test_sequence = &sequences[2];
    // s32 last_start = 0;
    // s32 next_start = test_sequence->sequence.Length();
    // while (!GenerateSubSequence(test_sequence, last_start))
    // {
    //     last_start = next_start;
    //     next_start = test_sequence->sequence.Length();
    // }

    // s32 next = PredictNext(test_sequence);
    // s32 prev = PredictPrevious(test_sequence);
    // for (s32 num : test_sequence->sequence) PrintF("%d ", num);
    // PrintF("\n");
    // PrintF("Next: %d, Prev: %d\n", next, prev);

    // for (Sequence& s : sequences)
    // {
    //     for (s32 num : s.sequence) PrintF("%d ", num);
    //     PrintF("\n");
    // }
    return result;
}

static s64 DoPartTwo(Span<char> input)
{
    s32 offset = 0;
    TArray<Sequence> sequences = {};
    while (offset < (s32)input.count)
    {
        Sequence s = {};
        bool end_of_line = false;
        while (!end_of_line && offset < (s32)input.count)
        {
            s32 num = ParseNumber(input.ptr, &offset);
            s.sequence.Append(num);
            end_of_line = (input[offset] == '\n');
            offset += 1; // Skip past the space (or newline, if we just encountered it).
        }
        s.count = 1;
        s.smallest_length = s.sequence.Length();
        sequences.Append(s);
    }

    for (Sequence& s : sequences)
    {
        s32 last_start = 0;
        s32 next_start = s.sequence.Length();
        while (!GenerateSubSequence(&s, last_start))
        {
            last_start = next_start;
            next_start = s.sequence.Length();
        }
    }

    s64 result = 0;

    for (Sequence& s : sequences) result += (s64)PredictPrevious(&s);
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


