#include "day02.h"

#include <charconv>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace day02
{
    // Ugly solutions below, but trying to avoid any type of list/array to 'stick' to progress in learn cpp :D

    constexpr int maxRed{ 12 };
    constexpr int maxGreen{ 13 };
    constexpr int maxBlue{ 14 };

    bool part1LineIsValid(const std::string_view line);
    int part2LinePower(std::string_view line);

    void run_day(bool example)
    {
        std::cout << "Running day 02 \n";

        const std::string fileName{ example ? "day02_example.txt" : "day02_real.txt" };
        std::ifstream file{ fileName };

        int sum{};
        int lineId{1};
        std::string line;
        while (std::getline(file, line))
        {
            if(part1LineIsValid(line))
            {
                sum += lineId;
            }

            lineId++;
        }

        std::cout << "Part 1 answer: " << sum << '\n';

        file.close();
        file.open(fileName);
        sum = 0;
        while (std::getline(file, line))
        {
            sum += part2LinePower(line);
        }

        std::cout << "Part 2 answer: " << sum << '\n';
    }

    bool singleColorRevealFits(std::string_view colorReveal)
    {
        if (colorReveal[0] == ' ')
        {
            colorReveal.remove_prefix(1);
        }

        const size_t spacePos{ colorReveal.find(' ') };
        int number;
        std::from_chars(colorReveal.data(), colorReveal.data() + colorReveal.size(), number);

        if (colorReveal[spacePos + 1] == 'b')
        {
            return number <= maxBlue;
        }

        if (colorReveal[spacePos + 1] == 'g')
        {
            return number <= maxGreen;
        }

        if (colorReveal[spacePos + 1] == 'r')
        {
            return number <= maxRed;
        }

        std::cout << "Found unexpected letter" << colorReveal[spacePos + 1];
        throw;
    }

    bool fullRevealFitsColors(std::string_view reveal)
    {
        bool valid{ true };

        // Loop over seperate color reveals
        size_t commaPos;
        while ((commaPos = reveal.find(',')) != std::string::npos)
        {
            const std::string_view colorReveal = reveal.substr(0, commaPos);
            valid &= singleColorRevealFits(colorReveal);

            reveal.remove_prefix(commaPos + 1);
        }

        // Handle reveal after last comma
        valid &= singleColorRevealFits(reveal);

        return valid;
    }


    bool part1LineIsValid(std::string_view line)
    {
        bool valid{ true };

        // Parse of game id header
        line.remove_prefix(line.find(":") + 1);

        // Check each reveal for being valid.
        size_t semicolonPos;
        while ((semicolonPos = line.find(';')) != std::string::npos)
        {
            valid &= fullRevealFitsColors(line.substr(0, semicolonPos));
            line.remove_prefix(semicolonPos + 1);
        }

        // Handle the last game after semicolons (no ending semicolon):
        valid &= fullRevealFitsColors(line);

        return valid;
    }

    void updateMinimaForColorReveal(std::string_view colorReveal, int& minRed, int& minBlue, int& minGreen)
    {
        if (colorReveal[0] == ' ')
        {
            colorReveal.remove_prefix(1);
        }

        const size_t spacePos{ colorReveal.find(' ') };
        int number;
        std::from_chars(colorReveal.data(), colorReveal.data() + colorReveal.size(), number);

        if (colorReveal[spacePos + 1] == 'b')
        {
            if (number > minBlue)
            {
                minBlue = number;
            }

            return;
        }

        if (colorReveal[spacePos + 1] == 'g')
        {
            if (number > minGreen)
            {
                minGreen = number;
            }

            return;
        }

        if (colorReveal[spacePos + 1] == 'r')
        {
            if (number > minRed)
            {
                minRed = number;
            }

            return;
        }

        std::cout << "Found unexpected letter" << colorReveal[spacePos + 1];
        throw;
    }

    void updateMinimaForFullReveal(std::string_view reveal, int& minRed, int& minBlue, int& minGreen)
    {
        // Loop over seperate color reveals
        size_t commaPos;
        while ((commaPos = reveal.find(',')) != std::string::npos)
        {
            const std::string_view colorReveal = reveal.substr(0, commaPos);
            updateMinimaForColorReveal(colorReveal,
                minRed,
                minBlue,
                minGreen);

            reveal.remove_prefix(commaPos + 1);
        }

        // Handle reveal after last comma
        updateMinimaForColorReveal(reveal,
            minRed,
            minBlue,
            minGreen);
    }

    int part2LinePower(std::string_view line)
    {
        int minRed{};
        int minBlue{};
        int minGreen{};
    
        // Parse of game id header
        line.remove_prefix(line.find(":") + 1);
    
        // Check each reveal for updating minima.
        size_t semicolonPos;
        while ((semicolonPos = line.find(';')) != std::string::npos)
        {
            updateMinimaForFullReveal(line.substr(0, semicolonPos),
                minRed,
                minBlue,
                minGreen);
            line.remove_prefix(semicolonPos + 1);
        }

        // Handle the last game after semicolons (no ending semicolon):
        updateMinimaForFullReveal(line,
            minRed,
            minBlue,
            minGreen);
    
        return minRed * minBlue * minGreen;
    }
}
