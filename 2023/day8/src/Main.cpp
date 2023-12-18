
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

#define ENCODE_KEY(a,b,c) ((u32)(((a)<<16) | ((b)<<8) | (c)))
#define ENCODE_KEY2(a,b,c) ((u16)((((a) - 'A') << 10) | (((b) - 'A') << 5) | ((c) - 'A')))
#define ENCODE_NODE2(left, right) ((u32)(((left) << 16) | (right)))

#define DECODE_LEFT(node) ((u16)((node) >> 16))
#define DECODE_RIGHT(node) ((u16)((node) & 0xffff))

#define IS_START2(key) ((bool)(((key) & 0x1f) == 0))
#define IS_END2(key) ((bool)(((key) & 0x1f) == 25))

struct Node
{
    u32 left;
    u32 right;
};

struct NodePair
{
    u32 key;
    Node value;
};

static inline bool IsStartingNode(u32 key) {return ((key & 0xff) == 'A');}
static inline bool IsEndingNode(u32 key) {return ((key & 0xff) == 'Z');}

// static inline bool IsStartingNode2(u16 key) {return ((key & 0x1f) == ('A' - 'A'));}
// static inline bool IsEndingNode2(u16 key) {return ((key & 0x1f) == ('Z' - 'A'));}


static s64 DoPartOne(Span<char> input)
{
    NodePair* map = 0;

    char* instructions = input.ptr;

    s32 offset = 0;
    while (input[offset] != '\n') ++offset;
    s32 instructions_length = offset;
    input[offset] = '\0';
    offset += 2;

    while (offset < input.count)
    {
        u32 key = ENCODE_KEY(input[offset], input[offset + 1], input[offset + 2]);
        Node value = {ENCODE_KEY(input[offset + 7], input[offset + 8], input[offset + 9]), ENCODE_KEY(input[offset + 12], input[offset + 13], input[offset + 14])};
        //PrintF("Key: %c%c%c, IsStart: %d, IsEnd: %d\n", input[offset], input[offset + 1], input[offset + 2], IsStartingNode(key), IsEndingNode(key));
        offset += 17;
        hmput(map, key, value);
    }

    s64 step_count = 0;

    s32 instructions_offset = 0;
    u32 current_node = ENCODE_KEY('A', 'A', 'A');
    while (current_node != ENCODE_KEY('Z', 'Z', 'Z'))
    {
        bool is_left = (instructions[instructions_offset] == 'L');
        instructions_offset = (instructions_offset + 1) % instructions_length;
        current_node = (is_left) ? hmget(map, current_node).left : hmget(map, current_node).right;
        ++step_count;
    }

    return step_count;
}

s64 GreatestCommonDivisor(s64 a, s64 b) {return (b) ? GreatestCommonDivisor(b, a % b) : a;}
s64 LeastCommonMultiple(s32* in, s32 count)
{
    Assert(count > 0);

    s64 result = in[0];
    for (s32 i = 1; i < count; ++i)
    {
        result = in[i] * (result / GreatestCommonDivisor(in[i], result));
    }
    return result;
}

static s64 DoPartTwo(Span<char> input)
{
   u32 map[1<<15];

    TArray<u16> simultaneous_nodes = {};

    char* instructions = input.ptr;

    s32 offset = 0;
    while (input[offset] != '\n') ++offset;
    s32 instructions_length = offset;
    input[offset] = '\0';
    offset += 2;

    // Replace the L and R sequence with a 0 and 1 sequence, just to make checking slightly easier.
    for (s32 i = 0; i < instructions_length; ++i) instructions[i] = (instructions[i] == 'L') ? 0 : 1;

    // Encode the map in a big lookup table. Since each letter can hold 26 values, that fits in 5 bits (where 'A' == 0 and 'Z' == 25).
    // We pack the three letters into 15 bits of a 16 bit key, and then create a lookup table of all 32768 possible keys.

    while (offset < input.count)
    {
        u16 key = ENCODE_KEY2(input[offset], input[offset + 1], input[offset + 2]);
        u16 left = ENCODE_KEY2(input[offset + 7], input[offset + 8], input[offset + 9]);
        u16 right = ENCODE_KEY2(input[offset + 12], input[offset + 13], input[offset + 14]);
        map[key] = ENCODE_NODE2(left, right);

        if (IS_START2(key)) simultaneous_nodes.Append(key);
        offset += 17;
    }

    TArray<s32> cycle_lengths = {};
    s32 instructions_offset = 0;

    for (u16 start : simultaneous_nodes)
    {
        s32 cycle_length = 0;
        u16 current = start;
        while (!IS_END2(current))
        {
            bool is_left = (instructions[instructions_offset]);
            u32 node = map[current];
            current = (is_left) ? DECODE_RIGHT(node) : DECODE_LEFT(node);

            cycle_length += 1;
            instructions_offset += 1;
            if (instructions_offset == instructions_length) instructions_offset = 0;
        }
        cycle_lengths.Append(cycle_length);
    }

    return LeastCommonMultiple(&cycle_lengths[0], cycle_lengths.Length());
}

int main(int argc, char* argv[])
{
    // Read the input file into a buffer.
    IString path = (argc > 1) ? argv[1] : DEFAULT_INPUT_PATH;
    Span<u8> input_file1 = Platform::ReadFileToBuffer(path);
    Span<u8> input_file2 = Platform::ReadFileToBuffer(path);
    // Start timing.
    Platform::Timer timer = {};
    Platform::TimerStart(&timer);

    // Do the actual work.
    //s64 part1 = DoPartOne({(char*)input_file1.ptr, (u32)input_file1.count});
    s64 part1 = 0;
    u64 part1_counts = Platform::TimerMeasureCounts(&timer);

    s64 part2 = DoPartTwo({(char*)input_file2.ptr, (u32)input_file2.count});
    u64 part2_counts = Platform::TimerMeasureCounts(&timer);

    // Stop timing.
    u64 part1_us = Platform::TimerCountsToMicroseconds(&timer, part1_counts);
    u64 part2_us = Platform::TimerCountsToMicroseconds(&timer, part2_counts - part1_counts);

    // Print results.
    PrintF("Part 1: %lld (Computed in %lldus)\nPart 2: %lld (Computed in %lldus)\n", part1, part1_us, part2, part2_us);
    // Free the input file and exit.
    free(input_file1.ptr);
    free(input_file2.ptr);
    return 0;
}


