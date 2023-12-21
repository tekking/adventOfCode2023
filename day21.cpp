#include "day21.h"
#include "helpers.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace day21
{
    long long solvePart1(std::ifstream& file, bool example);
    long long solvePart2(std::ifstream& file, bool example);

    void run_day(bool example)
    {
        std::cout << "Running day 21 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day21_example.txt" : "inputs/day21_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file, example) << '\n';

        file.close();
        file.open(fileName);

        if (example)
        {
            std::cout << "Current solution here only really works for real input!\n";
            std::cout << "Part 2 answer: " << solvePart2(file, example) << '\n';
        }
        else
        {
            std::cout << "Current solution doesn't actually solve, just gives values to determine quadratic formula with!\n";
            std::cout << "Part 2 answer: " << solvePart2(file, example) << '\n';
        }
    }

    struct Point
    {
        size_t x{};
        size_t y{};

        [[nodiscard]] Point operator+(const Point& other) const
        {
            return Point{ x + other.x, y + other.y };
        }
    };

    std::array<Point, 4> directionOffsets{
        Point(-1, 0),
        Point(1, 0),
        Point(0, -1),
        Point(0, 1)
    };

    struct WalkState
    {
        Point p{};
        int stepsTaken{};
    };

    struct Garden
    {
        std::vector<std::vector<bool>> gardenPlots{};
        std::vector<std::vector<int>> nrOfStepsNeeded{};

        size_t width;
        size_t height;

        size_t startX;
        size_t startY;

        void parseInput(std::ifstream& file)
        {
            size_t y{};
            while(!file.eof())
            {
                std::vector<bool> gardenRow{};
                std::string line;
                std::getline(file, line);
                size_t x{};
                for (const char c : line)
                {
                    gardenRow.push_back(c == '.' || c == 'S');

                    if (c == 'S')
                    {
                        startX = x;
                        startY = y;
                    }

                    x++;
                }

                gardenPlots.push_back(gardenRow);
                y++;
            }

            width = gardenPlots[0].size();
            height = gardenPlots.size();

            nrOfStepsNeeded = std::vector(height, std::vector(width, -1));
        }

        // Simple breadth first fill out of (bounded) min steps
        void fillOutStepsNeeded()
        {
            std::queue<WalkState> queue{};
            queue.emplace(Point{ startX, startY }, 0);
            while(!queue.empty())
            {
                const WalkState& w{ queue.front() };

                if(nrOfStepsNeeded[w.p.y][w.p.x] >= 0)
                {
                    queue.pop();
                    continue;
                }

                nrOfStepsNeeded[w.p.y][w.p.x] = w.stepsTaken;

                for (const auto& directionOffset : directionOffsets)
                {
                    Point newPoint{ w.p + directionOffset };
                    // Using overflow to check for lower bound
                    if (newPoint.x < width && newPoint.y < height
                        && gardenPlots[newPoint.y][newPoint.x])
                    {
                        queue.emplace(newPoint, w.stepsTaken + 1);
                    }
                }

                queue.pop();
            }
        }

        long long partTwoBreadthFirstScore(const long long nrOfSteps)
        {
            // Horrible ugly solution generating extended map
            const auto nrOfExtraCopies{ (nrOfSteps + (width/2)) / width };
            std::vector nrOfStepsNeededWithCopies(height * (1 + 2 * nrOfExtraCopies), std::vector(width * (1 + 2 * nrOfExtraCopies), -1));

            std::queue<WalkState> queue{};
            // start offset by nrOfExtraCopies copies on x & y
            queue.emplace(Point{ startX + nrOfExtraCopies * width, startY + nrOfExtraCopies * height }, 0);
            while (!queue.empty())
            {
                const WalkState& w{ queue.front() };

                if (w.stepsTaken > nrOfSteps)
                {
                    break;
                }

                if (nrOfStepsNeededWithCopies[w.p.y][w.p.x] >= 0)
                {
                    queue.pop();
                    continue;
                }

                nrOfStepsNeededWithCopies[w.p.y][w.p.x] = w.stepsTaken;

                for (const auto& directionOffset : directionOffsets)
                {
                    // note skipping over logic for edge walking here since we added extra copies anyway
                    Point newPoint{ w.p + directionOffset };

                    if (gardenPlots[newPoint.y % height][newPoint.x % width])
                    {
                        queue.emplace(newPoint, w.stepsTaken + 1);
                    }
                }

                queue.pop();
            }


            long long sum{};
            for (const auto& row : nrOfStepsNeededWithCopies)
            {
                for (const auto r : row)
                {
                    if (r % 2 == (nrOfSteps % 2))
                    {
                        sum++;
                    }
                }
            }

            return sum;
        }

        [[nodiscard]] long long scoreNrOfGardensPartOne(const int nrOfSteps) const
        {
            long long sum{};
            for (size_t y = 0; y < height; y++)
            {
                for (size_t x = 0; x < width; x++)
                {
                    if (!gardenPlots[y][x])
                    {
                        continue;
                    }

                    const auto steps{ nrOfStepsNeeded[y][x] };
                    // Can always 'skip' two steps by walking back and forth, so anything
                    // below nrOfSteps is reachable if matches %2.
                    if (steps <= nrOfSteps && (steps % 2) == (nrOfSteps % 2))
                    {
                        sum++;
                    }
                }
            }

            return sum;
        }
    };

    long long solvePart1(std::ifstream& file, bool example)
    {
        Garden garden{};
        garden.parseInput(file);
        garden.fillOutStepsNeeded();

        return garden.scoreNrOfGardensPartOne(example ? 6 : 64);
    }

    long long solvePart2(std::ifstream& file, bool example)
    {
        Garden garden{};
        garden.parseInput(file);

        std::cout << garden.partTwoBreadthFirstScore(65LL) << '\n';
        std::cout << garden.partTwoBreadthFirstScore(65LL + garden.width) << '\n';
        std::cout << garden.partTwoBreadthFirstScore(65LL + 2 * garden.width) << '\n';
        std::cout << garden.partTwoBreadthFirstScore(65LL + 3 * garden.width) << '\n';

        return 0;
    }
}