#include "day13.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

namespace day13
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 13 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day13_example.txt" : "inputs/day13_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct Field
    {
        std::vector<std::vector<bool>> field{};

        [[nodiscard]] bool horizontalReflectionIsValid(const size_t nrLeft) const
        {
            return horizontalReflectionIsSmudgedValid(nrLeft, 0);
        }

        [[nodiscard]] bool verticalReflectionIsValid(const size_t nrTop) const
        {
            return verticalReflectionIsSmudgedValid(nrTop, 0);
        }

        [[nodiscard]] bool horizontalReflectionIsSmudgedValid(const size_t nrLeft, int nrOfSmudges) const
        {
            const auto width = field[0].size();
            const auto height = field.size();

            const size_t nrToCompare = std::min(nrLeft, width - nrLeft);
            int diffCount{};

            for (size_t y = 0; y < height; y++)
            {
                const std::vector<bool>& row{ field[y] };

                for (size_t dx = 0; dx < nrToCompare; dx++)
                {
                    if (row[nrLeft - 1 - dx] != row[nrLeft + dx])
                    {
                        if (diffCount > nrOfSmudges)
                        {
                            return false;
                        }

                        diffCount++;
                    }
                }
            }

            return diffCount == nrOfSmudges;
        }

        [[nodiscard]] bool verticalReflectionIsSmudgedValid(const size_t nrTop, int nrOfSmudges) const
        {
            const auto width = field[0].size();
            const auto height = field.size();

            const size_t nrToCompare = std::min(nrTop, height - nrTop);
            int diffCount{};

            for (size_t x = 0; x < width; x++)
            {
                for (size_t dy = 0; dy < nrToCompare; dy++)
                {
                    if (field[nrTop - 1 - dy][x] != field[nrTop + dy][x])
                    {
                        if (diffCount > nrOfSmudges)
                        {
                            return false;
                        }

                        diffCount++;
                    }
                }
            }

            return diffCount == nrOfSmudges;
        }
    };

    std::vector<Field> parseInput(std::ifstream& file)
    {
        std::vector<Field> fields{};
        Field field{ std::vector<std::vector<bool>>{} };

        while(!file.eof())
        {
            std::string line;
            std::getline(file, line);

            if (line.empty())
            {
                fields.push_back(field);
                field = Field{ std::vector<std::vector<bool>>{} };
            }
            else
            {
                std::vector<bool> rowInField;
                for (char c : line)
                {
                    rowInField.push_back(c == '#');
                }
                field.field.push_back(rowInField);
            }
        }

        // Add the last field that's not ended by empty line:
        fields.push_back(field);
        return fields;
    }

    long long findReflectionScore(const Field& field)
    {
        for (size_t nrLeft = 1; nrLeft < field.field[0].size(); nrLeft++)
        {
            if (field.horizontalReflectionIsValid(nrLeft))
            {
                return static_cast<long long>(nrLeft);
            }
        }

        for (size_t nrTop = 1; nrTop < field.field.size(); nrTop++)
        {
            if (field.verticalReflectionIsValid(nrTop))
            {
                return 100LL * static_cast<long long>(nrTop);
            }
        }

        return 0;
    }

    long long findSmudgedReflectionScore(const Field& field)
    {
        for (size_t nrLeft = 1; nrLeft < field.field[0].size(); nrLeft++)
        {
            if (field.horizontalReflectionIsSmudgedValid(nrLeft, 1))
            {
                return static_cast<long long>(nrLeft);
            }
        }

        for (size_t nrTop = 1; nrTop < field.field.size(); nrTop++)
        {
            if (field.verticalReflectionIsSmudgedValid(nrTop, 1))
            {
                return 100LL * static_cast<long long>(nrTop);
            }
        }

        return 0;
    }

    long long solvePart1(std::ifstream& file)
    {
        const auto fields = parseInput(file);
        long long sum{};

        for (auto& field : fields)
        {
            sum += findReflectionScore(field);
        }

        return sum;
    }

    long long solvePart2(std::ifstream& file)
    {
        const auto fields = parseInput(file);
        long long sum{};

        for (auto& field : fields)
        {
            sum += findSmudgedReflectionScore(field);
        }

        return sum;
    }
}