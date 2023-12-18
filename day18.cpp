#include "day18.h"
#include "helpers.h"

#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <ranges>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

namespace day18
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 18 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day18_example.txt" : "inputs/day18_real.txt" };
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

    Direction flipDirection(const Direction d)
    {
        switch (d) {
        case Direction::north: return Direction::south;
        case Direction::east: return Direction::west;
        case Direction::south: return Direction::north;
        case Direction::west: return Direction::east;
        }

        assert(false);
        return {};
    }

    struct Point
    {
        int x{};
        int y{};

        Point operator+(const Point& other) const
        {
            return Point{
            x + other.x,
            y + other.y
            };
        }

        bool operator==(const Point& other) const
        {
            return x == other.x && y == other.y;
        }

        // Bad, but serviceable hash for here
        [[nodiscard]] size_t hash() const
        {
            return (x * 997 + y) % 7919;
        }
    };

    struct PointHash
    {
        size_t operator()(const Point& point) const
        {
            return point.hash();
        }
    };

    struct TrenchPoint
    {
        Point p{};
        std::string colorCode{};
        Direction dirOfPrevPoint{};
        Direction dirOfNextPoint{};
    };
    
    // Returns an offset point for the given direction
    Point offsetByDir(const Direction d)
    {
        switch (d) {
        case Direction::north:
            return Point{ 0, -1 };

        case Direction::east:
            return Point{ 1, 0 };

        case Direction::south:
            return Point{ 0, 1 };

        case Direction::west:
            return Point{ -1, 0 };
        }

        assert(false);
        return {};
    }

    struct Instruction
    {
        Direction dir{};
        int dist{};
        std::string colorCode{};
    };

    struct Input
    {
        std::vector<Instruction> instructions;

        Input(std::ifstream& file)
        {
            instructions = std::vector<Instruction>{};
            while (!file.eof())
            {
                std::string line;
                std::getline(file, line);

                std::stringstream ss{};
                ss << line;

                std::string dirSection;
                ss >> dirSection;
                Direction d{};
                switch (dirSection[0])
                {
                case 'R':
                    d = Direction::east;
                    break;

                case 'D':
                    d = Direction::south;
                    break;

                case 'L':
                    d = Direction::west;
                    break;

                case 'U':
                    d = Direction::north;
                    break;

                default:
                    assert(false);
                }

                int dist;
                ss >> dist;

                std::string colorCode;
                ss >> colorCode;

                instructions.push_back(Instruction{
                    d,
                    dist,
                    colorCode
                    });
            }
        }
    };

    struct Trench
    {
        std::vector<TrenchPoint> points{};
        std::vector<std::vector<bool>> dugOut{};

        // Performance of map here not super impressive, maybe better as 2d array lookup?
        std::unordered_map<Point, TrenchPoint, PointHash> trenchLookup{};
        int minX{};
        int minY{};
        int maxX{};
        int maxY{};

        // Runs along the trench instructions.
        // Note: tested & path does not cross itself for input, and ends back at 0,0
        void digTrench(const Input& input)
        {
            Point curPoint{};

            for (const auto& instruction : input.instructions)
            {
                auto offset{ offsetByDir(instruction.dir) };
                for (int i = 0; i < instruction.dist; i++)
                {
                    if (!points.empty())
                    {
                        points[points.size() - 1].dirOfNextPoint = instruction.dir;
                    }

                    curPoint = curPoint + offset;
                    points.push_back(TrenchPoint{curPoint, instruction.colorCode, flipDirection(instruction.dir)});
                }
            }

            points[points.size() - 1].dirOfNextPoint = input.instructions[0].dir;

            for (const auto& point : points)
            {
                minX = std::min(minX, point.p.x);
                minY = std::min(minY, point.p.y);
                maxX = std::max(maxX, point.p.x);
                maxY = std::max(maxY, point.p.y);

                trenchLookup[point.p] = point;
            }
        }

        [[nodiscard]] Point convertAbsoluteXyToRelativePoint(const size_t x, const size_t y) const
        {
            return Point{ static_cast<int>(x) + minX, static_cast<int>(y) + minY };
        }

        int determineNumberOfCrossingsFromRelativePoint(Point relPoint)
        {
            int nrOfCrossings{};
            while (relPoint.y > minY)
            {
                relPoint.y--;

                if (!trenchLookup.contains(relPoint))
                {
                    // Not an intersection at all
                    continue;
                }

                TrenchPoint intersect{ trenchLookup[relPoint] };

                if (intersect.dirOfNextPoint != Direction::north && intersect.dirOfPrevPoint != Direction::north)
                {
                    // Intersection in single point (going west -> east or west <- east)
                    nrOfCrossings++;
                    continue;
                }

                const bool currentlyCrossingFromEast = intersect.dirOfPrevPoint == Direction::north
                    ? intersect.dirOfNextPoint == Direction::east
                    : intersect.dirOfPrevPoint == Direction::east;

                // Start of an upwards going section
                while (intersect.dirOfNextPoint == Direction::north || intersect.dirOfPrevPoint == Direction::north)
                {
                    // Don't need to check versus minY since trench will never go off map.
                    relPoint.y--;
                    intersect = trenchLookup[relPoint];
                }

                // Now at intersection exit point
                const Direction exitDirection = intersect.dirOfPrevPoint == Direction::south
                    ? intersect.dirOfNextPoint
                    : intersect.dirOfPrevPoint;

                if ((currentlyCrossingFromEast && exitDirection == Direction::west)
                    || (!currentlyCrossingFromEast && exitDirection == Direction::east))
                {
                    // Was a real intersection
                    nrOfCrossings++;
                }

                // Otherwise was 'fake' intersection (entering from east and exiting to east or vice versa).
            }

            return nrOfCrossings;
        }

        // Fills dugOut property based on points on trench
        void digOut()
        {

            const size_t height = static_cast<size_t>(maxY - minY) + 1;
            const size_t width = static_cast<size_t>(maxX - minX) + 1;
            dugOut = std::vector(height, std::vector(width, false));

            for(size_t y = 0; y < height; y++)
            {
                for(size_t x = 0; x < width; x++)
                {
                    Point relPoint{ convertAbsoluteXyToRelativePoint(x, y) };
                    if (trenchLookup.contains(relPoint))
                    {
                        // Part of the trench itself
                        dugOut[y][x] = true;
                        continue;
                    }

                    // Uses logic where point is inside if straight line to north crosses the trench an odd number of times.
                    // Same as day 10 part 2.
                    const auto nrOfCrossings{ determineNumberOfCrossingsFromRelativePoint(relPoint) };
                    if (nrOfCrossings % 2 == 1)
                    {
                        dugOut[y][x] = true;
                    }
                }
            }
        }

        long long digScore()
        {
            long long result{};

            for (int y = 0; y <= maxY - minY; y++)
            {
                for (int x = 0; x <= maxX - minX; x++)
                {
                    if (dugOut[static_cast<size_t>(y)][static_cast<size_t>(x)])
                    {
                        result++;
                    }
                }
            }

            return result;
        }

        void printOutline() const
        {
            std::cout << '\n';

            for(int y = minY; y <= maxY; y++)
            {
                for(int x = minX; x <= maxX; x++)
                {
                    if (trenchLookup.contains(Point{x, y}))
                    {
                        std::cout << '#';
                    }
                    else
                    {
                        std::cout << '.';
                    }
                }

                std::cout << '\n';
            }
        }

        void printDugOut() const
        {
            std::cout << '\n';

            for (int y = 0; y <= maxY - minY; y++)
            {
                for (int x = 0; x <= maxX - minX; x++)
                {
                    if (dugOut[static_cast<size_t>(y)][static_cast<size_t>(x)])
                    {
                        std::cout << '#';
                    }
                    else
                    {
                        std::cout << '.';
                    }
                }

                std::cout << '\n';
            }
        }
    };

    long long solvePart1(std::ifstream& file)
    {
        const Input input(file);
        Trench trench{};
        trench.digTrench(input);
        // trench.printOutline();

        trench.digOut();
        // trench.printDugOut();
        return trench.digScore();
    }

    long long solvePart2(std::ifstream& file)
    {
        return 0;
    }
}