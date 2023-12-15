#include "day03.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace day03
{
    int solvePart1(std::ifstream& file);
    int solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 03 \n";

        const std::string fileName{ example ? "inputs/day03_example.txt" : "inputs/day03_real.txt" };
        std::ifstream file{ fileName };

        int sum{ solvePart1(file) };

        std::cout << "Part 1 answer: " << sum << '\n';

        file.close();
        file.open(fileName);
        sum = solvePart2(file);
        
        std::cout << "Part 2 answer: " << sum << '\n';
    }

    std::vector<std::vector<char>> parseInput(std::ifstream& file)
    {
        // For height loop over lines and count:
        size_t height{};
        std::string line;
        while (std::getline(file, line))
        {
            height++;
        }

        // Create outer vector
        std::vector<std::vector<char>> field(height);

        // Reset file stream
        file.clear();
        file.seekg(0);

        // Read in each line of file
        size_t currentLineIndex{};
        while (std::getline(file, line))
        {
            const std::vector<char> fieldLine(line.begin(), line.end());
            field[currentLineIndex] = fieldLine;
            currentLineIndex++;
        }

        return field;
    }

    int solvePart1(std::ifstream& file)
    {
        const std::vector<std::vector<char>> field{ parseInput(file) };
        const int height{ static_cast<int>(field.size()) };
        const int width{ static_cast<int>(field[0].size()) };
        int partsSum{};

        // Loop over the lines detecting numbers:
        for (size_t y = 0; y < height; y ++)
        {
            size_t x{};

            // Keep going on line until we pass right-hand of field.
            while (x < width)
            {
                // Skip over non-digits
                while (x < width && !std::isdigit(field[y][x]))
                    x++;

                int partNumber{};
                // Collect full partNumber
                char digit;
                const size_t startOfNumber{ x };
                while (x < width && std::isdigit(digit = field[y][x]))
                {
                    partNumber *= 10;
                    partNumber += digit - '0';
                    x++;
                }

                const size_t endOfNumber{ x - 1 };

                // Now need to determine if number is partConnected
                bool partAdjacent{};
                for (size_t compareY = y - 1; compareY <= y + 1; compareY++)
                {
                    for (size_t compareX = startOfNumber - 1; compareX <= endOfNumber + 1; compareX++)
                    {
                        if (compareY > 0 && compareY < height && compareX > 0 && compareX < width
                            && !std::isdigit(field[compareY][compareX]) && field[compareY][compareX] != '.')
                            // Found an adjacent square in field with non-digit, non-. character
                            partAdjacent = true;
                    }
                }

                if (partAdjacent)
                    partsSum += partNumber;
            }
        }

        return partsSum;
    }

    int solvePart2(std::ifstream& file)
    {
        const std::vector<std::vector<char>> field{ parseInput(file) };
        const size_t height{ (field.size()) };
        const size_t width{ (field[0].size()) };

        // We create two new fields where we keep track of the nr of adjacent numbers if *
        // and the product of adjacent numbers.
        std::vector<std::vector<int>> adjCountField(height, std::vector<int>(width, 0));
        std::vector<std::vector<int>> adjProductField(height, std::vector<int>(width, 1));

        // Loop over the lines detecting numbers to fill out our adj fields:
        for (size_t y = 0; y < height; y++)
        {
            size_t x{};

            // Keep going on line until we pass right-hand of field.
            while (x < width)
            {
                // Skip over non-digits
                while (x < width && !std::isdigit(field[y][x]))
                    x++;

                int partNumber{};
                // Collect full partNumber
                char digit;
                const size_t startOfNumber{ x };
                while (x < width && std::isdigit(digit = field[y][x]))
                {
                    partNumber *= 10;
                    partNumber += digit - '0';
                    x++;
                }

                const size_t endOfNumber{ x - 1 };

                // Now check for adjacent gears to up our count/product field for
                for (size_t compareY = y - 1; compareY <= y + 1; compareY++)
                {
                    for (size_t compareX = startOfNumber - 1; compareX <= endOfNumber + 1; compareX++)
                    {
                        if (compareY > 0 && compareY < height && compareX > 0 && compareX < width
                            && field[compareY][compareX] == '*')
                        {
                            // Found an adjacent square in field with gear in it, up count & product.
                            adjCountField[compareY][compareX] += 1;
                            adjProductField[compareY][compareX] *= partNumber;
                        }
                    }
                }
            }
        }

        // Loop over the adj fields to determine the sum of faulty gears
        int faultySum{};
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                if (adjCountField[y][x] == 2)
                    faultySum += adjProductField[y][x];
            }
        }

        return faultySum;
    }
}