#include "day23.h"
#include "helpers.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

namespace day23
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 23 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day23_example.txt" : "inputs/day23_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct Point
    {
        int x{};
        int y{};

        Point operator+(const Point& other) const
        {
            return Point{ x + other.x, y + other.y };
        }

        bool operator==(const Point& other) const
        {
            return x == other.x && y == other.y;
        }

        bool operator!=(const Point& other) const
        {
            return !(*this == other);
        }
    };

    struct PointHasher
    {
        std::size_t operator()(const Point& p) const
        {
            // bad hash
            return static_cast<size_t>(((p.x * 179) + (p.y * 3213)) % 329133);
        }
    };

    // Orders of below two north, east, south, west
    std::array<Point, 4> adjacentOffsets{
        Point{0, -1},
        Point{1, 0},
        Point{0, 1},
        Point{-1, 0}
    };

    std::array<char, 4> exitArrows{ '^', '>', 'v', '<' };

    bool isArrow(char c)
    {
        return c == '>' || c == 'v' || c == '<' || c == '^';
    }

    // Assumption from looking at example & real input:
    // Map consists of route segments that end in a choice of two routes
    // Probably no loops?
    struct Map
    {
        std::vector < std::string> tiles{};
        int height{};
        int width{};

        void parseInput(std::ifstream& file)
        {
            while (!file.eof())
            {
                std::string line;
                std::getline(file, line);
                tiles.push_back(line);
            }

            height = static_cast<int>(tiles.size());
            width = static_cast<int>(tiles[0].size());
        }

        [[nodiscard]] char getPos(const Point& p) const
        {
            if (p.y < 0 || p.y >= height || p.x < 0 || p.x >= width)
            {
                // Just treat out of bounds as # for simplicity
                return '#';
            }

            return tiles[static_cast<size_t>(p.y)][static_cast<size_t>(p.x)];
        }

        [[nodiscard]] long long findLongestRoute() const
        {
            // Find starting tile
            Point startPos;
            for (size_t x = 0; x < static_cast<size_t>(width); x ++ )
            {
                if (tiles[0][x] == '.')
                {
                    startPos = Point{ static_cast<int>(x), 0 };
                }
            }

            return findLongestFromPoint(startPos);
        }

        [[nodiscard]] long long findLongestFromPoint(Point p) const
        {
            // Assumption here:
            // After split/start we're always in a spot with 1 adjacent '.' square
            // Follow this finding one new adjacent '.' every step, till we reach a
            // '>' or end.

            long long moved{};
            bool foundNextPoint{ false };
            Point nextPointOnPath;
            for (const auto& adjacentOffset : adjacentOffsets)
            {
                const Point option{ p + adjacentOffset };

                if (getPos(option) == '.')
                {
                    // Validation 1 adjacent point.
                    assert(!foundNextPoint);

                    nextPointOnPath = option;
                    foundNextPoint = true;
                }
            }

            moved++;

            // Follow path till there is an adjacent arrow
            // Assumption: don't need to check for bounds of map because map is surrounded by '#'
            Point lastPoint{ p };
            Point curPoint{ nextPointOnPath };
            while (true)
            {
                if (curPoint.y == (height - 1))
                {
                    // Reached the goal (bottom row)
                    return moved;
                }

                foundNextPoint = false;
                for (const auto& adjacentOffset : adjacentOffsets)
                {
                    const Point option{ curPoint + adjacentOffset };

                    if (option != lastPoint && getPos(option) == '.')
                    {
                        // Validation 1 adjacent point.
                        assert(!foundNextPoint);

                        nextPointOnPath = option;
                        foundNextPoint = true;
                    }
                }

                if (foundNextPoint)
                {
                    moved++;
                    lastPoint = curPoint;
                    curPoint = nextPointOnPath;
                    continue;
                }

                // If we didn't find '.', look for arrow
                bool foundArrow{ false };
                for (const auto& adjacentOffset : adjacentOffsets)
                {
                    const Point option{ curPoint + adjacentOffset };
                    if (option != lastPoint && isArrow(getPos(option)))
                    {
                        // Validation 1 adjacent point.
                        assert(!foundArrow);

                        // Move an extra square along the arrow
                        nextPointOnPath = option + adjacentOffset;
                        foundArrow = true;
                    }
                }

                assert(foundArrow);
                curPoint = nextPointOnPath;
                moved += 2;
                break;
            }

            // Now we are at an intersection
            std::vector<Point> exits{};

            for (size_t i = 0; i < 4; i++)
            {
                Point option{ curPoint + adjacentOffsets[i] };
                if (getPos(option) == exitArrows[i])
                {
                    // Move second step across the arrow
                    exits.push_back(option + adjacentOffsets[i]);
                }
            }

            assert(!exits.empty());
            moved += 2;

            long long maxOfSubPaths{};
            for (const auto& exit : exits)
            {
                maxOfSubPaths = std::max(maxOfSubPaths, findLongestFromPoint(exit));
            }

            return moved + maxOfSubPaths;
        }
    };

    // Assumption for part two:
    // map can be split up in intersections (points with >2 ajacent empty points) & paths inbetween
    // Approach is to parse into graph of intersection and pathweight inbetween.
    struct Intersection
    {
        Point pos{};
        size_t index{};

        std::vector <std::pair<long long, size_t>> adjacentIntersectionIndices{};

        bool isStart{};
        bool isFinish{};

        [[nodiscard]] bool operator==(const Intersection& other) const
        {
            return pos == other.pos;
        }
    };

    struct PartTwoMap
    {
        std::vector < std::string> tiles{};
        int height{};
        int width{};

        std::unordered_map<size_t, Intersection> intersectionLookupByIndex{};
        size_t intersectionIndex{};

        void parseInput(std::ifstream& file)
        {
            while (!file.eof())
            {
                std::string line;
                std::getline(file, line);
                tiles.push_back(line);
            }

            height = static_cast<int>(tiles.size());
            width = static_cast<int>(tiles[0].size());
        }

        [[nodiscard]] char getPos(const Point& p) const
        {
            if (p.y < 0 || p.y >= height || p.x < 0 || p.x >= width)
            {
                // Just treat out of bounds as # for simplicity
                return '#';
            }

            if (tiles[static_cast<size_t>(p.y)][static_cast<size_t>(p.x)] == '#')
            {
                return '#';
            }

            // Everything except # is . in part2
            return '.';
        }

        void determineIntersections()
        {
            // Start by adding start pos
            Point startPos;
            for (size_t x = 0; x < static_cast<size_t>(width); x++)
            {
                if (tiles[0][x] == '.')
                {
                    startPos = Point{ static_cast<int>(x), 0 };
                }
            }

            Intersection startIntersection{ startPos };
            startIntersection.isStart = true;
            startIntersection.index = 0;
            intersectionLookupByIndex[0] = startIntersection;
            intersectionIndex++;

            // Recursively add all intersections to lookup, starting from the startPos
            addIntersectionToGraph(intersectionLookupByIndex[0]);
        }

        void addIntersectionToGraph(const Intersection& intersection)
        {
            Intersection& intersectionRef{ intersectionLookupByIndex[intersection.index] };
            std::vector<Intersection> intersectionsToRecurOn{};

            for (auto adjacentOffset : adjacentOffsets)
            {
                Point option{ adjacentOffset + intersectionRef.pos };
                if (getPos(option) == '.')
                {
                    auto result{ followPath(option, intersectionRef.pos) };

                    // Determine to recur on the found intersection
                    Intersection i{ result.second };
                    if (std::ranges::any_of(intersectionLookupByIndex, [i](const std::pair<size_t, Intersection>& p) {return p.second == i; }))
                    {
                        const auto existingInter{ *std::ranges::find_if(intersectionLookupByIndex, [i](const std::pair<size_t, Intersection>& p) {return p.second == i; }) };
                        intersectionRef.adjacentIntersectionIndices.emplace_back(result.first, existingInter.second.index);
                        continue;
                    }

                    i.index = intersectionIndex;
                    intersectionIndex++;
                    intersectionLookupByIndex[i.index] = i;

                    if (i.pos.y == height - 1)
                    {
                        // Intersection is end node, don't need to recur from it
                        // just add directly to lookup
                        intersectionLookupByIndex[i.index].isFinish = true;
                    }
                    else
                    {
                        // Is an intersection to recur on
                        intersectionsToRecurOn.push_back(intersectionLookupByIndex[i.index]);
                    }

                    // Add the result to the adjacents of current intersection.
                    intersectionRef.adjacentIntersectionIndices.emplace_back(result.first, i.index);
                }
            }

            for (auto& i : intersectionsToRecurOn)
            {
                addIntersectionToGraph(i);
            }
        }

        // Follows a path (until an intersection is reached). Returns the coordinates of the intersection
        // and number of steps to reach it from originPoint.
        [[nodiscard]] std::pair<long long, Point> followPath(const Point firstPointOnPath, const Point originPoint) const
        {
            // Repeatedly follow path until we reach a node where non-previous adjacent empty positions != 1.
            long long moved{ 1 };
            Point lastPoint{ originPoint };
            Point curPoint{ firstPointOnPath };

            while (true)
            {
                bool foundNextPoint{false};
                Point nextPoint;
                bool foundIntersection{ false };

                for (const auto& adjacentOffset : adjacentOffsets)
                {
                    const Point option{ curPoint + adjacentOffset };

                    if (option != lastPoint && getPos(option) == '.')
                    {
                        // Validation 1 adjacent point.
                        if (foundNextPoint)
                        {
                            // Found a node with two possible next points, meaning intersection;
                            foundIntersection = true;
                            break;
                        }

                        nextPoint = option;
                        foundNextPoint = true;
                    }
                }

                if (foundIntersection)
                {
                    break;
                }

                if (!foundNextPoint)
                {
                    // No next point found, assumption: this only happens at start/end (no other deadends in maze)
                    assert(curPoint.y == 0 || curPoint.y == width - 1);
                    break;
                }

                if (foundNextPoint)
                {
                    moved++;
                    lastPoint = curPoint;
                    curPoint = nextPoint;
                    continue;
                }
            }

            return std::pair{ moved, curPoint };
        }

        struct searchState
        {
            size_t intersectionIndex;
            long long passedLookup;
            long long distance;
        };

        long long findLongestPathStackBased()
        {
            std::vector<searchState> stateStack{ searchState{0, 1LL << 0, 0LL}};
            long long maxDistanceFound{};

            while(!stateStack.empty())
            {
                searchState state{ stateStack.back() };
                stateStack.pop_back();

                const auto& intersection{ intersectionLookupByIndex[state.intersectionIndex] };

                if (intersection.isFinish)
                {
                    maxDistanceFound = std::max(state.distance, maxDistanceFound);
                    continue;
                }

                for (const auto& [distance, adjacentIntersectIndex] : intersection.adjacentIntersectionIndices)
                {
                    const long long bitMask{ 1LL << adjacentIntersectIndex };
                    if ((state.passedLookup & bitMask) > 1)
                    {
                        // already passed in the past of this state
                        continue;
                    }

                    stateStack.emplace_back(
                        adjacentIntersectIndex,
                        state.passedLookup | bitMask,
                        state.distance + distance
                    );
                }
            }

            return maxDistanceFound;
        }

        // long long findLongestPathLengthThroughIntersections()
        // {
        //     // naive implementation where we just pass lookup of which intersections we already passed:
        //
        //     // Start by finding start pos
        //     Point startPos;
        //     for (size_t x = 0; x < static_cast<size_t>(width); x++)
        //     {
        //         if (tiles[0][x] == '.')
        //         {
        //             startPos = Point{ static_cast<int>(x), 0 };
        //         }
        //     }
        //
        //     std::array<bool, 40> passedLookup{};
        //     // return longestRecur(startPos, passedLookup);
        //     return longestRecur(startPos, 0LL);
        // }

        // // debug version that also returns the points taken in longest path
        // std::pair<long long, std::vector<Point>> findLongestPathThroughIntersections()
        // {
        //     // naive implementation where we just pass lookup of which intersections we already passed:
        //
        //     // Start by finding start pos
        //     Point startPos;
        //     for (size_t x = 0; x < static_cast<size_t>(width); x++)
        //     {
        //         if (tiles[0][x] == '.')
        //         {
        //             startPos = Point{ static_cast<int>(x), 0 };
        //         }
        //     }
        //
        //     std::array<bool, 40> passedLookup{};
        //     return longestRecurWithPath(startPos, passedLookup);
        // }
        //
        // // note: 40 is large enough for my input
        // long long longestRecur(const Point pos, std::array<bool, 40>& passedLookup)
        // {
        //     const auto& intersection{ intersectionsLookup[pos] };
        //
        //     if (passedLookup[intersection.index])
        //     {
        //         // Just large negative to discourage reusing passed intersections.
        //         return -1000000;
        //     }
        //
        //     if (intersection.isFinish)
        //     {
        //         // End
        //         return 0;
        //     }
        //
        //     passedLookup[intersection.index] = true;
        //
        //     long long max{-100000000};
        //     for (const auto& [distance, adjacentIntersectPos] : intersection.adjacentIntersections)
        //     {
        //         max = std::max(max, distance + longestRecur(adjacentIntersectPos, passedLookup));
        //     }
        //
        //     // Make passedLookup reusable for above in recursion without needing to make copies.
        //     passedLookup[intersection.index] = false;
        //     return max;
        // }

        // // note: 40 is large enough for my input
        // long long longestRecur(const Point pos, long long passedLookup)
        // {
        //     const auto& intersection{ intersectionsLookup[pos] };
        //     const long long bitMask{ 1LL << intersection.index };
        //
        //     if ((passedLookup & bitMask) > 1)
        //     {
        //         // Just large negative to discourage reusing passed intersections.
        //         return -1000000;
        //     }
        //
        //     if (intersection.isFinish)
        //     {
        //         // End
        //         return 0;
        //     }
        //
        //     passedLookup |= bitMask;
        //
        //     long long max{ -100000000 };
        //     for (const auto& [distance, adjacentIntersectPos] : intersection.adjacentIntersections)
        //     {
        //         max = std::max(max, distance + longestRecur(adjacentIntersectPos, passedLookup));
        //     }
        //
        //     // Make passedLookup reusable for above in recursion without needing to make copies.
        //     return max;
        // }

        // // debug version that also returns the points taken in longest path
        // std::pair<long long, std::vector<Point>> longestRecurWithPath(const Point pos, std::array<bool, 40>& passedLookup)
        // {
        //     const auto& intersection{ intersectionsLookup[pos] };
        //
        //     if (passedLookup[intersection.index])
        //     {
        //         // Just large negative to discourage reusing passed intersections.
        //         return std::pair{ -1000000, std::vector<Point>{intersection.pos} };
        //     }
        //
        //     if (intersection.isFinish)
        //     {
        //         // End
        //         return std::pair{0, std::vector<Point>{intersection.pos}};
        //     }
        //
        //     passedLookup[intersection.index] = true;
        //
        //     long long max{-100000000};
        //     std::vector<Point> longestPath;
        //     for (const auto& [distance, adjacentIntersectPos] : intersection.adjacentIntersections)
        //     {
        //         const auto& [subDist, subPath]{ longestRecurWithPath(adjacentIntersectPos, passedLookup) };
        //         if (subDist + distance > max)
        //         {
        //             max = subDist + distance;
        //             longestPath = subPath;
        //         }
        //     }
        //
        //     // Make passedLookup reusable for above in recursion without needing to make copies.
        //     passedLookup[intersection.index] = false;
        //     longestPath.push_back(pos);
        //     return std::pair{ max, longestPath };
        // }
    };

    long long solvePart1(std::ifstream& file)
    {
        Map map{};
        map.parseInput(file);
        return map.findLongestRoute();
    }

    long long solvePart2(std::ifstream& file)
    {
        PartTwoMap map{};
        map.parseInput(file);
        map.determineIntersections();
        return map.findLongestPathStackBased();
    }
}