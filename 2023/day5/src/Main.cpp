
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

struct Range
{
    s64 dst;
    s64 src;
    s64 length;
};

static bool IsDigit(char c) {return (c >= '0' && c <= '9');}
static void SkipToNextDigit(char** input) {while (!IsDigit(*(*input))) ++(*input);}
static Range ParseRange(char** input)
{
    Range result = {};
    result.dst = strtoll(*input, input, 10);
    result.src = strtoll(*input, input, 10);
    result.length = strtoll(*input, input, 10);
    *input += 1;
    return result;
}

static s64 FollowMap(const TArray<Range>& map, s64 key)
{
    for (Range r : map)
    {
        if (key >= r.src && key < r.src + r.length)
        {
            return r.dst + (key - r.src);
        }
    }
    return key;
}

static s64 DoPartOne(IString input)
{
    char* next = (char*)input.Ptr(); // const cast, oof
    SkipToNextDigit(&next);
    TArray<s64> seeds = {};
    while (*next != '\n')
    {
        s64 i = strtoll(next, &next, 10);
        seeds.Append(i);
    }

    SkipToNextDigit(&next);
    TArray<Range> seed_to_soil = {};
    while (*next != '\n') seed_to_soil.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> soil_to_fertilizer = {};
    while (*next != '\n') soil_to_fertilizer.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> fertilizer_to_water = {};
    while (*next != '\n') fertilizer_to_water.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> water_to_light = {};
    while (*next != '\n') water_to_light.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> light_to_temperature = {};
    while (*next != '\n') light_to_temperature.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> temperature_to_humidity = {};
    while (*next != '\n') temperature_to_humidity.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> humidity_to_location = {};
    while ((next - input.Ptr()) < (s64)input.Length()) humidity_to_location.Append(ParseRange(&next));

    s64 smallest_location = S64_MAX;

    for (s64 seed : seeds)
    {
        s64 soil = FollowMap(seed_to_soil, seed);
        s64 fertilizer = FollowMap(soil_to_fertilizer, soil);
        s64 water = FollowMap(fertilizer_to_water, fertilizer);
        s64 light = FollowMap(water_to_light, water);
        s64 temperature = FollowMap(light_to_temperature, light);
        s64 humidity = FollowMap(temperature_to_humidity, temperature);
        s64 location = FollowMap(humidity_to_location, humidity);

        if (location < smallest_location) smallest_location = location;
    }
    return smallest_location;
}

static s64 DoPartTwo(IString input)
{
char* next = (char*)input.Ptr(); // const cast, oof
    SkipToNextDigit(&next);
    TArray<Range> seeds = {};
    while (*next != '\n')
    {
        Range r = {};
        r.src = strtoll(next, &next, 10);
        r.length = strtoll(next, &next, 10);
        seeds.Append(r);
    }

    SkipToNextDigit(&next);
    TArray<Range> seed_to_soil = {};
    while (*next != '\n') seed_to_soil.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> soil_to_fertilizer = {};
    while (*next != '\n') soil_to_fertilizer.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> fertilizer_to_water = {};
    while (*next != '\n') fertilizer_to_water.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> water_to_light = {};
    while (*next != '\n') water_to_light.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> light_to_temperature = {};
    while (*next != '\n') light_to_temperature.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> temperature_to_humidity = {};
    while (*next != '\n') temperature_to_humidity.Append(ParseRange(&next));

    SkipToNextDigit(&next);
    TArray<Range> humidity_to_location = {};
    while ((next - input.Ptr()) < (s64)input.Length()) humidity_to_location.Append(ParseRange(&next));

    s64 smallest_location = S64_MAX;

    s64 sum_of_ranges = 0;
    for (Range seed_range : seeds)
    {
        sum_of_ranges += seed_range.length;
        PrintF("%lld\n", sum_of_ranges);
        // PrintF("Doing the next range...\n");
        // for (s64 s = seed_range.src; s < seed_range.src + seed_range.length; ++s)
        // {
        //     s64 soil = FollowMap(seed_to_soil, s);
        //     s64 fertilizer = FollowMap(soil_to_fertilizer, soil);
        //     s64 water = FollowMap(fertilizer_to_water, fertilizer);
        //     s64 light = FollowMap(water_to_light, water);
        //     s64 temperature = FollowMap(light_to_temperature, light);
        //     s64 humidity = FollowMap(temperature_to_humidity, temperature);
        //     s64 location = FollowMap(humidity_to_location, humidity);
        //     if (location < smallest_location) smallest_location = location;
        // }
    }
    return smallest_location;
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


