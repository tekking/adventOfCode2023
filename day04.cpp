#include "day04.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <numeric>
#include <string>
#include <sstream>
#include <vector>

namespace day04
{
    int solvePart1(std::ifstream& file);
    int solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 04 \n";

        const std::string fileName{ example ? "inputs/day04_example.txt" : "inputs/day04_real.txt" };
        std::ifstream file{ fileName };

        int sum{ solvePart1(file) };

        std::cout << "Part 1 answer: " << sum << '\n';

        file.close();
        file.open(fileName);
        sum = solvePart2(file);

        std::cout << "Part 2 answer: " << sum << '\n';
    }

    std::vector<std::pair<std::vector<bool>, std::vector<int>>> parseInput(std::ifstream& file)
    {
        // Input parsing uses fact that nrs are all <100 to create 100-long array to hold bools marking
        // winning numbers.

        // For nr of numbers per game, look at first line
        std::string line;
        std::getline(file, line);
        const size_t nrOfNumbers{ (line.size() - line.find('|') - 1) / 3 };

        // For nr of lines loop over lines and count:
        size_t nrOfLines{1};
        while (std::getline(file, line))
        {
            nrOfLines++;
        }

        // Create outer vector
        std::vector<std::pair<std::vector<bool>, std::vector<int>>> input(nrOfLines);

        // Reset file stream
        file.clear();
        file.seekg(0);

        // Read in each line of file
        unsigned int currentLineIndex{};
        while (std::getline(file, line))
        {
            std::vector<bool> winningForLine(100, false);
            std::vector<int> numbersForLine(nrOfNumbers);
            std::stringstream lineStream;
            std::string placeHolder{};
            lineStream << line;

            // Could also be constant removal of start of string, but let's stick to stream manipulation
            lineStream >> placeHolder >> placeHolder;

            unsigned int nextNr;
            lineStream >> nextNr; // Resharper seems to think this is invalid...
            while (!lineStream.fail())
            {
                winningForLine[nextNr] = true;
                lineStream >> nextNr;
            }

            lineStream.clear();

            unsigned int index{ 0 };
            lineStream >> placeHolder;
            lineStream >> nextNr;
            while (!lineStream.fail())
            {
                numbersForLine[index] = static_cast<int>(nextNr);
                lineStream >> nextNr;
                index++;
            }

            const std::pair linePair{ winningForLine, numbersForLine };
            input[currentLineIndex] = linePair;
            currentLineIndex++;
        }

        return input;
    }

    int solvePart1(std::ifstream& file)
    {
        const auto input = parseInput(file);

        int sum{ 0 };
        for (auto& line : input)
        {
            // Count nr of wins on line.
            int lineWinning{ 0 };
            for (int number : line.second)
                if (line.first[static_cast<unsigned int>(number)])
                    lineWinning++;

            // Calculate line score (=2^(n-1)).
            if (lineWinning > 0)
                sum += static_cast<int>(std::pow(2, lineWinning - 1));
        }

        return sum;
    }

    int solvePart2(std::ifstream& file)
    {
        const auto input = parseInput(file);

        // Start out with 1 of each scratchCard.
        std::vector<int> nrOfEachCard(input.size(), 1);
        unsigned int lineIndex{ 0 };
        for (auto& line : input)
        {
            int lineWinning{ 0 };

            for (const int number : line.second)
                if (line.first[static_cast<unsigned int>(number)])
                    lineWinning++;

            // Up all upcoming cards based on nr of wins this line, each goes up by nr of current line.
            for (int i = 1; i <= lineWinning; i++)
            {
                if (lineIndex + i >= input.size())
                    // Going past last card
                    break;

                nrOfEachCard[lineIndex + i] += nrOfEachCard[lineIndex];
            }

            lineIndex++;
        }

        // Return sum of count of each card.
        return std::accumulate(nrOfEachCard.begin(), nrOfEachCard.end(), 0);
    }
}