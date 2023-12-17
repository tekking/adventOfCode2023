#include "day17.h"
#include "helpers.h"

#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <ranges>
#include <queue>
#include <string>
#include <vector>

namespace day17
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 17 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day17_example.txt" : "inputs/day17_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    enum class Direction
    {
        north,
        east,
        south,
        west
    };

    struct State
    {
        size_t x{};
        size_t y{};
        int nrMovedStraight{};
        Direction lastDirection{};
        int costSoFar{};
        size_t heuristicScore{};

        [[nodiscard]] auto calculateHeuristicScore(const size_t goalX, const size_t goalY) const
        {
            return costSoFar + (goalX - x) + (goalY - y);
        }

        State(size_t xP, size_t yP, int nrMovedStraightP, Direction lastDirectionP, int costSoFarP, size_t goalX, size_t goalY)
        {
            x = xP;
            y = yP;
            nrMovedStraight = nrMovedStraightP;
            lastDirection = lastDirectionP;
            costSoFar = costSoFarP;
            heuristicScore = calculateHeuristicScore(goalX, goalY);
        }
    };

    struct City
    {
        std::vector<std::vector<int>> map;
        size_t width;
        size_t height;

        explicit City(std::ifstream& file)
        {
            map = std::vector<std::vector<int>>{};
            while(!file.eof())
            {
                std::string line;
                std::getline(file, line);
                std::vector<int> row{};
                for (char c : line)
                {
                    row.push_back(c - '0');
                }
                map.push_back(row);
            }

            width = map[0].size();
            height = map.size();
        }

        [[nodiscard]] auto heuristicScoreState(const State& s) const
        {
            return (width - s.x - 1) + (height - s.y - 1);
        }
    };

    // A*, though state is complicated by the 3 in 1 direction restraint.
    // Using manhattan distance as remainder heuristic
    long long determineShortestPathLength(const City& city)
    {
        const size_t goalX{ city.width - 1 };
        const size_t goalY{ city.height - 1 };
        const State startPos{ 0, 0, 0, Direction::east, 0, goalX, goalY };

        // To keep track of passed states
        // bool, 16 array for nrMovedStraight * 4 + direction.
        std::vector passed(city.height, std::vector(city.width, std::array<bool, 16>{}));


        auto cmp = [](const State l, const State r) { return l.heuristicScore > r.heuristicScore; };
        std::priority_queue<State, std::vector<State>, decltype(cmp)> statePriorityQueue(cmp);
        statePriorityQueue.push(startPos);

        while(!statePriorityQueue.empty())
        {
            State s{ statePriorityQueue.top() };
            statePriorityQueue.pop();

            const auto directionIndex = s.nrMovedStraight * 4LL + (static_cast<size_t>(s.lastDirection));
            if (passed[s.y][s.x][directionIndex])
            {
                continue;
            }

            passed[s.y][s.x][directionIndex] = true;

            if (s.y == goalY && s.x == goalX)
            {
                return s.costSoFar;
            }

            if (s.y > 0 && s.lastDirection != Direction::south)
            {
                if (s.lastDirection == Direction::north)
                {
                    if (s.nrMovedStraight < 3)
                    {
                        State newState{ s.x, s.y - 1, s.nrMovedStraight + 1, Direction::north, s.costSoFar + city.map[s.y - 1][s.x], goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
                else
                {
                    State newState{ s.x, s.y - 1, 1, Direction::north, s.costSoFar + city.map[s.y - 1][s.x], goalX, goalY };
                    statePriorityQueue.push(newState);
                }
            }

            if (s.y < goalY && s.lastDirection != Direction::north)
            {
                if (s.lastDirection == Direction::south)
                {
                    if (s.nrMovedStraight < 3)
                    {
                        State newState{ s.x, s.y + 1, s.nrMovedStraight + 1, Direction::south, s.costSoFar + city.map[s.y + 1][s.x], goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
                else
                {
                    State newState{ s.x, s.y + 1, 1, Direction::south, s.costSoFar + city.map[s.y + 1][s.x], goalX, goalY };
                    statePriorityQueue.push(newState);
                }
            }

            if (s.x > 0 && s.lastDirection != Direction::east)
            {
                if (s.lastDirection == Direction::west)
                {
                    if (s.nrMovedStraight < 3)
                    {
                        State newState{ s.x - 1, s.y, s.nrMovedStraight + 1, Direction::west, s.costSoFar + city.map[s.y][s.x - 1], goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
                else
                {
                    State newState{ s.x - 1, s.y, 1, Direction::west, s.costSoFar + city.map[s.y][s.x - 1], goalX, goalY };
                    statePriorityQueue.push(newState);
                }
            }

            if (s.x < goalX && s.lastDirection != Direction::west)
            {
                if (s.lastDirection == Direction::east)
                {
                    if (s.nrMovedStraight < 3)
                    {
                        State newState{ s.x + 1, s.y, s.nrMovedStraight + 1, Direction::east, s.costSoFar + city.map[s.y][s.x + 1], goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
                else
                {
                    State newState{ s.x + 1, s.y, 1, Direction::east, s.costSoFar + city.map[s.y][s.x + 1], goalX, goalY };
                    statePriorityQueue.push(newState);
                }
            }
        }

        // Found no path to goal
        assert(false);
        return 0;
    }

    // A*, though state is complicated by the min 4, max 10 in 1 direction restraint.
    // Using manhattan distance as remainder heuristic
    // Not actually 100% sure that heuristic is admissible here, because of the >= 4 moved straight
    // requirement at the end, but it works for the inputs.
    long long determineShortestPathLengthForUltraCrucible(const City& city)
    {
        const size_t goalX{ city.width - 1 };
        const size_t goalY{ city.height - 1 };
        const State startPos{ 0, 0, 0, Direction::east, 0, goalX, goalY };

        // To keep track of passed states
        // bool, 44 array for nrMovedStraight * 4 + direction.
        std::vector passed(city.height, std::vector(city.width, std::array<bool, 44>{}));


        auto cmp = [](const State l, const State r) { return l.heuristicScore > r.heuristicScore; };
        std::priority_queue<State, std::vector<State>, decltype(cmp)> statePriorityQueue(cmp);
        statePriorityQueue.push(startPos);

        while (!statePriorityQueue.empty())
        {
            State s{ statePriorityQueue.top() };
            statePriorityQueue.pop();

            const auto directionIndex = s.nrMovedStraight * 4LL + (static_cast<size_t>(s.lastDirection));
            if (passed[s.y][s.x][directionIndex])
            {
                continue;
            }

            passed[s.y][s.x][directionIndex] = true;

            if (s.y == goalY && s.x == goalX && s.nrMovedStraight >= 4)
            {
                return s.costSoFar;
            }

            if (s.y > 0 && s.lastDirection != Direction::south)
            {
                if (s.lastDirection == Direction::north)
                {
                    if (s.nrMovedStraight < 10)
                    {
                        State newState{ s.x, s.y - 1, s.nrMovedStraight + 1, Direction::north, s.costSoFar + city.map[s.y - 1][s.x], goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
                else
                {
                    if (s.nrMovedStraight >= 4 && s.y > 3)
                    {
                        int costSoFar{ s.costSoFar };
                        for(int i =1; i <= 4; i++)
                        {
                            costSoFar += city.map[s.y - i][s.x];
                        }
                        State newState{ s.x, s.y - 4, 4, Direction::north, costSoFar, goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
            }

            if (s.y < goalY && s.lastDirection != Direction::north)
            {
                if (s.lastDirection == Direction::south)
                {
                    if (s.nrMovedStraight < 10)
                    {
                        State newState{ s.x, s.y + 1, s.nrMovedStraight + 1, Direction::south, s.costSoFar + city.map[s.y + 1][s.x], goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
                else
                {
                    if (s.nrMovedStraight >= 4 && s.y < goalY - 3)
                    {
                        int costSoFar{ s.costSoFar };
                        for (int i = 1; i <= 4; i++)
                        {
                            costSoFar += city.map[s.y + i][s.x];
                        }

                        State newState{ s.x, s.y + 4, 4, Direction::south, costSoFar, goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
            }

            if (s.x > 0 && s.lastDirection != Direction::east)
            {
                if (s.lastDirection == Direction::west)
                {
                    if (s.nrMovedStraight < 10)
                    {
                        State newState{ s.x - 1, s.y, s.nrMovedStraight + 1, Direction::west, s.costSoFar + city.map[s.y][s.x - 1], goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
                else
                {
                    if (s.nrMovedStraight >= 4 && s.x > 3)
                    {
                        int costSoFar{ s.costSoFar };
                        for (int i = 1; i <= 4; i++)
                        {
                            costSoFar += city.map[s.y][s.x - i];
                        }

                        State newState{ s.x - 4, s.y, 4, Direction::west, costSoFar, goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
            }

            if (s.x < goalX && s.lastDirection != Direction::west)
            {
                if (s.lastDirection == Direction::east)
                {
                    if (s.nrMovedStraight < 10)
                    {
                        State newState{ s.x + 1, s.y, s.nrMovedStraight + 1, Direction::east, s.costSoFar + city.map[s.y][s.x + 1], goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
                else
                {
                    if (s.nrMovedStraight >= 4 && s.x < goalX - 3)
                    {
                        int costSoFar{ s.costSoFar };
                        for (int i = 1; i <= 4; i++)
                        {
                            costSoFar += city.map[s.y][s.x + i];
                        }

                        State newState{ s.x + 4, s.y, 4, Direction::east, costSoFar, goalX, goalY };
                        statePriorityQueue.push(newState);
                    }
                }
            }
        }

        // Found no path to goal
        assert(false);
        return 0;
    }

    long long solvePart1(std::ifstream& file)
    {
        const City city(file);
        return determineShortestPathLength(city);
    }

    long long solvePart2(std::ifstream& file)
    {
        const City city(file);
        return determineShortestPathLengthForUltraCrucible(city);
    }
}