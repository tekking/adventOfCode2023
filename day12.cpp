#include "day12.h"
#include "helpers.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>
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

    struct State
    {
        bool inOngoingGroup{};
        size_t conditionIndex{};
        size_t requiredIndex{};

        [[nodiscard]] bool operator==(const State& other) const
        {
            return inOngoingGroup == other.inOngoingGroup && conditionIndex == other.conditionIndex && requiredIndex == other.requiredIndex;
        }
    };

    struct StateHasher
    {
        std::size_t operator()(const State& p) const
        {
            // bad hash, but relying on inputs being fairly short,  < 100
            return p.conditionIndex + p.requiredIndex * 10 + (p.inOngoingGroup ? 10000 : 0);
        }
    };
    
    struct Puzzle
    {
        std::string conditions{};
        std::string requiredConditions{};

        std::unordered_map<State, long long, StateHasher> solvedLookup{};

        long long determineOptionsForPuzzle(const State& curState)
        {
            if (solvedLookup.contains(curState))
            {
                return solvedLookup[curState];
            }

            if (requiredConditions.size() == curState.requiredIndex)
            {
                // Can determine if we're valid for the rest of the puzzle, any ? must be . at this point to be valid.
                bool failed{};
                for (size_t i = curState.conditionIndex; i < conditions.size(); i++)
                {
                    if (conditions[i] == '#')
                    {
                        failed = true;
                    }
                }

                if (!failed)
                {
                    solvedLookup[curState] = 1;
                    return 1;
                }

                solvedLookup[curState] = 0;
                return 0;
            }

            if ((requiredConditions.size() - curState.requiredIndex) > (conditions.size() - curState.conditionIndex))
            {
                // Can't fill out required remainder anymore, no possibilities
                solvedLookup[curState] = 0;
                return 0;
            }

            // At this point we know both are non-empty
            if (requiredConditions[curState.requiredIndex] == '.')
            {
                if (conditions[curState.conditionIndex] == '#')
                {
                    // Failed if we need a seperator '.', but we're starting with #
                    solvedLookup[curState] = 0;
                    return 0;
                }

                const auto result{ determineOptionsForPuzzle(State{false, curState.conditionIndex + 1, curState.requiredIndex + 1}) };
                solvedLookup[curState] = result;
                return result;
            }

            // If we're currently looking for '#' we can also skip '.' if we're not in an ongoing group
            if (curState.inOngoingGroup)
            {
                // Next condition must be '#'
                if (conditions[curState.conditionIndex] == '.')
                {
                    solvedLookup[curState] = 0;
                    return 0;
                }

                // Force '?' to be '#' or just pass a '#'
                const bool stillOngoing = requiredConditions.size() > (curState.requiredIndex + 1) && requiredConditions[curState.requiredIndex + 1] == '#';
                const auto result{ determineOptionsForPuzzle(State{stillOngoing, curState.conditionIndex + 1, curState.requiredIndex + 1}) };
                solvedLookup[curState] = result;
                return result;
            }

            // If not we can either start a group with '#' or just 'do nothing' with '.':
            if (conditions[curState.conditionIndex] == '.')
            {
                const auto result{ determineOptionsForPuzzle(State{false, curState.conditionIndex + 1, curState.requiredIndex}) };
                solvedLookup[curState] = result;
                return result;
            }

            if (conditions[curState.conditionIndex] == '#')
            {
                const bool ongoingGroupAfter = requiredConditions.size() > (curState.requiredIndex + 1) && requiredConditions[curState.requiredIndex + 1] == '#';
                const auto result{ determineOptionsForPuzzle(State{ongoingGroupAfter, curState.conditionIndex + 1, curState.requiredIndex + 1}) };
                solvedLookup[curState] = result;
                return result;
            }

            // Finally if we're at a '?' we can choose:
            const bool ongoingGroupAfter = requiredConditions.size() > (curState.requiredIndex + 1) && requiredConditions[curState.requiredIndex + 1] == '#';
            const auto result{ determineOptionsForPuzzle(State{ongoingGroupAfter, curState.conditionIndex + 1, curState.requiredIndex + 1})
                + determineOptionsForPuzzle(State{false, curState.conditionIndex + 1, curState.requiredIndex}) };
            solvedLookup[curState] = result;
            return result;
        }
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

    long long solvePart1(std::ifstream& file)
    {
        long long sum{};

        while(!file.eof())
        {
            std::string line;
            std::getline(file, line);

            auto puzzle = parseInputToPuzzle(line);
            sum += puzzle.determineOptionsForPuzzle(State{ false, 0, 0 });
        }

        return sum;
    }

    long long solvePart2(std::ifstream& file)
    {
        long long sum{};

        while (!file.eof())
        {
            std::string line;
            std::getline(file, line);

            auto puzzle = parseInputToPuzzlePartTwo(line);
            sum += puzzle.determineOptionsForPuzzle(State{ false, 0, 0 });
        }

        return sum;
    }
}