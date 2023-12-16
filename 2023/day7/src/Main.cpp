
#include "Core/EngineCore.h"
#include "Platform/Platform.h"

#define DEFAULT_INPUT_PATH "input.txt"

// Parses a number out of the input starting at offset, updating the offset to the end of the parsed number.
// Doesn't parse negative numbers.
static s32 ParseNumber(const char* input, s32* offset)
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

enum class HandType
{
    None = 0, // This is an error if we encounter it.
    HighCard,
    OnePair,
    TwoPairs,
    ThreeOfAKind,
    FullHouse,
    FourOfAKind,
    FiveOfAKind
};

struct Hand
{
    char* cards;
    HandType type;
    s32 bid;
};

HandType GetHandTypePartOne(Span<char> input, Hand hand, TArray<char>& card_buckets)
{
    // Create a bucket for each distinct type of card that we encounter.
    // Store the number of  cards in each bucket. For example, the hand AAJJ3
    // would have three buckets, A, J, and 3, with counts 2, 2, and 1.
    s32 bucket_counts[5] = {};
    card_buckets.SetLength(0);

    for (s32 card_idx = 0; card_idx < 5; ++card_idx)
    {
        char card = hand.cards[card_idx];
        s32 bucket_idx = card_buckets.IndexOf(card);
        if (bucket_idx >= 0) bucket_counts[bucket_idx] += 1;
        else
        {
            bucket_counts[card_buckets.Length()] = 1;
            card_buckets.Append(card);
        }
    }

    // Find the count of the largest bucket.
    s32 max_bucket_size = 0;
    for (s32 i = 0; i < card_buckets.Length(); ++i) if (bucket_counts[i] > max_bucket_size) max_bucket_size = bucket_counts[i];

    // Depending on the size of the largest bucket and the number of buckets,
    // we can determine what type of hand this is.
    switch (card_buckets.Length())
    {
        case 1: return HandType::FiveOfAKind;
        case 2: return (max_bucket_size == 4) ? HandType::FourOfAKind : HandType::FullHouse;
        case 3: return (max_bucket_size == 3) ? HandType::ThreeOfAKind : HandType::TwoPairs;
        case 4: return HandType::OnePair;
        case 5: return HandType::HighCard;
        default: Assert(false);
    }
    return HandType::None;
}

s32 HandComparatorPartOne(const void* p1, const void* p2)
{
    // Lookup table for the strength of different cards.
    char strength_lut[85];
    strength_lut['2'] = 1;
    strength_lut['3'] = 2;
    strength_lut['4'] = 3;
    strength_lut['5'] = 4;
    strength_lut['6'] = 5;
    strength_lut['7'] = 6;
    strength_lut['8'] = 7;
    strength_lut['9'] = 8;
    strength_lut['T'] = 9;
    strength_lut['J'] = 10;
    strength_lut['Q'] = 11;
    strength_lut['K'] = 12;
    strength_lut['A'] = 13;

    Hand* h1 = (Hand*)p1;
    Hand* h2 = (Hand*)p2;

    // If the types differ, we can just compare them directly.
    s32 score = (s32)h1->type - (s32)h2->type;
    if (score) return score;

    // If not, we need to compare the score of each hand, lexicographically.
    for (s32 i = 0; i < 5; ++i)
    {
        score =  strength_lut[h1->cards[i]] - strength_lut[h2->cards[i]];
        if (score) return score;
    }

    return score;
}

HandType GetHandTypePartTwo(Span<char> input, Hand hand, TArray<char>& card_buckets)
{
    // Same as part one, create buckets and count the cards in each bucket.
    s32 bucket_counts[5] = {};
    card_buckets.SetLength(0);

    s32 joker_bucket = -1; // Will be >= 0 if one of the buckets contains jokers.
    for (s32 card_idx = 0; card_idx < 5; ++card_idx)
    {
        char card = hand.cards[card_idx];
        s32 bucket_idx = card_buckets.IndexOf(card);
        if (bucket_idx >= 0) bucket_counts[bucket_idx] += 1;
        else
        {
            if (card == 'J') joker_bucket = card_buckets.Length();
            bucket_counts[card_buckets.Length()] = 1;
            card_buckets.Append(card);
        }
    }

    // Same as part one, find the biggest bucket, and determine an initial hand type.
    s32 max_bucket_size = 0;
    for (s32 i = 0; i < card_buckets.Length(); ++i) if (bucket_counts[i] > max_bucket_size) max_bucket_size = bucket_counts[i];

    HandType type = HandType::None;
    switch (card_buckets.Length())
    {
        case 1: type = HandType::FiveOfAKind; break;
        case 2: type = (max_bucket_size == 4) ? HandType::FourOfAKind : HandType::FullHouse; break;
        case 3: type = (max_bucket_size == 3) ? HandType::ThreeOfAKind : HandType::TwoPairs; break;
        case 4: type = HandType::OnePair; break;
        case 5: type = HandType::HighCard; break;
        default: Assert(false); break;
    }

    // If this hand contained any jokers, we can modify it based on the number of jokers encountered.
    if (joker_bucket >= 0)
    {
        s32 joker_count = bucket_counts[joker_bucket];

        switch (type)
        {
            case HandType::HighCard: type = HandType::OnePair; break;
            case HandType::OnePair: type = HandType::ThreeOfAKind; break;
            case HandType::TwoPairs: type = (joker_count == 2) ? HandType::FourOfAKind : HandType::FullHouse; break;
            case HandType::ThreeOfAKind: type = HandType::FourOfAKind; break;
            case HandType::FullHouse: type = HandType::FiveOfAKind; break;
            case HandType::FourOfAKind: type = HandType::FiveOfAKind; break;
            case HandType::FiveOfAKind: break;
            default: Assert(false); break;
        }
    }
    return type;
}

s32 HandComparatorPartTwo(const void* p1, const void* p2)
{
    // Same lookup table as in part one, except the joker has the lowest strength.
    char strength_lut[85];
    strength_lut['J'] = 1;
    strength_lut['2'] = 2;
    strength_lut['3'] = 3;
    strength_lut['4'] = 4;
    strength_lut['5'] = 5;
    strength_lut['6'] = 6;
    strength_lut['7'] = 7;
    strength_lut['8'] = 8;
    strength_lut['9'] = 9;
    strength_lut['T'] = 10;
    strength_lut['Q'] = 11;
    strength_lut['K'] = 12;
    strength_lut['A'] = 13;

    Hand* h1 = (Hand*)p1;
    Hand* h2 = (Hand*)p2;

    s32 score = (s32)h1->type - (s32)h2->type;
    if (score) return score;

    for (s32 i = 0; i < 5; ++i)
    {
        score =  strength_lut[h1->cards[i]] - strength_lut[h2->cards[i]];
        if (score) return score;
    }

    return score;
}

static s64 DoPartOne(Span<char> input)
{
    // General strategy:
    // 1. Iterate through lines to parse each hand (hand string, bid, and type).
    // 2. Sort the hands in-place by strength.
    // 3. Compute the total score.
    TArray<Hand> hands = {};
    s32 offset = 0;
    while (offset < input.count)
    {
        Hand hand = {};
        hand.cards = &input[offset];
        input[offset + 5] = '\0';
        offset += 6; // Iterate past the hand (5 chars and then a space).
        hand.bid = ParseNumber(input.ptr, &offset);
        offset += 1; // Iterate past the newline.
        hands.Append(hand);
    }

    TArray<char> buckets = TArray<char>();
    for (Hand& hand : hands) hand.type = GetHandTypePartOne(input, hand, buckets);

    qsort((void*)&hands[0], hands.Length(), sizeof(Hand), HandComparatorPartOne);

    s64 total_score = 0;
    for (s32 i = 0; i < hands.Length(); ++i) total_score += (hands[i].bid * (i + 1));
    return total_score;
}

static s64 DoPartTwo(Span<char> input)
{
    // General strategy: Same as part one, but with slightly different rules for hand types
    // and strengths.
    TArray<Hand> hands = {};

    s32 offset = 0;
    while (offset < input.count)
    {
        Hand hand = {};
        hand.cards = &input[offset];
        input[offset + 5] = '\0';
        offset += 6; // Iterate past the hand (5 chars and then a space).
        hand.bid = ParseNumber(input.ptr, &offset);
        offset += 1; // Iterate past the newline.
        hands.Append(hand);
    }

    TArray<char> buckets = TArray<char>();
    for (Hand& hand : hands) hand.type = GetHandTypePartTwo(input, hand, buckets);

    qsort((void*)&hands[0], hands.Length(), sizeof(Hand), HandComparatorPartTwo);

    s64 total_score = 0;
    for (s32 i = 0; i < hands.Length(); ++i) total_score += (hands[i].bid * (i + 1));
    return total_score;
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


