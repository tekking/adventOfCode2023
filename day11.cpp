#include "day11.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

namespace day11
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 11 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day11_example.txt" : "inputs/day11_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct Galaxy
    {
        int x{};
        int y{};
        int originalX{};
        int originalY{};
    };

    std::vector<Galaxy> parseInput(std::ifstream& file)
    {
        std::vector<Galaxy> result{};
        int y{};
        while (!file.eof())
        {
            std::string line;
            std::getline(file, line);
            for(size_t x = 0; x < line.size(); x++)
            {
                if (line[x] == '#')
                {
                    result.push_back(Galaxy{ static_cast<int>(x), y, static_cast<int>(x), y});
                }
            }

            y++;
        }

        return result;
    }

    void expandGalaxies(std::vector<Galaxy>& galaxies, const int expandFactor)
    {
        int maxX{};
        int maxY{};
        for (auto& galaxy : galaxies)
        {
            maxX = std::max(maxX, galaxy.x);
            maxY = std::max(maxY, galaxy.y);
        }

        // Expand the empty space, by moving galaxies with higher x/y out, first on y, then x
        // Don't modify original so we can keep using that for comparison
        for (int y = 0; y < maxY; y++)
        {
            if (!std::ranges::any_of(galaxies, [y](Galaxy g) {return g.originalY == y; }))
            {
                for (auto& galaxy : galaxies)
                {
                    if (galaxy.originalY > y)
                    {
                        galaxy.y += (expandFactor - 1);
                    }
                }
            }
        }

        for (int x = 0; x < maxX; x++)
        {
            if (!std::ranges::any_of(galaxies, [x](Galaxy g) {return g.originalX == x; }))
            {
                for (auto& galaxy : galaxies)
                {
                    if (galaxy.originalX > x)
                    {
                        galaxy.x += (expandFactor - 1);
                    }
                }
            }
        }
    }

    long long solvePart1(std::ifstream& file)
    {
        auto galaxies{ parseInput(file) };
        expandGalaxies(galaxies, 2);

        long long distanceSum{};
        for(size_t outer = 0; outer < galaxies.size(); outer++)
        {
            for(size_t inner = outer + 1; inner < galaxies.size(); inner++)
            {
                // Shortest path is just manhattan distance
                distanceSum += std::abs(galaxies[inner].x - galaxies[outer].x) + std::abs(galaxies[inner].y - galaxies[outer].y);
            }
        }

        return distanceSum;
    }

    long long solvePart2(std::ifstream& file)
    {
        // Same as part 1, but just different amount added to x/y when expanding.
        auto galaxies{ parseInput(file) };
        expandGalaxies(galaxies, 1000000);

        long long distanceSum{};
        for (size_t outer = 0; outer < galaxies.size(); outer++)
        {
            for (size_t inner = outer + 1; inner < galaxies.size(); inner++)
            {
                // Shortest path is just manhattan distance
                distanceSum += std::abs(galaxies[inner].x - galaxies[outer].x) + std::abs(galaxies[inner].y - galaxies[outer].y);
            }
        }

        return distanceSum;
    }
}