#include "day18.h"
#include "helpers.h"

#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <ranges>
#include <set>
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
            return static_cast<size_t>(x * 997 + y) % 7919;
        }
    };

    struct Instruction
    {
        Direction dir{};
        int dist{};
        std::string colorCode{};
    };

    struct Input
    {
        std::vector<Instruction> instructions;

        static Instruction parseHexInstruction(const std::string& hexInstruction)
        {
            const auto distString{ hexInstruction.substr(0, 5) };

            std::stringstream ss{};
            ss << std::hex << distString;

            int dist;
            ss >> dist;

            Direction d{};
            switch (hexInstruction[5])
            {
            case '0':
                d = Direction::east;
                break;

            case '1':
                d = Direction::south;
                break;

            case '2':
                d = Direction::west;
                break;

            case '3':
                d = Direction::north;
                break;

            default:
                assert(false);
            }

            return Instruction{ d, dist };
        }

        void loadPart1Input(std::ifstream& file)
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

        void loadPart2Input(std::ifstream& file)
        {
            instructions = std::vector<Instruction>{};
            while (!file.eof())
            {
                std::string line;
                std::getline(file, line);

                std::stringstream ss{};
                ss << line;

                std::string colorCode;
                ss >> colorCode >> colorCode >> colorCode;
                colorCode = colorCode.substr(2, colorCode.size() - 3);

                instructions.push_back(parseHexInstruction(colorCode));
            }
        }
    };

    struct HorizontalLineSection
    {
        long long lowX{};
        long long highX{};
        long long y{};

        [[nodiscard]] bool operator<(const HorizontalLineSection& other) const
        {
            return y < other.y;
        }
    };

    struct VerticalLineSection
    {
        long long x{};
        long long lowY{};
        long long highY{};
    };

    struct HorizontalOverlap
    {
        long long lowX{};
        long long highX{};
        std::set<HorizontalLineSection> originalIntersectSections{};
    };

    // Takes a set of not overlapping (with each other) overlap sections, each of which keeps track of the y values it represents in the overlap
    // and a new linesection with it's y value. Returns a new list of not-overlapping overlap sections, updated with the line section.
    std::vector<HorizontalOverlap> expandOverlapsWithLineSection(const std::vector<HorizontalOverlap>& overlaps, const HorizontalLineSection lineSection)
    {
        std::vector<HorizontalLineSection> notIntersectedYet{lineSection};
        std::vector<HorizontalOverlap> result{};

        for (const auto& overlap : overlaps)
        {
            if (overlap.highX < lineSection.lowX
                || overlap.lowX > lineSection.highX)
            {
                // No overlap at all
                result.push_back(overlap);
                continue;
            }

            // Handle the update of overlaps:
            if (overlap.highX <= lineSection.highX
                && overlap.lowX >= lineSection.lowX)
            {
                // Easy case, fully contained in new line section, so update full overlap:
                HorizontalOverlap newOverlap{ overlap.lowX, overlap.highX, overlap.originalIntersectSections };
                newOverlap.originalIntersectSections.insert(lineSection);
                result.push_back(newOverlap);
            }
            else
            {
                // Split into
                // 1. 'below' part
                // 2. 'within' part

                if (overlap.lowX < lineSection.lowX)
                {
                    HorizontalOverlap belowOverlap{ overlap.lowX, lineSection.lowX - 1, overlap.originalIntersectSections };
                    result.push_back(belowOverlap);
                }

                HorizontalOverlap withinOverlap{
                    std::max(overlap.lowX, lineSection.lowX),
                    std::min(overlap.highX, lineSection.highX),
                    overlap.originalIntersectSections
                };
                withinOverlap.originalIntersectSections.insert(lineSection);
                result.push_back(withinOverlap);

                if (overlap.highX > lineSection.highX)
                {
                    HorizontalOverlap aboveOverlap{ lineSection.highX + 1, overlap.highX, overlap.originalIntersectSections };
                    result.push_back(aboveOverlap);
                }
            }

            // Handle the update over not-intersected segments
            std::vector<HorizontalLineSection> newNotIntersectedYet;
            for (const auto& notIntersectedSection : notIntersectedYet)
            {
                // Split into
                // 1. 'below' part
                // 2. 'within' part (which is not added here)
                // 3. 'above' part
                if (notIntersectedSection.lowX < overlap.lowX)
                {
                    HorizontalLineSection belowSection{
                        notIntersectedSection.lowX,
                        std::min(notIntersectedSection.highX, overlap.lowX - 1),
                        notIntersectedSection.y
                    };
                    newNotIntersectedYet.push_back(belowSection);
                }

                if (notIntersectedSection.highX > overlap.highX)
                {
                    HorizontalLineSection aboveSection{
                        std::max(notIntersectedSection.lowX, overlap.highX + 1),
                        notIntersectedSection.highX,
                        notIntersectedSection.y
                    };
                    newNotIntersectedYet.push_back(aboveSection);
                }
            }

            notIntersectedYet = newNotIntersectedYet;
        }

        // Add whatever has not been added to any intersections yet to it's own new overlap section:
        for (const auto& notIntersectedSection : notIntersectedYet)
        {
            HorizontalOverlap newOverlap{ notIntersectedSection.lowX, notIntersectedSection.highX, std::set<HorizontalLineSection> {lineSection} };
            result.push_back(newOverlap);
        }

        return result;
    }

    struct Part2Trench
    {
        std::vector<HorizontalOverlap> overlapSections{};
        std::vector<VerticalLineSection> verticalSections{};

        void determineOverlapAndSectionsForInput(const Input& input)
        {
            overlapSections = std::vector<HorizontalOverlap>{};

            Point currentPos{};
            for (const auto& inputLine : input.instructions)
            {

                // Only update overlap for horizontal sections but keep track of vertical sections.
                if (inputLine.dir == Direction::east || inputLine.dir == Direction::west)
                {
                    const int newX{ inputLine.dir == Direction::east ? currentPos.x + inputLine.dist : currentPos.x - inputLine.dist };
                    const int lowX{ std::min(currentPos.x, newX) };
                    const int highX{ std::max(currentPos.x, newX) };

                    const HorizontalLineSection section{ lowX, highX, currentPos.y };
                    overlapSections = expandOverlapsWithLineSection(overlapSections, section);
                }
                else
                {
                    const int newY{ inputLine.dir == Direction::south ? currentPos.y + inputLine.dist : currentPos.y - inputLine.dist };
                    const int lowY{ std::min(currentPos.y, newY) };
                    const int highY{ std::max(currentPos.y, newY) };
                    const VerticalLineSection section{ currentPos.x, lowY, highY };
                    verticalSections.push_back(section);
                }

                // Update currentPos:
                switch (inputLine.dir) {
                case Direction::north:
                    currentPos.y -= inputLine.dist;
                    break;
                case Direction::east:
                    currentPos.x += inputLine.dist;
                    break;
                case Direction::south:
                    currentPos.y += inputLine.dist;
                    break;
                case Direction::west:
                    currentPos.x -= inputLine.dist;
                    break;
                }
            }
        }

        // Uses the overlap sections to determine the total space
        [[nodiscard]] long long scoreOnSections() const
        {
            long long sum{};

            for (const auto& overlapSection : overlapSections)
            {
                // Loop over the overlapping sections going down on y, keeping track if we are currently inside or outside:
                const long long widthOfOverlap{ overlapSection.highX - overlapSection.lowX + 1 };

                if (widthOfOverlap > 1)
                {
                    // Simple case, always flips inside/outside on each intersection, just need to sometimes count one or two
                    // vertical lines on the outside edge
                    bool insideArea{ false };
                    long long lastY{};
                    
                    for (const auto& originalIntersectSection : overlapSection.originalIntersectSections)
                    {
                        if (insideArea)
                        {
                            // Add the height times width to the total
                            sum += (originalIntersectSection.y - lastY + 1) * widthOfOverlap;
                        }
                        else
                        {
                            auto lowXVerticalSection = std::ranges::find_if(verticalSections, [overlapSection, originalIntersectSection](VerticalLineSection section)
                                {
                                    return section.x == overlapSection.lowX && section.highY == originalIntersectSection.y;
                                });

                            auto highXVerticalSection = std::ranges::find_if(verticalSections, [overlapSection, originalIntersectSection](VerticalLineSection section)
                                {
                                    return section.x == overlapSection.highX && section.highY == originalIntersectSection.y;
                                });

                            if (lowXVerticalSection != verticalSections.end())
                            {
                                sum += lowXVerticalSection->highY - lowXVerticalSection->lowY - 1;
                            }

                            if (highXVerticalSection != verticalSections.end())
                            {
                                sum += highXVerticalSection->highY - highXVerticalSection->lowY - 1;
                            }
                        }

                        lastY = originalIntersectSection.y;

                        // Update inside/outside status
                        insideArea = !insideArea;
                    }

                    assert(!insideArea);

                }
                else
                {
                    // More complicated scenario where vertical edges can exit to a different side than the previous edge entered from.
                    bool insideArea{ false };
                    bool onVerticalEdge{ false };
                    bool edgeEnteredFromEast{};
                    long long lastY{};

                    for (const auto& originalIntersectSection : overlapSection.originalIntersectSections)
                    {
                        if (insideArea)
                        {
                            // Add the height to the total
                            sum += originalIntersectSection.y - lastY + 1;
                        }
                        else if (onVerticalEdge)
                        {
                            // Add only the height of the edge
                            sum += (originalIntersectSection.y - lastY + 1);
                        }

                        lastY = originalIntersectSection.y;

                        // Update status of inside & on edge (note overlapSection.lowX == highX):
                        if (originalIntersectSection.lowX == overlapSection.lowX
                            || originalIntersectSection.highX == overlapSection.lowX)
                        {
                            // Edge intersection
                            if (onVerticalEdge)
                            {
                                // Exiting an edge, need to determine if inside changes
                                onVerticalEdge = false;
                                const bool edgeExitedToEast{ originalIntersectSection.lowX == overlapSection.lowX };

                                if (edgeEnteredFromEast != edgeExitedToEast)
                                {
                                    // Entered/exited from different direction
                                    insideArea = !insideArea;
                                }

                                if (insideArea)
                                {
                                    // Substract 1 for the overlap of edge with upcoming inside section
                                    sum--;
                                }
                            }
                            else
                            {
                                // Starting an edge, doesn't change inside/outside status yet
                                onVerticalEdge = true;
                                edgeEnteredFromEast = originalIntersectSection.lowX == overlapSection.lowX;

                                if (insideArea)
                                {
                                    // Substract 1 for the overlap of edge with previous inside section
                                    sum--;
                                }
                            }
                        }
                        else
                        {
                            // Update inside/outside status
                            insideArea = !insideArea;
                        }
                    }

                    assert(!insideArea);
                }
            }

            return sum;
        }
    };

    long long solvePart1(std::ifstream& file)
    {
        Input input{};
        input.loadPart1Input(file);
        Part2Trench trench{};
        trench.determineOverlapAndSectionsForInput(input);
        return trench.scoreOnSections();
    }

    long long solvePart2(std::ifstream& file)
    {
        Input input{};
        input.loadPart2Input(file);
        Part2Trench trench{};
        trench.determineOverlapAndSectionsForInput(input);
        return trench.scoreOnSections();
    }

    void test_part2()
    {
        Input testInput{
            std::vector<Instruction>{
            Instruction{Direction::east, 2},
            Instruction{Direction::south, 2},
            Instruction{Direction::west, 2},
            Instruction{Direction::north, 2}}
        };

        Part2Trench trench{};
        trench.determineOverlapAndSectionsForInput(testInput);
        long long score{ trench.scoreOnSections() };
        assert(score == 9LL);
        
        testInput = Input{
            std::vector<Instruction>{
            Instruction{Direction::east, 4},
            Instruction{Direction::south, 4},
            Instruction{Direction::west, 1},
            Instruction{Direction::north, 2},
            Instruction{Direction::west, 2},
            Instruction{Direction::south, 2},
            Instruction{Direction::west, 1},
            Instruction{Direction::north, 4}}
        };
        trench = Part2Trench{};
        trench.determineOverlapAndSectionsForInput(testInput);
        score = trench.scoreOnSections();
        assert(score == 23LL);
        
        testInput = Input{
            std::vector<Instruction>{
            Instruction{Direction::east, 4},
            Instruction{Direction::south, 2},
            Instruction{Direction::west, 2},
            Instruction{Direction::south, 2},
            Instruction{Direction::east, 2},
            Instruction{Direction::south, 2},
            Instruction{Direction::west, 4},
            Instruction{Direction::north, 6}}
        };
        trench = Part2Trench{};
        trench.determineOverlapAndSectionsForInput(testInput);
        score = trench.scoreOnSections();
        assert(score == 33LL);
        
        testInput = Input{
            std::vector<Instruction>{
            Instruction{Direction::east, 2},
            Instruction{Direction::south, 1},
            Instruction{Direction::east, 3},
            Instruction{Direction::south, 3},
            Instruction{Direction::east, 1},
            Instruction{Direction::south, 1},
            Instruction{Direction::west, 1},
            Instruction{Direction::south, 1},
            Instruction{Direction::west, 2},
            Instruction{Direction::north, 1},
            Instruction{Direction::west, 2},
            Instruction{Direction::north, 1},
            Instruction{Direction::west, 1},
            Instruction{Direction::north, 5}}
        };
        trench = Part2Trench{};
        trench.determineOverlapAndSectionsForInput(testInput);
        score = trench.scoreOnSections();
        assert(score == 37LL);

        testInput = Input{
            std::vector<Instruction>{
            Instruction{Direction::east, 6},
            Instruction{Direction::south, 2},
            Instruction{Direction::west, 2},
            Instruction{Direction::south, 2},
            Instruction{Direction::east, 5},
            Instruction{Direction::south, 2},
            Instruction{Direction::west, 7},
            Instruction{Direction::south, 2},
            Instruction{Direction::east, 4},
            Instruction{Direction::south, 2},
            Instruction{Direction::west, 6},
            Instruction{Direction::north, 10}}
        };
        trench = Part2Trench{};
        trench.determineOverlapAndSectionsForInput(testInput);
        score = trench.scoreOnSections();
        assert(score == 80LL);
    }
}