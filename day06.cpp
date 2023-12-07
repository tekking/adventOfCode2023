#include "day06.h"
#include "helpers.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace day06
{
    int solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 06 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day06_example.txt" : "inputs/day06_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    int solvePart1(std::ifstream& file)
    {
        std::string line;
        std::getline(file, line);

        // Parse times
        size_t sepPos{ line.find(':') };
        std::vector times { parseLineOfNumbers(line.substr(sepPos + 1, line.size() - sepPos - 1))};

        // Parse distances
        std::getline(file, line);
        sepPos = line.find(':');
        std::vector distances{ parseLineOfNumbers(line.substr(sepPos + 1, line.size() - sepPos - 1)) };

        int totalProduct{ 1 };
        for (size_t i{}; i < times.size(); i++)
        {
            const int time{ times[i] };
            const int distance{ distances[i] };

            for (int waitTime{}; waitTime < time / 2; waitTime ++)
            {
                const int distanceReached{ waitTime * (time - waitTime) };
                if (distanceReached > distance)
                {
                    const int countOfOptions{ time - (2 * waitTime) + 1 };
                    totalProduct *= countOfOptions;
                    break;
                }
            }
        }

        return totalProduct;
    }

    long long solvePart2(std::ifstream& file)
    {
        std::string line;
        std::getline(file, line);

        // Parse time
        size_t sepPos{ line.find(':') };
        std::string numberString{ line.substr(sepPos + 1, line.size() - sepPos - 1) };
        numberString.erase(remove_if(numberString.begin(), numberString.end(), isspace), numberString.end());

        long long time;
        std::stringstream numberStream;

        numberStream << numberString;
        numberStream >> time;

        // Parse distance
        std::getline(file, line);
        sepPos = line.find(':');
        numberString = line.substr(sepPos + 1, line.size() - sepPos - 1);
        numberString.erase(remove_if(numberString.begin(), numberString.end(), isspace), numberString.end());

        long long distance;
        numberStream.clear();
        numberStream << numberString;
        numberStream >> distance;

        // Same logic as part 1
        long long totalProduct{ 1 };
        for (int waitTime{}; waitTime < time / 2; waitTime++)
        {
            const long long distanceReached{ waitTime * (time - waitTime) };
            if (distanceReached > distance)
            {
                const long long countOfOptions{ time - (2LL * waitTime) + 1 };
                totalProduct *= countOfOptions;
                break;
            }
        }

        return totalProduct;
    }
}