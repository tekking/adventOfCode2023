#include "day16.h"
#include "helpers.h"

#include <array>
#include <cassert>
#include <fstream>
#include <list>
#include <iostream>
#include <ranges>
#include <queue>
#include <string>
#include <vector>

namespace day16
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 16 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day16_example.txt" : "inputs/day16_real.txt" };
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


    struct Beam
    {
        size_t x;
        size_t y;
        Direction dir;

        void moveStep()
        {
            switch (dir) {
            case Direction::north:
                y--;
                break;
            case Direction::east:
                x++;
                break;
            case Direction::south:
                y++;
                break;
            case Direction::west:
                x--;
                break;
            }
        }

        void updateDirWithCorner(const char corner)
        {
            if (corner == '\\')
            {
                switch (dir) {
                case Direction::north:
                    dir = Direction::west;
                    break;
                case Direction::east:
                    dir = Direction::south;
                    break;
                case Direction::south:
                    dir = Direction::east;
                    break;
                case Direction::west:
                    dir = Direction::north;
                    break;
                }
            }
            else
            {
                switch (dir) {
                case Direction::north:
                    dir = Direction::east;
                    break;
                case Direction::east:
                    dir = Direction::north;
                    break;
                case Direction::south:
                    dir = Direction::west;
                    break;
                case Direction::west:
                    dir = Direction::south;
                    break;
                }
            }
        }
    };

    struct Field
    {
        std::vector<std::string> field{};
        // Interesting note: for some reasone writing to this triple nested array
        // is much faster than updating a vector<vector<bool>> (used to have seperate energized variable).

        std::vector<std::vector<std::array<bool, 4>>> directionPassed;
        size_t width;
        size_t height;

        explicit Field(const std::vector<std::string>& f)
        {
            field = f;
            height = f.size();
            width = f[0].size();
            resetField();
        }

        void fireBeam(size_t enterX, size_t enterY, Direction enterDir)
        {
            // Offset by one (reversed) step, since loops moves before marking
            switch (enterDir) {
            case Direction::north:
                enterY++;
                break;
            case Direction::east:
                enterX--;
                break;
            case Direction::south:
                enterY--;
                break;
            case Direction::west:
                enterX++;
                break;
            }

            std::queue<Beam> beamQueue{};
            beamQueue.push(Beam{ enterX, enterY, enterDir });
            while (!beamQueue.empty())
            {
                Beam& b = beamQueue.front();

                while(true)
                {
                    b.moveStep();
                    // Use overflow to also monitor for going off low end
                    if (b.x >= width || b.y >= height)
                    {
                        break;
                    }

                    // loop detection
                    if (directionPassed[b.y][b.x][static_cast<unsigned int>(b.dir)])
                    {
                        break;
                    }

                    directionPassed[b.y][b.x][static_cast<unsigned int>(b.dir)] = true;

                    const char c{ field[b.y][b.x] };

                    if (c == '.')
                    {
                        continue;
                    }

                    if (c == '|')
                    {
                        if (b.dir == Direction::north || b.dir == Direction::south)
                        {
                            continue;
                        }

                        // Replace b with beams going up & down:
                        b.dir = Direction::north;
                        beamQueue.push(Beam{ b.x, b.y, Direction::south });
                        continue;
                    }

                    if (c == '-')
                    {
                        if (b.dir == Direction::west || b.dir == Direction::east)
                        {
                            continue;
                        }

                        // Replace b with beams going up & down:
                        b.dir = Direction::west;
                        beamQueue.push(Beam{ b.x, b.y, Direction::east });
                        continue;
                    }

                    b.updateDirWithCorner(c);
                }

                beamQueue.pop();
            }
        }

        void resetField()
        {
            directionPassed = std::vector(height, std::vector(width, std::array<bool, 4>{}));
        }

        void printEnergized() const
        {
            std::cout << '\n';
            for (const auto& row : directionPassed)
            {
                for (const auto b : row)
                {
                    if (std::ranges::any_of(b, [](bool b){return b;}))
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

        [[nodiscard]] long long score() const
        {
            long long sum{};
            for (const auto& row : directionPassed)
            {
                for (const auto b : row)
                {
                    for (size_t i = 0; i < 4; i++)
                    {
                        if (b[i])
                        {
                            sum++;
                            break;
                        }
                    }
                }
            }

            return sum;
        }
    };

    Field parseInput(std::ifstream& file)
    {
        std::vector<std::string> field{};
        while(!file.eof())
        {
            std::string line;
            std::getline(file, line);
            field.push_back(line);
        }

        return Field( field );
    }

    long long solvePart1(std::ifstream& file)
    {
        auto field{ parseInput(file) };
        field.fireBeam(0, 0, Direction::east);
        // field.printEnergized();
        return field.score();
    }

    long long solvePart2(std::ifstream& file)
    {
        auto field{ parseInput(file) };
        long long maxScore{};
        // Loop over left & right side:
        for (size_t y = 0; y < field.height; y++)
        {
            field.resetField();
            field.fireBeam(0, y, Direction::east);
            maxScore = std::max(maxScore, field.score());

            field.resetField();
            field.fireBeam(field.width - 1, y, Direction::west);
            maxScore = std::max(maxScore, field.score());
        }

        // Loop over top & bottom side:
        for (size_t x = 0; x < field.width ; x++)
        {
            field.resetField();
            field.fireBeam(x, 0, Direction::south);
            maxScore = std::max(maxScore, field.score());

            field.resetField();
            field.fireBeam(x, field.height - 1, Direction::north);
            maxScore = std::max(maxScore, field.score());
        }

        return maxScore;
    }
}