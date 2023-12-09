#include "day09.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

namespace day09
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 09 " << (example ? "(example)" : "") << '\n';

        std::string fileName{ example ? "inputs/day09_example.txt" : "inputs/day09_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    using numberList = std::vector<int>;

    std::vector<numberList> parseInput(std::ifstream& file)
    {
        std::vector<numberList> lines{};
        std::string line;
        while(!file.eof())
        {
            std::getline(file, line);
            lines.push_back(parseLineOfNumbers(line));
        }

        return lines;
    }

    numberList getDiffs(numberList sequence)
    {
        numberList diffs{};
        for(size_t i = 0; i < sequence.size() - 1; i++)
        {
            diffs.push_back(sequence[i + 1] - sequence[i]);
        }

        return diffs;
    }


    long long solvePart1(std::ifstream& file)
    {
        const auto input{ parseInput(file) };
        long long totalSum{};

        // For running back, use the fact that the increase to the next in the original sequence
        // is equal to the sum of the last element of all diff sequences, so don't need to remember those
        // sequences, just sum their last values.
        for (auto sequence : input)
        {
            long long finalElementOfDiffsSum{};
            numberList diffs{ getDiffs(sequence) };
            while (std::ranges::any_of(diffs, [](const int i) {return i != 0; }))
            {
                finalElementOfDiffsSum += diffs[diffs.size() - 1];
                diffs = getDiffs(diffs);
            }

            totalSum += sequence[sequence.size() - 1] + finalElementOfDiffsSum;
        }

        return totalSum;
    }

    long long solvePart2(std::ifstream& file)
    {
        const auto input{ parseInput(file) };
        long long totalSum{};

        // Basically the same as part 1, except we need to keep track of value of first elements.
        // Since we substract each layer, we need to alternate +/- to our sum.
        for (auto sequence : input)
        {
            long long firstElementOfDiffsSum{};
            numberList diffs{ getDiffs(sequence) };
            bool plus{ true };
            while (std::ranges::any_of(diffs, [](const int i) {return i != 0; }))
            {
                firstElementOfDiffsSum += plus ? diffs[0] : -diffs[0];
                diffs = getDiffs(diffs);
                plus = !plus;
            }

            totalSum += sequence[0] - firstElementOfDiffsSum;
        }

        return totalSum;
    }
}