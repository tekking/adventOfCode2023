#include "day22.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <ranges>
#include <string>
#include <vector>

namespace day22
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 22 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day22_example.txt" : "inputs/day22_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct Point
    {
        size_t x{};
        size_t y{};
        size_t z{};

        [[nodiscard]] bool operator==(const Point& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    enum class Alignment
    {
        xAligned,
        yAligned,
        zAligned,
        singleBlock
    };

    struct Brick
    {
        Point endA{};
        Point endB{};
        int originalIndex{};
        std::vector<Point> coordinatesContained{};

        Brick(const Point& a, const Point& b, int index)
        {
            endA = a;
            endB = b;
            originalIndex = index;
            initializeOccupiedCoordinates();
        }

        [[nodiscard]] size_t lowestZ() const
        {
            return std::min(endA.z, endB.z);
        }

        [[nodiscard]] bool operator==(const Brick& other) const
        {
            return originalIndex == other.originalIndex;
        }

        void dropOne()
        {
            endA.z--;
            endB.z--;

            for (Point& point : coordinatesContained)
            {
                point.z--;
            }
        }

        void raiseOne()
        {
            endA.z++;
            endB.z++;

            for (Point& point : coordinatesContained)
            {
                point.z++;
            }
        }

        [[nodiscard]] bool intersects(const std::vector<std::vector<std::vector<bool>>>& occupiedCoordinatesLookup) const
        {
            for (const auto& point : coordinatesContained)
            {
                if (occupiedCoordinatesLookup[point.x][point.y][point.z])
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] bool intersects(const std::vector<std::vector<std::vector<int>>>& occupiedCoordinatesByBrickLookup) const
        {
            for (const auto& point : coordinatesContained)
            {
                const auto occupiedBy{ occupiedCoordinatesByBrickLookup[point.x][point.y][point.z] };
                if (occupiedBy >= 0 && occupiedBy != originalIndex)
                {
                    return true;
                }
            }

            return false;
        }

        void initializeOccupiedCoordinates()
        {
            if (endA.x != endB.x)
            {
                for (size_t x{ std::min(endA.x, endB.x) }; x <= std::max(endA.x, endB.x); x++)
                {
                    coordinatesContained.emplace_back(x, endA.y, endA.z);
                }
            }
            else if (endA.y != endB.y)
            {
                for (size_t y{ std::min(endA.y, endB.y) }; y <= std::max(endA.y, endB.y); y++)
                {
                    coordinatesContained.emplace_back(endA.x, y, endA.z);
                }
            }
            else if (endA.z != endB.z)
            {
                for (size_t z{ std::min(endA.z, endB.z) }; z <= std::max(endA.z, endB.z); z++)
                {
                    coordinatesContained.emplace_back(endA.x, endA.y, z);
                }
            }
            else
            {
                coordinatesContained.push_back(endA);
            }
        }

        [[nodiscard]] const std::vector<Point>& getCoordinatesOccupied() const
        {
            return coordinatesContained;
        }
    };

    struct Puzzle
    {
        std::vector<Brick> fallingBricks{};

        std::vector<Brick> restingBricks{};

        std::vector<std::vector<std::vector<bool>>> occupiedCoordinateLookup{};

        void parseInput(std::ifstream& file)
        {
            int index{};
            while(!file.eof())
            {
                std::string line;
                std::getline(file, line);

                const auto sepPos{ line.find('~') };
                const auto coordinatesA{ parseLineOfSymbolSeperatedNumbers(line.substr(0, sepPos)) };
                const auto coordinatesB{ parseLineOfSymbolSeperatedNumbers(line.substr(sepPos + 1, line.size() - sepPos - 1)) };

                fallingBricks.emplace_back(
                    Point{ static_cast<size_t>(coordinatesA[0]), static_cast<size_t>(coordinatesA[1]), static_cast<size_t>(coordinatesA[2]) },
                    Point{ static_cast<size_t>(coordinatesB[0]), static_cast<size_t>(coordinatesB[1]), static_cast<size_t>(coordinatesB[2]) },
                    index);

                index++;
            }
        }

        void dropBricks()
        {
            // Strategy is to drop bricks by increasing Z, since bricks are always straight lines, they should never come to rest on a brick
            // with equal or lower lowest z.
            std::ranges::sort(fallingBricks, [](const Brick& b1, const Brick& b2) {return b1.lowestZ() < b2.lowestZ(); });

            // Determine ranges for the occupied lookup
            size_t maxX{};
            size_t maxY{};
            size_t maxZ{};
            for (const auto& fallingBrick : fallingBricks)
            {
                maxX = std::max(std::max(fallingBrick.endA.x, fallingBrick.endB.x), maxX);
                maxY = std::max(std::max(fallingBrick.endA.y, fallingBrick.endB.y), maxY);
                maxZ = std::max(std::max(fallingBrick.endA.z, fallingBrick.endB.z), maxZ);
            }

            occupiedCoordinateLookup = std::vector(maxX + 1, std::vector(maxY + 1, std::vector(maxZ + 1, false)));

            for (auto fallingBrick : fallingBricks)
            {
                // Determine the resting location of the brick
                // Naive implementation for now, just drop by 1 until we intersect an existing resting brick or the ground

                // Loop until intersections (with blocks or ground)
                while(true)
                {
                    fallingBrick.dropOne();

                    if (fallingBrick.endA.z == 0 || fallingBrick.endB.z == 0)
                    {
                        // Can't fall below ground;
                        break;
                    }

                    if(fallingBrick.intersects(occupiedCoordinateLookup))
                    {
                        break;
                    }
                }

                // Raise back one before intersection
                fallingBrick.raiseOne();

                // Update list of resting blocks & occupied coordinates
                restingBricks.push_back(fallingBrick);
                for (const auto& [x, y, z] : fallingBrick.getCoordinatesOccupied())
                {
                    occupiedCoordinateLookup[x][y][z] = true;
                }
            }
        }

        [[nodiscard]] bool brickIsSafeToRemove(const Brick& b)
        {
            // Naive implementation that reruns dropping algorithm for all bricks after removal
            // Determine ranges for the occupied lookup
            size_t maxX{};
            size_t maxY{};
            size_t maxZ{};
            for (const auto& fallingBrick : fallingBricks)
            {
                maxX = std::max(std::max(fallingBrick.endA.x, fallingBrick.endB.x), maxX);
                maxY = std::max(std::max(fallingBrick.endA.y, fallingBrick.endB.y), maxY);
                maxZ = std::max(std::max(fallingBrick.endA.z, fallingBrick.endB.z), maxZ);
            }

            // Keep track of which blocks occupies each coordinate (if any)
            auto occupiedByBrick = std::vector(maxX + 1, std::vector(maxY + 1, std::vector(maxZ + 1, -1)));

            // Insert all but the given brick
            for (const auto& restingBrick : restingBricks)
            {
                if (restingBrick == b)
                {
                    continue;
                }

                for (const auto& [x, y, z] : restingBrick.getCoordinatesOccupied())
                {
                    occupiedByBrick[x][y][z] = restingBrick.originalIndex;
                }
            }

            // Check if any brick now can drop
            for (auto& restingBrick : restingBricks)
            {
                restingBrick.dropOne();

                if (restingBrick.endA.z == 0 || restingBrick.endB.z == 0)
                {
                    // Can't fall below ground;
                    restingBrick.raiseOne();
                    continue;
                }

                if (restingBrick.intersects(occupiedByBrick))
                {
                    restingBrick.raiseOne();
                    continue;
                }

                // Brick can drop!
                restingBrick.raiseOne();
                return false;
            }

            return true;
        }

        [[nodiscard]] long long determineNumberOfBricksDroppedByRemoval(const Brick& b)
        {
            // Naive implementation that reruns dropping algorithm for all bricks after removal
            // Determine ranges for the occupied lookup
            size_t maxX{};
            size_t maxY{};
            size_t maxZ{};
            for (const auto& fallingBrick : fallingBricks)
            {
                maxX = std::max(std::max(fallingBrick.endA.x, fallingBrick.endB.x), maxX);
                maxY = std::max(std::max(fallingBrick.endA.y, fallingBrick.endB.y), maxY);
                maxZ = std::max(std::max(fallingBrick.endA.z, fallingBrick.endB.z), maxZ);
            }

            // Keep track of which blocks occupies each coordinate (if any)
            auto occupiedLookup = std::vector(maxX + 1, std::vector(maxY + 1, std::vector(maxZ + 1, false)));

            // Sort the resting bricks
            std::ranges::sort(restingBricks, [](const Brick& b1, const Brick& b2) {return b1.lowestZ() < b2.lowestZ(); });

            // Check if any brick now can drop more than it's current position
            long long nrDropped{};
            for (Brick restingBrick : restingBricks)
            {
                if (restingBrick == b)
                {
                    // Remove the named block
                    continue;
                }

                bool hasDropped{};

                while (true)
                {
                    restingBrick.dropOne();

                    if (restingBrick.endA.z == 0 || restingBrick.endB.z == 0)
                    {
                        // Can't fall below ground;
                        break;
                    }

                    if (restingBrick.intersects(occupiedLookup))
                    {
                        break;
                    }

                    // Succeeded at dropping at least once
                    hasDropped = true;
                }

                if (hasDropped)
                {
                    nrDropped++;
                }

                // Raise back one before intersection
                restingBrick.raiseOne();

                // Update occupied coordinates
                for (const auto& [x, y, z] : restingBrick.getCoordinatesOccupied())
                {
                    occupiedLookup[x][y][z] = true;
                }
            }

            return nrDropped;
        }

        [[nodiscard]] long long determineNumberOfSafeBricksToDrop()
        {
            long long nr{};
            for (const auto& restingBrick : restingBricks)
            {
                if(brickIsSafeToRemove(restingBrick))
                {
                    nr++;
                }
            }

            return nr;
        }

        [[nodiscard]] long long determineNumberOfTotalBricksDropped()
        {

            long long nr{};
            for (const auto& restingBrick : restingBricks)
            {
                nr += determineNumberOfBricksDroppedByRemoval(restingBrick);
            }

            return nr;
        }
    };

    long long solvePart1(std::ifstream& file)
    {
        Puzzle puzzle{};
        puzzle.parseInput(file);
        puzzle.dropBricks();
        return puzzle.determineNumberOfSafeBricksToDrop();
    }

    long long solvePart2(std::ifstream& file)
    {
        Puzzle puzzle{};
        puzzle.parseInput(file);
        puzzle.dropBricks();
        return puzzle.determineNumberOfTotalBricksDropped();
    }
}