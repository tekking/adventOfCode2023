#include "day12.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <numeric>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace day12
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 12 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day12_example.txt" : "inputs/day12_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    // struct conditionRecord
    // {
    //     const std::string_view conditions{};
    //     const std::vector<int> groups{};
    //     const bool hasToStartDamaged{};
    //     const bool hasToStartOperational{};
    //     const size_t mininumRemainingSpacesNeeded{};
    //
    //     // Record guaranteed to be invalid if we reach end of conditions, but have groups remaining
    //     // or if we start with '#' but have no groups remaning
    //     // or we are in a active group (startsInGroup), but first char is '.'.
    //     [[nodiscard]] bool isInvalid() const
    //     {
    //         if (conditions.size() == 0)
    //         {
    //             // Valid if no groups, invalid if any groups left.
    //             return !groups.empty();
    //         }
    //
    //         if (conditions[0] == '#' && groups.size() == 0)
    //         {
    //             return true;
    //         }
    //
    //         if (hasToStartDamaged && conditions[0] == '.')
    //         {
    //             return true;
    //         }
    //
    //         if (hasToStartOperational && conditions[0] == '#')
    //         {
    //             return true;
    //         }
    //
    //         if (mininumRemainingSpacesNeeded > conditions.size())
    //         {
    //             return true;
    //         }
    //
    //         // No guarantee to be valid, but iterate on it for now.
    //         return false;
    //     }
    //
    //     [[nodiscard]] bool startsStatic() const
    //     {
    //         return conditions[0] != '?';
    //     }
    //
    //     [[nodiscard]] bool canStartOperational() const
    //     {
    //         return !hasToStartDamaged;
    //     }
    //
    //     [[nodiscard]] bool canStartDamaged() const
    //     {
    //         return !hasToStartOperational && !groups.empty();
    //     }
    //
    //     [[nodiscard]] conditionRecord removeDamagedSpot() const
    //     {
    //         if (groups[0] == 1)
    //         {
    //             return conditionRecord{
    //                 conditions.substr(1, conditions.size() - 1),
    //                 std::vector(groups.begin() + 1, groups.end()),
    //                 false,
    //                 true,
    //                 mininumRemainingSpacesNeeded > 1 ? mininumRemainingSpacesNeeded - 2 : 0 // It's only -1 on the last group
    //             };
    //         }
    //
    //         std::vector newGroups(groups);
    //         newGroups[0]--;
    //         return conditionRecord{
    //             conditions.substr(1, conditions.size() - 1),
    //             newGroups,
    //             true,
    //             false,
    //             mininumRemainingSpacesNeeded - 1
    //         };
    //     }
    //
    //     [[nodiscard]] conditionRecord removeOperationalSpot() const
    //     {
    //         return conditionRecord{
    //             conditions.substr(1, conditions.size() - 1),
    //             groups,
    //             false,
    //             false,
    //             mininumRemainingSpacesNeeded
    //         };
    //     }
    //
    //     [[nodiscard]] conditionRecord removeStaticFirst() const
    //     {
    //         switch (conditions[0])
    //         {
    //             case '.':
    //                 return removeOperationalSpot();
    //
    //             case '#':
    //                 return removeDamagedSpot();
    //
    //             default:
    //                 assert(false);
    //                 return conditionRecord{};
    //         }
    //     }
    // };
    //
    // // Approach is recursive over remainder of spots, starting at front.
    // // Splits on any '?' spot in the spots.
    // long long determineNumberOfOptions(const conditionRecord& record)
    // {
    //     if (record.isInvalid())
    //     {
    //         return 0;
    //     }
    //
    //     if (record.conditions.empty())
    //     {
    //         return 1;
    //     }
    //
    //     // If the current first character of the remaining spots is known (static), then
    //     // number is just the same as number for the remainder of spots (updating groups/ingroup status etc)
    //     if (record.startsStatic())
    //     {
    //         return determineNumberOfOptions(record.removeStaticFirst());
    //     }
    //
    //     // Otherwise it's the sum of possibilities on both options:
    //     const long long numberAfterOperation = record.canStartOperational()
    //         ? determineNumberOfOptions(record.removeOperationalSpot())
    //         : 0;
    //
    //     // Damaged only a possibility if there are any groups left.
    //     const long long numberAfterDamaged = record.canStartDamaged()
    //         ? determineNumberOfOptions(record.removeDamagedSpot())
    //         : 0;
    //
    //     return numberAfterOperation + numberAfterDamaged;
    // }
    //
    // conditionRecord parseInputLine(std::string_view line)
    // {
    //     const size_t spacePos = line.find(' ');
    //     const std::string_view conditions{ line.substr(0, spacePos) };
    //     const std::string_view groupsString{ line.substr(spacePos + 1, line.size() - spacePos - 1) };
    //     const std::vector<int> groups = parseLineOfSymbolSeperatedNumbers(groupsString);
    //
    //     const size_t minSpacesNeeded{ std::accumulate(groups.begin(), groups.end(), 0) + groups.size() - 1 };
    //     conditionRecord record{ conditions, groups, false, false, minSpacesNeeded };
    //
    //     return record;
    // }
    //
    // conditionRecord parseInputLinePart2(std::string_view line)
    // {
    //     const size_t spacePos = line.find(' ');
    //     const std::string_view singleSetOfConditions{ line.substr(0, spacePos) };
    //     const std::string_view groupsString{ line.substr(spacePos + 1, line.size() - spacePos - 1) };
    //     const auto singleSetOfGroups = parseLineOfSymbolSeperatedNumbers(groupsString);
    //
    //     // ehhh, i guess safe since we finish a line before we start on the next
    //     static std::string conditions;
    //     conditions = "";
    //     std::vector<int> groups{};
    //
    //     for (int i =0; i < 5; i++)
    //     {
    //         conditions.append(singleSetOfConditions);
    //         if (i < 4)
    //         {
    //             conditions.append("?");
    //         }
    //
    //         for (int group : singleSetOfGroups)
    //         {
    //             groups.push_back(group);
    //         }
    //     }
    //
    //     const size_t minSpacesNeeded{ std::accumulate(groups.begin(), groups.end(), 0) + groups.size() - 1 };
    //     return conditionRecord{ conditions, groups, false, false, minSpacesNeeded };
    // }
    
    struct Puzzle
    {
        std::string conditions{};
        std::string requiredConditions{};
    };
    
    struct PuzzleView
    {
        std::string_view conditions{};
        std::string_view requiredConditions{};
    };

    Puzzle parseInputToPuzzle(std::string_view line)
    {
        const size_t spacePos = line.find(' ');
        const std::string conditions{ line.substr(0, spacePos) };

        const std::string_view groupsString{ line.substr(spacePos + 1, line.size() - spacePos - 1) };
        const std::vector<int> groups = parseLineOfSymbolSeperatedNumbers(groupsString);

        // Represent groups as symbols that 'must' be in condition string in some subsequence.
        std::string groupsAsSymbolString{};
        for (const int group : groups)
        {
            groupsAsSymbolString.append(std::string(static_cast<size_t>(group), '#'));
            groupsAsSymbolString.push_back('.');
        }

        // Don't need a . after last group.
        groupsAsSymbolString.pop_back();


        return Puzzle{conditions, groupsAsSymbolString};
    }

    Puzzle parseInputToPuzzlePartTwo(std::string_view line)
    {
        const size_t spacePos = line.find(' ');
        std::string conditions{ line.substr(0, spacePos) };

        // Append extra copies:
        for (int i= 0; i< 4; i++)
        {
            conditions.push_back('?');
            conditions.append(line.substr(0, spacePos));
        }

        const std::string_view groupsString{ line.substr(spacePos + 1, line.size() - spacePos - 1) };
        const std::vector<int> groups = parseLineOfSymbolSeperatedNumbers(groupsString);

        // Represent groups as symbols that 'must' be in condition string in some subsequence.
        std::string groupsAsSymbolString{};

        // Loop for the times 5
        for(int i =0; i<5; i++)
        {
            for (const int group : groups)
            {
                groupsAsSymbolString.append(std::string(static_cast<size_t>(group), '#'));
                groupsAsSymbolString.push_back('.');
            }
        }

        // Don't need a . after last group.
        groupsAsSymbolString.pop_back();


        return Puzzle{ conditions, groupsAsSymbolString };
    }

    long long determineOptionsForPuzzle(const PuzzleView puzzle, const bool inOngoingGroup)
    {
        if (puzzle.requiredConditions.empty())
        {
            // Can determine if we're valid for the rest of the puzzle, any ? must be . at this point to be valid.
            if (std::ranges::any_of(puzzle.conditions, [](char c){return c == '#';}))
            {
                // Can't have '#' in the remainder if no more required
                return 0;
            }

            // Else only possibility is all '.'
            return 1;
        }

        if (puzzle.requiredConditions.size() > puzzle.conditions.size())
        {
            // Can't fill out required remainder anymore, no possibilities
            return 0;
        }

        // At this point we know both are non-empty
        if (puzzle.requiredConditions[0] == '.')
        {
            if (puzzle.conditions[0] == '#')
            {
                // Failed if we need a seperator '.', but we're starting with #
                return 0;
            }

            // Can continue by either forcing a '?' to be '.' or just passing static '.'
            return determineOptionsForPuzzle(PuzzleView{
                                                 puzzle.conditions.substr(1, puzzle.conditions.size() - 1),
                                                 puzzle.requiredConditions.substr(1, puzzle.requiredConditions.size() - 1)
                                             },
                                             false);
        }

        // If we're currently looking for '#' we can also skip '.' if we're not in an ongoing group
        if (inOngoingGroup)
        {
            // Next condition must be '#'
            if (puzzle.conditions[0] == '.')
            {
                return 0;
            }

            // Force '?' to be '#' or just pass a '#'
            const bool stillOngoing = puzzle.requiredConditions.size() > 1 && puzzle.requiredConditions[1] == '#';
            return determineOptionsForPuzzle(PuzzleView{
                                                 puzzle.conditions.substr(1, puzzle.conditions.size() - 1),
                                                 puzzle.requiredConditions.substr(1, puzzle.requiredConditions.size() - 1)
                },
                stillOngoing);
        }

        // If not we can either start a group with '#' or just 'do nothing' with '.':
        if (puzzle.conditions[0] == '.')
        {
            return determineOptionsForPuzzle(PuzzleView{
                                                    puzzle.conditions.substr(1, puzzle.conditions.size() - 1),
                                                    puzzle.requiredConditions
                },
                false);
        }

        if (puzzle.conditions[0] == '#')
        {
            const bool ongoingGroupAfter = puzzle.requiredConditions.size() > 1 && puzzle.requiredConditions[1] == '#';
            return determineOptionsForPuzzle(PuzzleView{
                                                 puzzle.conditions.substr(1, puzzle.conditions.size() - 1),
                                                 puzzle.requiredConditions.substr(1, puzzle.requiredConditions.size() - 1)
                },
                ongoingGroupAfter);
        }

        // Finally if we're at a '?' we can choose:
        const bool ongoingGroupAfter = puzzle.requiredConditions.size() > 1 && puzzle.requiredConditions[1] == '#';
        const long long optionsAfterDamaged{ determineOptionsForPuzzle(PuzzleView{
                                                                           puzzle.conditions.substr(1, puzzle.conditions.size() - 1),
                                                                           puzzle.requiredConditions.substr(1, puzzle.requiredConditions.size() - 1)
                                                                       },
                                                                       ongoingGroupAfter)
        };
        const long long optionsAfterOperational{ determineOptionsForPuzzle(PuzzleView{
                                                                               puzzle.conditions.substr(1, puzzle.conditions.size() - 1),
                                                                               puzzle.requiredConditions
                                                                           },
                                                                           false)
        };

        return optionsAfterDamaged + optionsAfterOperational;
    }

    long long solvePart1(std::ifstream& file)
    {
        long long sum{};

        while(!file.eof())
        {
            std::string line;
            std::getline(file, line);

            // auto record = parseInputLine(line);
            // sum += determineNumberOfOptions(record);

            const auto puzzle = parseInputToPuzzle(line);
            const PuzzleView view{ puzzle.conditions, puzzle.requiredConditions };
            sum += determineOptionsForPuzzle(view, false);
        }

        return sum;
    }

    long long solvePart2(std::ifstream& file)
    {
        long long sum{};

        int nr{  };
        while (!file.eof())
        {
            std::string line;
            std::getline(file, line);

            // auto record = parseInputLinePart2(line);
            // sum += determineNumberOfOptions(record);

            const auto puzzle = parseInputToPuzzlePartTwo(line);
            const PuzzleView view{ puzzle.conditions, puzzle.requiredConditions };
            sum += determineOptionsForPuzzle(view, false);

            nr++;
            std::cout << nr << '\n';
        }

        return sum;
    }
}