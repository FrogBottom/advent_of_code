
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

#define RIGHT 0x01
#define UP 0x02
#define LEFT 0x04
#define DOWN 0x08
#define START 0x10
#define LOOP 0x20
#define INSIDE 0x40
#define LOOP_CROSSING 0x80
#define DIRECTIONS_MASK 0x0f

struct Map
{
    u8* data;
    s32 width;
    s32 height;
    s32 start_x;
    s32 start_y;

    u8& operator() (s32 x, s32 y) {u8 d = 0; return (x < 0 || x >= width || y < 0 || y >= height) ? d : data[y * width + x];}
    u8& operator() (s32 i) {u8 d = 0; return (i < 0 || i >= width * height) ? d : data[i];}
};

u8 TranslateSymbol(char symbol)
{
    switch (symbol)
    {
        case '|': return UP | DOWN;
        case '-': return LEFT | RIGHT;
        case 'L': return UP | RIGHT;
        case 'J': return UP | LEFT;
        case '7': return DOWN | LEFT;
        case 'F': return DOWN | RIGHT;
        case '.': return 0;
        case 'S': return START;
        default: Assert(false); return 0;
    }
}

void FixConnections(Map& map, s32 x, s32 y)
{
        u8& v     = map(x, y);
        u8  right = map(x + 1, y);
        u8  up    = map(x, y - 1);
        u8  left  = map(x - 1, y);
        u8  down  = map(x, y + 1);

        if ((v & RIGHT) && !(right & (LEFT  | START))) v &= ~RIGHT;
        if ((v & UP)    && !(up    & (DOWN  | START))) v &= ~UP;
        if ((v & LEFT)  && !(left  & (RIGHT | START))) v &= ~LEFT;
        if ((v & DOWN)  && !(down  & (UP    | START))) v &= ~DOWN;

        if (v & START)
        {
            map.start_x = x;
            map.start_y = y;
            if (right & LEFT)  v |= RIGHT;
            if (up    & DOWN)  v |= UP;
            if (left  & RIGHT) v |= LEFT;
            if (down  & UP)    v |= DOWN;
        }
}

// Uses the input x, y, and coming_from values to traverse to the next cell.
// Updates the x, y, and coming_from values accordingly.
void FollowPipe(Map& map, s32& x, s32& y, u8& from)
{
    u8 to = (map(x, y) & DIRECTIONS_MASK) & ~from;
    switch (to)
    {
        case RIGHT: x += 1; from = LEFT;  return;
        case UP:    y -= 1; from = DOWN;  return;
        case LEFT:  x -= 1; from = RIGHT; return;
        case DOWN:  y += 1; from = UP;    return;
        default: Assert(false); return;
    }
}

const char* CellToString(u8 cell)
{
    if (cell & INSIDE)       return "*";
    else if (!(cell & LOOP)) return " ";
    else if (cell & START)   return "S";

    switch(cell & DIRECTIONS_MASK)
    {
        case 0:     return " ";
        case RIGHT: return " ";
        case UP:    return " ";
        case LEFT:  return " ";
        case DOWN:  return " ";
        case (UP   | DOWN):  return "│";
        case (LEFT | RIGHT): return "─";
        case (UP   | RIGHT): return "╰";
        case (UP   | LEFT):  return "╯";
        case (DOWN | LEFT):  return "╮";
        case (DOWN | RIGHT): return "╭";
        default: Assert(false); return "X";
    }
}

void PrintMap(Map& map)
{
    for (s32 y = 0; y < map.height; ++y)
    {
        for (s32 x = 0; x < map.width; ++x)
        {
            u8 cell = map(x, y);
            if (cell & LOOP_CROSSING) {PrintF("\033[31m%s\033[0m", CellToString(cell));}
            else if (cell & INSIDE)   {PrintF("\033[35m%s\033[0m", CellToString(cell));}
            else                      {PrintF("%s", CellToString(map(x, y)));}
        }
        PrintF("\n");
    }
}

Map ParseInput(Span<char> input)
{
    s32 cols = 0;
    while (input[cols] != '\n') ++cols;
    s32 rows = (s32)input.count / (cols + 1);
    if (input[input.count - 1] != '\n') rows += 1; // Include the last row even if it doesn't have a line feed at the end.

    Map map = {(u8*)malloc(rows * cols), cols, rows};
    for (s32 y = 0; y < rows; ++y) for (s32 x = 0; x < cols; ++x) map(x, y) = TranslateSymbol(input[(rows + 1) * y + x]);
    for (s32 y = 0; y < rows; ++y) for (s32 x = 0; x < cols; ++x) FixConnections(map, x, y);
    return map;
}

static s64 DoPartOne(Span<char> input)
{
    Map map = ParseInput(input);
    s32 x1 = map.start_x;
    s32 x2 = map.start_x;
    s32 y1 = map.start_y;
    s32 y2 = map.start_y;
    u8 from1 = 0;
    u8 from2 = 0;
    s64 count = 1;

    s32* x = &x1;
    s32* y = &y1;
    u8* from = &from1;

    if (map(map.start_x, map.start_y) & RIGHT) {*x += 1; *from = LEFT;  x = &x2; y = &y2; from = &from2;}
    if (map(map.start_x, map.start_y) & UP)    {*y -= 1; *from = DOWN;  x = &x2; y = &y2; from = &from2;}
    if (map(map.start_x, map.start_y) & LEFT)  {*x -= 1; *from = RIGHT; x = &x2; y = &y2; from = &from2;}
    if (map(map.start_x, map.start_y) & DOWN)  {*y += 1; *from = UP;}

    do
    {
        FollowPipe(map, x1, y1, from1);
        FollowPipe(map, x2, y2, from2);
        count += 1;
    } while (!(x1 == x2 && y1 == y2));

    free(map.data);
    return count;
}

static s64 DoPartTwo(Span<char> input)
{
    // General strategy: Scan across each row. If we have crossed the loop an odd number of times, we are inside.
    Map map = ParseInput(input);
    s32 x = map.start_x;
    s32 y = map.start_y;
    u8 from = 0;
    if (map(map.start_x, map.start_y) & RIGHT)      {x += 1; from = LEFT;}
    else if (map(map.start_x, map.start_y) & UP)    {y -= 1; from = DOWN;}
    else if (map(map.start_x, map.start_y) & LEFT)  {x -= 1; from = RIGHT;}
    else if (map(map.start_x, map.start_y) & DOWN)  {y += 1; from = UP;}
    else Assert(false);

    s32 initial_x = x;
    s32 initial_y = y;
    do
    {
        u8 to = (map(x, y) & DIRECTIONS_MASK) & ~from;

        // Mark all cells in the path as members of the loop. If the pipe is fully vertical,
        // it is a crossing. Otherwise, we arbitrarily pick from=DOWN and to=DOWN to count as crossings.
        // This prevents horizontal lines from counting as more than one loop-crossing.
        u8 mask = LOOP;
        if ((from == UP || from == DOWN) && (to == UP || to == DOWN)) mask |= LOOP_CROSSING;
        else if (from == DOWN || to == DOWN) mask |= LOOP_CROSSING;
        map(x, y) |= mask;
        FollowPipe(map, x, y, from);
    } while (!(x == initial_x && y == initial_y));

    // Iterate across each row, toggling is_inside every time we cross over the loop.
    // Count the number of non-loop cells that we find while is_inside is true. This is our result.
    s64 count = 0;
    for (s32 y = 0; y < map.height; ++y)
    {
        bool is_inside = false;
        for (s32 x = 0; x < map.width; ++x)
        {
            u8& v = map(x, y);
            bool is_loop = (v & LOOP);
            if (v & LOOP_CROSSING) is_inside = !is_inside;
            if (is_inside && !(v & LOOP)) {v |= INSIDE; count += 1;}
        }
    }

    // PrintMap(map);

    free(map.data);
    return count;
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


