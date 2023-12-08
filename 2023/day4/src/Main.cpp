
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

s32 NumberOfBInA(const TArray<s32>& a, const TArray<s32>& b)
{
    s32 result = 0;
    for (s32 val : b) if (a.Contains(val)) ++result;
    return result;
}

static s32 DoPartOne(IString input)
{
    s32 total_score = 0;
    s32 line_length = (s32)(strchr(input.Ptr(), '\n') - input.Ptr());
    TArray<s32> winning_numbers = TArray<s32>(10);
    TArray<s32>your_numbers = TArray<s32>(25);
    for (s32 offset = 0; offset < input.Length(); offset += (line_length + 1))
    {
        const char* line = input.Ptr() + offset;

        s32 game_number;

        s32* a = &winning_numbers[0];
        s32* b = &your_numbers[0];

        // This is what clean code looks like.
        sscanf(line, "Card %d: %d %d %d %d %d %d %d %d %d %d | %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
        &game_number,
        a, a+1, a+2, a+3, a+4, a+5, a+6, a+7, a+8, a+9,
        b, b+1, b+2, b+3, b+4, b+5, b+6, b+7, b+8, b+9,
        b+10, b+11, b+12, b+13, b+14, b+15, b+16, b+17,
        b+18, b+19, b+20, b+21, b+22, b+23, b+24);

        s32 match_count = NumberOfBInA(winning_numbers, your_numbers);
        s32 score = (match_count) ? (1 << (match_count - 1)) : 0;
        total_score += score;
        PrintF("Game %d: Score %d, match count %d\n", game_number, score, match_count);
    }
    return total_score;
}

static s32 ComputeScoreRecursive(const TArray<s32>& games, s32 game_idx)
{
    s32 additional_games = games[game_idx];
    s32 added_score = additional_games;
    for (s32 i = 1; i <= additional_games; ++i) added_score += ComputeScoreRecursive(games, game_idx + i);
    return added_score;
}

static s32 DoPartTwo(IString input)
{
    s32 line_length = (s32)(strchr(input.Ptr(), '\n') - input.Ptr());
    TArray<s32> winning_numbers = TArray<s32>(10);
    TArray<s32>your_numbers = TArray<s32>(25);

    TArray<s32> games = {};

    for (s32 offset = 0; offset < input.Length(); offset += (line_length + 1))
    {
        const char* line = input.Ptr() + offset;

        s32 game_number;

        s32* a = &winning_numbers[0];
        s32* b = &your_numbers[0];

        // This is what clean code looks like.
        sscanf(line, "Card %d: %d %d %d %d %d %d %d %d %d %d | %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
        &game_number,
        a, a+1, a+2, a+3, a+4, a+5, a+6, a+7, a+8, a+9,
        b, b+1, b+2, b+3, b+4, b+5, b+6, b+7, b+8, b+9,
        b+10, b+11, b+12, b+13, b+14, b+15, b+16, b+17,
        b+18, b+19, b+20, b+21, b+22, b+23, b+24);

        s32 match_count = NumberOfBInA(winning_numbers, your_numbers);
        games.Append(match_count);
    }

    s32 total_games = games.Length();
    for (s32 i = 0; i < games.Length(); ++i) total_games += ComputeScoreRecursive(games, i);
    return total_games;
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


