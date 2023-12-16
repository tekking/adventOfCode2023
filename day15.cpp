#include "day15.h"
#include "helpers.h"

#include <cassert>
#include <fstream>
#include <list>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

namespace day15
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 15 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day15_example.txt" : "inputs/day15_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    long long hashString(std::string_view string)
    {
        long long result{};
        for (const char c : string)
        {
            result += c;
            result *= 17;
            result %= 256;
        }

        return result;
    }

    struct Lens
    {
        int focalLength{};
        std::string label{};

        bool operator==(const Lens& other) const
        {
            return label == other.label;
        }
    };

    struct Box
    {
        int index{};
        std::list<Lens> lenses{};

        long long score()
        {
            long long sum{};
            int lensNr{};
            for (auto& [focalLength, label] : lenses)
            {
                lensNr++;
                sum += (index + 1LL) * lensNr * focalLength;
            }

            return sum;
        }

        void addLens(const Lens& l)
        {
            for (auto& lens : lenses)
            {
                if (lens.label == l.label)
                {
                    lens.focalLength = l.focalLength;
                    return;
                }
            }

            lenses.push_back(l);
        }

        void removeLensByLabel(std::string_view label)
        {
            lenses.remove_if([label](const Lens l) {return l.label == label; });
        }
    };

    long long solvePart1(std::ifstream& file)
    {
        std::string input;
        std::getline(file, input);

        const auto instructions = splitStringBySeperator(input, ',');

        long long sum{};
        for (const auto& instruction : instructions)
        {
            sum += hashString(instruction);
        }

        return sum;
    }

    long long solvePart2(std::ifstream& file)
    {
        std::string input;
        std::getline(file, input);

        const auto instructions = splitStringBySeperator(input, ',');
        std::vector<Box> boxes{};
        for(int i = 0; i<256;i++)
        {
            boxes.push_back(Box{ i });
        }

        for (const auto& instruction : instructions)
        {
            std::string::size_type index;
            if ((index = instruction.find('-')) != std::string::npos)
            {
                const std::string label{ instruction.substr(0, index) };
                const auto hash{ hashString(label) };
                boxes[static_cast<size_t>(hash)].removeLensByLabel(label);
            }
            else
            {
                index = instruction.find('=');
                const std::string label{ instruction.substr(0, index) };
                const auto hash{ hashString(label) };

                const int focalLength = instruction[instruction.size() - 1] - '0';
                boxes[static_cast<size_t>(hash)].addLens(Lens{ focalLength, label });
            }
        }

        long long sum{};
        for (auto box : boxes)
        {
            sum += box.score();
        }

        return sum;
    }
}