#include "day10.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

// #define debug

namespace day10
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 10 " << (example ? "(example)" : "") << '\n';

        std::string fileName{ example ? "inputs/day10_example.txt" : "inputs/day10_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        fileName = example ? "inputs/day10_example2.txt" : "inputs/day10_real.txt";
        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct Point
    {
        size_t x;
        size_t y;

        bool operator==(const Point& other) const
        {
            return other.x == x && other.y == y;
        }

        bool operator!=(const Point& other) const
        {
            return !(*this == other);
        }
    };

    struct PipeMap
    {
        std::vector<std::string> mapSymbols;

        size_t height{ mapSymbols.size() };
        size_t width{ mapSymbols[0].size() };

        [[nodiscard]] Point startPoint() const
        {
            for(size_t y = 0; y < height; y++)
            {
                auto index{ mapSymbols[y].find('S') };
                if (index != std::string::npos)
                {
                    return Point{ index, y };
                }
            }

            // Map should always contain a 'S'
            assert(false);
            return Point{};
        }

        [[nodiscard]] char charAt(Point p) const
        {
            return mapSymbols[p.y][p.x];
        }

        // Returns a (maybe empty) list of points pipe at p connects to.
        [[nodiscard]] std::vector<Point> adjacentPointsFor(Point p) const
        {
            std::vector<Point> result{};

            switch (char c{ charAt(p) })
            {
                case '|':
                    if (p.y > 0)
                        result.push_back(Point{ p.x, p.y - 1 });

                    if (p.y < height - 1)
                        result.push_back(Point{ p.x, p.y + 1 });
                    break;

                case '-':
                    if (p.x > 0)
                        result.push_back(Point{ p.x - 1, p.y });

                    if (p.x < width - 1)
                        result.push_back(Point{ p.x + 1, p.y });
                    break;

                case 'L':
                    if (p.y > 0)
                        result.push_back(Point{ p.x, p.y - 1 });

                    if (p.x < width - 1)
                        result.push_back(Point{ p.x + 1, p.y });
                    break;

                case 'J':
                    if (p.y > 0)
                        result.push_back(Point{ p.x, p.y - 1 });

                    if (p.x > 0)
                        result.push_back(Point{ p.x - 1, p.y });
                    break;

                case '7':
                    if (p.x > 0)
                        result.push_back(Point{ p.x - 1, p.y });

                    if (p.y < height - 1)
                        result.push_back(Point{ p.x, p.y + 1 });
                    break;

                case 'F':
                    if (p.x < width - 1)
                        result.push_back(Point{ p.x + 1, p.y });

                    if (p.y < height - 1)
                        result.push_back(Point{ p.x, p.y + 1 });
                    break;

                case '.':
                case 'S':
                    break;

                default:
                    assert(false);
            }

            return result;
        }

        // Checks if there is a connection from the pipe at p1 to p2
        [[nodiscard]] bool isConnectedTo(Point p1, Point p2) const
        {
            auto connectedPoints{ adjacentPointsFor(p1) };
            return std::ranges::any_of(connectedPoints, [p2](Point p) {return p == p2; });
        }

        // Returns a (maybe empty) list of adjacent points of p, whose pipe connects to p.
        // For use on starting point (where we don't know real pipe).
        [[nodiscard]] std::vector<Point> reverseAdjacentPointsFor(Point p) const
        {
            std::vector<Point> result{};
            Point potentialPoint{};
            if (p.x > 0)
            {
                potentialPoint = Point{ p.x - 1, p.y };
                if (isConnectedTo(potentialPoint, p))
                {
                    result.push_back(potentialPoint);
                }
            }

            if (p.x < width - 1)
            {
                potentialPoint = Point{ p.x + 1, p.y };
                if (isConnectedTo(potentialPoint, p))
                {
                    result.push_back(potentialPoint);
                }
            }

            if (p.y > 0)
            {
                potentialPoint = Point{ p.x, p.y - 1 };
                if (isConnectedTo(potentialPoint, p))
                {
                    result.push_back(potentialPoint);
                }
            }

            if (p.y < height - 1)
            {
                potentialPoint = Point{ p.x, p.y + 1 };
                if (isConnectedTo(potentialPoint, p))
                {
                    result.push_back(potentialPoint);
                }
            }

            return result;
        }

        [[nodiscard]] std::vector<Point> getPathFromStartingPoint() const
        {
            const Point startingPoint{ startPoint() };

            // Find two points adjacent to S that connect to S (since we don't need what's under S)
            const std::vector<Point> adjacentToStart{ reverseAdjacentPointsFor(startingPoint) };
            assert(adjacentToStart.size() == 2);

            // Loop from adjacent[0] till we reach adjacent[1], keeping track of path.
            Point prevPoint{ startingPoint };
            Point walkingPoint{ adjacentToStart[0] };
            std::vector<Point> path{ adjacentToStart[1], startingPoint };
            while (walkingPoint != adjacentToStart[1])
            {
                path.push_back(walkingPoint);
                // Go to the adjacent point, which is not where we came from
                auto adjacentToWalk{ adjacentPointsFor(walkingPoint) };
                if (adjacentToWalk[0] == prevPoint)
                {
                    prevPoint = walkingPoint;
                    walkingPoint = adjacentToWalk[1];
                }
                else
                {
                    prevPoint = walkingPoint;
                    walkingPoint = adjacentToWalk[0];
                }
            }

            return path;
        }

        // Gets the char at point, but also if that is 'S' checks with the path to determine
        // the real pipe at that position.
        [[nodiscard]] char getRealPipeAtPoint(Point point, const std::vector<Point>& path) const
        {
            const char c{ charAt(point) };
            if (c != 'S')
            {
                return c;
            }

            assert(path[1] == point);
            bool connectedNorth{};
            bool connectedEast{};
            bool connectedSouth{};
            bool connectedWest{};

            const Point toNorth{ point.x, point.y - 1 };
            const Point toEast{ point.x + 1, point.y };
            const Point toSouth{ point.x, point.y + 1 };
            const Point toWest{ point.x - 1, point.y };

            if (toNorth == path[0] || toNorth == path[2])
            {
                connectedNorth = true;
            }

            if (toEast == path[0] || toEast == path[2])
            {
                connectedEast = true;
            }

            if (toSouth == path[0] || toSouth == path[2])
            {
                connectedSouth = true;
            }

            if (toWest == path[0] || toWest == path[2])
            {
                connectedWest = true;
            }

            if (connectedNorth && connectedEast)
                return 'L';

            if (connectedNorth && connectedSouth)
                return '|';

            if (connectedNorth && connectedWest)
                return 'J';

            if (connectedEast && connectedSouth)
                return 'F';

            if (connectedEast && connectedWest)
                return '-';

            if (connectedSouth && connectedWest)
                return '7';

            assert(false);
            return ' ';
        }

        void printPath(const std::vector<Point>& path) const
        {
            // For speed create a bool lookup instead of having to range over path each time:
            std::vector<std::vector<bool>> isOnPath(height, std::vector(width, false));

            for (auto p : path)
            {
                isOnPath[p.y][p.x] = true;
            }

            std::cout << '\n';
            for(size_t y{}; y < height; y++)
            {
                for(size_t x{}; x<width; x++)
                {
                    std::cout << (isOnPath[y][x] ? 'X' : 'O');
                }

                std::cout << '\n';
            }
        }

        void printPathAndEnclosed(const std::vector<Point>& path, const std::vector<Point>& enclosed) const
        {
            // For speed create a bool lookup instead of having to range over path each time:
            std::vector<std::vector<bool>> isOnPath(height, std::vector(width, false));
            std::vector<std::vector<bool>> isEnclosed(height, std::vector(width, false));

            for (auto p : path)
            {
                isOnPath[p.y][p.x] = true;
            }

            for (auto p : enclosed)
            {
                isEnclosed[p.y][p.x] = true;
            }

            std::cout << '\n';
            for (size_t y{}; y < height; y++)
            {
                for (size_t x{}; x < width; x++)
                {
                    std::cout << (isOnPath[y][x] ? 'X' : (isEnclosed[y][x] ? '|' : 'O'));
                }

                std::cout << '\n';
            }
        }
    };

    PipeMap parseInput(std::ifstream& file)
    {
        std::vector<std::string> lines{};
        while(!file.eof())
        {
            std::string line;
            std::getline(file, line);
            lines.push_back(line);
        }

        return PipeMap{ lines };
    }

    long long solvePart1(std::ifstream& file)
    {
        const auto map{ parseInput(file) };
        const auto path{ map.getPathFromStartingPoint() };
        return static_cast<long long>(path.size()) / 2;
    }

    long long solvePart2(std::ifstream& file)
    {
        const auto map{ parseInput(file) };
        const auto path{ map.getPathFromStartingPoint() };

#ifdef debug
        map.printPath(path);
#endif

        // Since the problem doesn't count 'double' enclosed spaces, we can use the following rule:
        // a point is enclosed if it's not on the path & a straight line from it to an edge has an odd number of points on the path
        // (which edge doesn't matter, so just always go straight up)

        // For speed create a bool lookup instead of having to range over path each time:
        std::vector<std::vector<bool>> isOnPath(map.height, std::vector(map.width, false));

        for (auto p : path)
        {
            isOnPath[p.y][p.x] = true;
        }

        long long nrEnclosed{};
#ifdef debug
        std::vector<Point> enclosedPoints;
#endif
        for (size_t y{}; y < map.height; y++)
        {
            for (size_t x{}; x < map.width; x++)
            {
                if (isOnPath[y][x])
                {
                    continue;
                }

                std::vector<Point> intersectPoints{};
                Point p{ x, y };
                while (p.y > 0)
                {
                    p = Point{ p.x, p.y - 1 };
                    if (isOnPath[p.y][p.x])
                    {
                        intersectPoints.push_back(p);
                    }
                }

                // An 'real' intersect is only when the path 'crosses' the line up,
                // meaning it starts from the right and exits to left (or reverse)
                if (intersectPoints.empty())
                {
                    continue;
                }

                int nrOfRealIntersects{};
                bool enteredFromLeft{ false };
                for(size_t i{}; i < intersectPoints.size(); i ++)
                {
                    char c{ map.getRealPipeAtPoint(intersectPoints[i], path) };

                    // Shouldn't happen since we loop through connected sections below and intersection can't start with it
                    assert(c != '|');

                    if (c == '-')
                    {
                        // always a real intersection
                        nrOfRealIntersects++;
                        continue;
                    }

                    if (c == 'J' || c == '7')
                    {
                        enteredFromLeft = true;
                    }

                    if (c == 'F' || c == 'L')
                    {
                        enteredFromLeft = false;
                    }

                    // To get to exit point of intersection between line & path, go up as long as current point has a connection to top
                    while (c == '|' || c == 'L' || c == 'J')
                    {
                        i++;
                        c = map.getRealPipeAtPoint(intersectPoints[i], path);
                    }

                    // Shouldn't happen since we loop through connected sections below;
                    assert(c != '|');

                    if (c == 'J' || c == '7')
                    {
                        if (!enteredFromLeft)
                        {
                            nrOfRealIntersects++;
                        }
                    }

                    if (c == 'F' || c == 'L')
                    {
                        if (enteredFromLeft)
                        {
                            nrOfRealIntersects++;
                        }
                    }
                }

                if (nrOfRealIntersects % 2 == 1)
                {
#ifdef debug
                    enclosedPoints.push_back(Point{ x, y });
#endif
                    nrEnclosed++;
                }
            }
        }

#ifdef debug
        map.printPathAndEnclosed(path, enclosedPoints);
#endif

        return nrEnclosed;
    }
}