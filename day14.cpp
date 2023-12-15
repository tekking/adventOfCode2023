#include "day14.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

namespace day14
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 14 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day14_example.txt" : "inputs/day14_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    enum class Rock
    {
        empty,
        round,
        square
    };

    struct Platform
    {
        std::vector<std::vector<Rock>> field{};

        void print() const
        {
            std::cout << '\n';
            for (const auto& row : field)
            {
                for (const auto rock : row)
                {
                    switch (rock) {
                    case Rock::empty:
                        std::cout << '.';
                        break;

                    case Rock::round:
                        std::cout << 'O';
                        break;

                    case Rock::square:
                        std::cout << '#';
                        break;
                    }
                }

                std::cout << '\n';
            }
        }

        void rollNorth()
        {
            for(size_t x = 0; x < field[0].size(); x++)
            {
                size_t nextAvailableSpot{};
                for (size_t y = 0; y < field.size(); y++)
                {
                    switch (field[y][x])
                    {
                    case Rock::round:
                        // Move the round rock up
                        field[y][x] = Rock::empty;
                        field[nextAvailableSpot][x] = Rock::round;
                        nextAvailableSpot++;
                        break;

                    case Rock::square:
                        nextAvailableSpot = y + 1;
                        break;

                    case Rock::empty:
                        break;
                    }
                }
            }
        }

        void rollSouth()
        {
            for (size_t x = 0; x < field[0].size(); x++)
            {
                size_t nextAvailableSpot{field.size() - 1};
                for (size_t y = field.size() - 1; y < field.size(); y--)
                {
                    switch (field[y][x])
                    {
                    case Rock::round:
                        // Move the round rock down
                        field[y][x] = Rock::empty;
                        field[nextAvailableSpot][x] = Rock::round;
                        nextAvailableSpot--;
                        break;

                    case Rock::square:
                        nextAvailableSpot = y - 1;
                        break;

                    case Rock::empty:
                        break;
                    }
                }
            }
        }

        void rollWest()
        {
            for (size_t y = 0; y < field.size(); y++)
            {
                size_t nextAvailableSpot{};
                for (size_t x = 0; x < field[0].size(); x++)
                {
                    switch (field[y][x])
                    {
                    case Rock::round:
                        // Move the round rock west
                        field[y][x] = Rock::empty;
                        field[y][nextAvailableSpot] = Rock::round;
                        nextAvailableSpot++;
                        break;

                    case Rock::square:
                        nextAvailableSpot = x + 1;
                        break;

                    case Rock::empty:
                        break;
                    }
                }
            }
        }

        void rollEast()
        {
            for (size_t y = 0; y < field.size(); y++)
            {
                size_t nextAvailableSpot{field[0].size() - 1};
                for (size_t x = field[0].size() - 1; x < field[0].size(); x--)
                {
                    switch (field[y][x])
                    {
                    case Rock::round:
                        // Move the round rock east
                        field[y][x] = Rock::empty;
                        field[y][nextAvailableSpot] = Rock::round;
                        nextAvailableSpot--;
                        break;

                    case Rock::square:
                        nextAvailableSpot = x - 1;
                        break;

                    case Rock::empty:
                        break;
                    }
                }
            }
        }

        void cycleRoll(bool debug = false)
        {
            rollNorth();
            if (debug)
            {
                print();
            }

            rollWest();
            if (debug)
            {
                print();
            }

            rollSouth();
            if (debug)
            {
                print();
            }

            rollEast();
            if (debug)
            {
                print();
            }
        }

        [[nodiscard]] long long scoreNorthWeight() const
        {
            long long sum{};
            for (size_t x = 0; x < field[0].size(); x++)
            {
                for (size_t y = 0; y < field.size(); y++)
                {
                    if (field[y][x] == Rock::round)
                    {
                        sum += static_cast<long long>(field.size() - y);
                    }
                }
            }

            return sum;
        }

        bool operator==(const Platform& other) const
        {
            for (size_t y = 0; y < field.size(); y++)
            {
                for (size_t x = 0; x < field[0].size(); x++)
                {
                    if (other.field[y][x] != field[y][x])
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    };

    Platform parseInput(std::ifstream& file)
    {
        std::string line;
        std::vector<std::vector<Rock>> field{};
        while(!file.eof())
        {
            std::vector<Rock> row;
            std::getline(file, line);

            for (const char i : line)
            {
                switch (i)
                {
                case 'O':
                    row.push_back(Rock::round);
                    break;

                case '#':
                    row.push_back(Rock::square);
                    break;

                default:
                    row.push_back(Rock::empty);
                    break;
                }
            }

            field.push_back(row);
        }

        return Platform{ field };
    }

    long long solvePart1(std::ifstream& file)
    {
        auto platform = parseInput(file);
        platform.rollNorth();
        // platform.print();
        return platform.scoreNorthWeight();
    }

    long long solvePart2(std::ifstream& file)
    {
        auto platform = parseInput(file);

        // Assumption: the cycling will hit a fairly short cycle pretty quickly, and then we can just calculate final result from that.
        std::vector<Platform> previousStates{};
        long long iteration{};
        long long offsetOfCycle;
        long long cycleLength;
        while(true)
        {
            auto match = std::ranges::find(previousStates, platform);
            if (match != previousStates.end())
            {
                offsetOfCycle = match - previousStates.begin();
                cycleLength = iteration - offsetOfCycle;
                break;
            }

            previousStates.push_back(platform);
            iteration++;

            platform.cycleRoll();
        }

        // Know the cycle now, we can calculate from here:
        constexpr long long targetIterations = 1000000000;

        const long long placeInCycle{ (targetIterations - offsetOfCycle) % cycleLength };
        const auto finalPlatform{ previousStates[static_cast<size_t>(offsetOfCycle + placeInCycle)] };

        // platform.print();
        return finalPlatform.scoreNorthWeight();
    }
}