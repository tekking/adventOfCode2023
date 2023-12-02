#include "day01.h"
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

namespace day01
{
    int part1LineSum(const std::string& line);
    int part2LineSum(const std::string& line);

    void run_day(const bool example)
    {
        std::cout << "Running day 01 \n";

        const std::string fileName{ example ? "day01_example.txt" : "day01_real.txt" };
        std::ifstream file{ fileName };

        int sum{};
        std::string line;
        while (std::getline(file, line))
        {
            sum += part1LineSum(line);
        }

        std::cout << "Part 1 answer: " << sum << '\n';

        file.close();
        file.open(fileName);
        sum = 0;
        while (std::getline(file, line))
        {
            sum += part2LineSum(line);
        }


        std::cout << "Part 2 answer: " << sum << '\n';
    }

    int part1LineSum(const std::string& line)
    {
        int secondDigit{};
        int firstDigit{ -1 };
        for (const char c : line)
        {
            if (std::isdigit(c))
            {
                if (firstDigit < 0)
                {
                    firstDigit = c - '0';
                }

                secondDigit = c - '0';
            }
        }

        return firstDigit * 10 + secondDigit;
    }

    const char* digits[9] = { "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };

    int detectEndingDigitWord(const std::string& string)
    {
        for (int i = 1; i < 10; i++)
        {
            if (string.ends_with(digits[i-1]))
            {
                return i;
            }
        }

        return -1;
    }

    int part2LineSum(const std::string& line)
    {
        int secondDigit{};
        int firstDigit{ -1 };

        for (int i = 0; i < line.length(); i++)
        {
            std::string subLine{ line.substr(0, i + 1) };
            int currentDigit;

            if (std::isdigit(subLine[i]))
            {
                currentDigit = subLine[i] - '0';
            }
            else
            {
                currentDigit = detectEndingDigitWord(subLine);
            }

            if (currentDigit > 0)
            {
                if (firstDigit < 0)
                {
                    firstDigit = currentDigit;
                }

                secondDigit = currentDigit;
            }
        }

        return firstDigit * 10 + secondDigit;
    }
}
