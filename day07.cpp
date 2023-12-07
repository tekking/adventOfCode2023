#include "day07.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace day07
{
    int solvePart1(std::ifstream& file);
    int solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 07 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day07_example.txt" : "inputs/day07_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct hand
    {
        std::string handDescription;
        int bid;
    };

    // Higher strength is better, 0 lowest, 12 highest.
    size_t symbolToStrengthPartOne(char symbol)
    {
        if (std::isdigit(symbol))
        {
            // 2 -> 0 etc for digits.
            return symbol - '0' - 2u;
        }

        switch (symbol)
        {
            case 'T':
                return 8;

            case 'J':
                return 9;

            case 'Q':
                return 10;

            case 'K':
                return 11;

            default:
                return 12;
        }
    }

    // Returns the strength of a hand (ignoring tiebreaker)
    // 0 = high card, 6 = five of a kind etc.
    int handToStrengthPartOne(const hand& hand)
    {
        assert(hand.handDescription.size() == 5);

        // Get the count of each label in hand
        std::vector<int> countPerSymbol(13, 0);
        for(size_t i = 0; i<5; i++)
        {
            countPerSymbol[symbolToStrengthPartOne(hand.handDescription[i])]++;
        }

        // Can check for all possible hands while only looping over counts once.
        // But need to keep track if we already saw 2/3 of.
        bool foundTwoOf{ false };
        bool foundThreeof{ false };

        for(size_t i =0; i < 13; i++)
        {
            // Easy cases that guarantee final answer first:
            if (countPerSymbol[i] == 5)
            {
                // Five of a kind
                return 6;
            }

            if (countPerSymbol[i] == 4)
            {
                // Four of a kind
                return 5;
            }

            // These cases are conditional on things we already found.
            if (countPerSymbol[i] == 3)
            {
                if (foundTwoOf)
                {
                    // Full house
                    return 4;
                }

                foundThreeof = true;
            }
            else if (countPerSymbol[i] == 2)
            {
                if (foundThreeof)
                {
                    // Full house
                    return 4;
                }

                if (foundTwoOf)
                {
                    // Two pairs
                    return 2;
                }

                foundTwoOf = true;
            }
        }

        // It's possible we haven't found a final hand score yet:
        if (foundThreeof)
        {
            // Three of a kind
            return 3;
        }

        if (foundTwoOf)
        {
            // One pair
            return 1;
        }

        // High card
        return 0;
    }

    bool compareHandsPartOne(const hand& handOne, const hand& handTwo)
    {
        const int handOneStrength{ handToStrengthPartOne(handOne) };
        const int handTwoStrength{ handToStrengthPartOne(handTwo) };

        // Check if hand strength (type of hand) is good enough
        if (handOneStrength != handTwoStrength)
        {
            return handOneStrength < handTwoStrength;
        }

        // Determine by card strengths
        for (size_t i = 0; i<5; i++)
        {
            if (handOne.handDescription[i] != handTwo.handDescription[i])
            {
                return symbolToStrengthPartOne(handOne.handDescription[i])
                    < symbolToStrengthPartOne(handTwo.handDescription[i]);
            }
        }

        // Exact equal hands
        return false;
    }

    // Higher strength is better, 0 lowest, 12 highest.
    size_t symbolToStrengthPartTwo(char symbol)
    {
        if (std::isdigit(symbol))
        {
            // 2 -> 1 etc for digits.
            return symbol - '0' - 1u;
        }

        switch (symbol)
        {
        case 'J':
            return 0;

        case 'T':
            return 9;

        case 'Q':
            return 10;

        case 'K':
            return 11;

        default:
            return 12;
        }
    }

    // Returns the strength of a hand (ignoring tiebreaker)
    // 0 = high card, 6 = five of a kind etc.
    int handToStrengthPartTwo(const hand& hand)
    {
        assert(hand.handDescription.size() == 5);

        // Get the count of each label in hand
        std::vector<int> countPerSymbol(13, 0);
        for (size_t i = 0; i < 5; i++)
        {
            countPerSymbol[symbolToStrengthPartTwo(hand.handDescription[i])]++;
        }

        // Now with joker it's easier to just check cases in order of strength, instead of during single loop
        int jokerCount{ countPerSymbol[0] };
        std::vector<int> nonJokerCounts{ countPerSymbol.begin() + 1, countPerSymbol.end() };

        if ((jokerCount == 5) || std::ranges::any_of(nonJokerCounts, [jokerCount](int count){return count == (5-jokerCount);}))
        {
            // Five of a kind
            return 6;
        }

        // 4 jokers is impossible, since that creates a five of a kind with remaining card
        if (std::ranges::any_of(nonJokerCounts, [jokerCount](int count) {return count == (4 - jokerCount); }))
        {
            assert(jokerCount < 4);

            // Four of a kind
            return 5;
        }

        // 3 jokers is impossible, since that creates a four of a kind
        if (std::ranges::any_of(nonJokerCounts, [jokerCount](int count) {return count == (3 - jokerCount); }))
        {
            assert(jokerCount < 3);

            // Two cases, either full house or three of a kind, check by possible joker counts:
            if (jokerCount == 2)
            {
                // Three of a kind, since 2 jokers in full house would allow a 4/5 of kind
                return 3;
            }

            if (jokerCount == 1)
            {
                // Note there cannot be 3 natural of any card, since that would mean a 4 of kind with joker, so check if there are two pairs (outside joker):
                if (std::ranges::count_if(nonJokerCounts, [](int count){return count == 2;}) == 2)
                {
                    // Full house (with the joker on one of pairs).
                    return 4;
                }

                return 3;
            }

            // Joker count == 0, so already know 3 count exists:
            if (std::ranges::any_of(nonJokerCounts, [](int count){return count ==2;}))
            {
                // Full house
                return 4;
            }

            // Three of a kind
            return 3;
        }

        // 2 jokers is impossible, since that creates a three of a kind
        if (std::ranges::any_of(nonJokerCounts, [jokerCount](int count) {return count == (2 - jokerCount); }))
        {
            assert(jokerCount < 2);

            if (jokerCount == 1)
            {
                // Either two pair or single pair, two pair if there is a natural (without joker) 2 count:
                if (std::ranges::any_of(nonJokerCounts, [](int count) {return count == 2; }))
                {
                    // Two pairs
                    return 2;
                }

                // One pair
                return 1;
            }

            if (std::ranges::count_if(nonJokerCounts, [](int count) {return count == 2; }) == 2)
            {
                // Two pairs
                return 2;
            }

            // One pair
            return 1;
        }

        // High card
        return 0;
    }

    bool compareHandsPartTwo(const hand& handOne, const hand& handTwo)
    {
        const int handOneStrength{ handToStrengthPartTwo(handOne) };
        const int handTwoStrength{ handToStrengthPartTwo(handTwo) };

        // Check if hand strength (type of hand) is good enough
        if (handOneStrength != handTwoStrength)
        {
            return handOneStrength < handTwoStrength;
        }

        // Determine by card strengths
        for (size_t i = 0; i < 5; i++)
        {
            if (handOne.handDescription[i] != handTwo.handDescription[i])
            {
                return symbolToStrengthPartTwo(handOne.handDescription[i])
                    < symbolToStrengthPartTwo(handTwo.handDescription[i]);
            }
        }

        // Exact equal hands
        return false;
    }

    std::vector<hand> parseInputToHands(std::ifstream& file)
    {
        std::string line;
        std::vector<hand> hands{};

        while(!file.eof())
        {
            std::getline(file, line);
            std::string handDesc{ line.substr(0, 5) };
            std::string bid{ line.substr(6, line.size() - 6) };
            hands.push_back(hand{ handDesc, stoi(bid) });
        }

        return hands;
    }

    int solvePart1(std::ifstream& file)
    {
        // Parse input
        std::vector<hand> hands{ parseInputToHands(file) };

        // Sort by (ascending) score
        std::ranges::sort(hands, compareHandsPartOne);

        // Calculate sum
        int sum{};
        for (size_t i = 0; i < hands.size(); i++)
        {
            sum += (static_cast<int>(i) + 1) * hands[i].bid;
        }

        return sum;
    }

    int solvePart2(std::ifstream& file)
    {
        // Parse input
        std::vector<hand> hands{ parseInputToHands(file) };

        // Sort by (ascending) score
        std::ranges::sort(hands, compareHandsPartTwo);

        // Calculate sum
        int sum{};
        for (size_t i = 0; i < hands.size(); i++)
        {
            sum += (static_cast<int>(i) + 1) * hands[i].bid;
        }

        return sum;
    }
}