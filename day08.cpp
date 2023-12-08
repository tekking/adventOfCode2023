#include "day08.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace day08
{
    int solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);
    long long altSolvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 08 " << (example ? "(example)" : "") << '\n';

        std::string fileName{ example ? "inputs/day08_example.txt" : "inputs/day08_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        fileName = example ? "inputs/day08_example2.txt" : "inputs/day08_real.txt" ;
        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << altSolvePart2(file) << '\n';
    }

    struct Map
    {
        std::string directions;
        std::unordered_map<std::string, std::pair<std::string, std::string>> connections;
    };

    Map parseInput(std::ifstream& file)
    {
        // Get directions
        std::string directions;
        std::getline(file, directions);

        // Skip empty line
        std::string line;
        std::getline(file, line);

        // Build up map of directions (1 string -> 2 strings)
        std::unordered_map<std::string, std::pair<std::string, std::string>> connections{};
        std::stringstream ss;
        while (!file.eof())
        {
            // Parse based on fact all identifiers are 3 char long
            std::getline(file, line);
            std::string source{ line.substr(0, 3) };
            std::string left{ line.substr(7, 3) };
            std::string right{ line.substr(12, 3) };
            connections[source] = std::pair{ left, right };
        }

        return Map{ directions, connections };
    }


    int solvePart1(std::ifstream& file)
    {
        auto map = parseInput(file);
        std::string location{ "AAA" };
        int stepsTaken = 0;
        size_t directionIndex = 0;

        while (location != "ZZZ")
        {
            if (directionIndex >= map.directions.size())
            {
                directionIndex = 0;
            }

            location = (map.directions[directionIndex] == 'L')
                ? map.connections[location].first
                : map.connections[location].second;

            stepsTaken++;
            directionIndex++;
        }

        return stepsTaken;
    }

    long long solvePart2(std::ifstream& file)
    {
        auto map = parseInput(file);

        // Find locations ending with 'A' as starts
        std::vector<std::string> locations{};
        for (const auto location : map.connections | std::views::keys)
        {
            if (location.ends_with('A'))
            {
                locations.push_back(location);
            }
        }

        long long stepsTaken = 0;
        size_t directionIndex = 0;

        // Loop while any of our locations do not end with 'Z'
        while (std::ranges::any_of(locations, [](const std::string& location) {return !location.ends_with('Z'); }))
        {
            if (directionIndex >= map.directions.size())
            {
                directionIndex = 0;
            }

            for (auto& location : locations)
            {
                location = (map.directions[directionIndex] == 'L')
                               ? map.connections[location].first
                               : map.connections[location].second;
            }

            stepsTaken++;
            directionIndex++;

            if (stepsTaken % 1000000 == 0)
            {
                std::cout << stepsTaken << '\n';
            }
        }

        return stepsTaken;
    }

    long long altSolvePart2(std::ifstream& file)
    {
        auto map = parseInput(file);

        // Find locations ending with 'A' as starts
        std::vector<std::string> locations{};
        for (const auto location : map.connections | std::views::keys)
        {
            if (location.ends_with('A'))
            {
                locations.push_back(location);
            }
        }

        int stepsTaken = 0;
        size_t directionIndex = 0;

        std::vector<int> initialZ(locations.size(), -1);
        std::vector<int> periodBetweenZ(locations.size(), -1);
        std::vector<std::string> firstZLocations(locations.size());

        // Attempt to solve by determining periods of time between 'Z' visits of each starting location
        // Trial run showed their 'loops' only pass through single 'Z' location each, otherwise this gets more complicated.
        // Also turns out all starts have period equal to initial, which simplifies equation solving.
        while (std::ranges::any_of(periodBetweenZ, [](const int p) {return p == -1; }))
        {
            if (directionIndex >= map.directions.size())
            {
                directionIndex = 0;
            }

            for (size_t i = 0; i < locations.size(); i++)
            {
                std::string& location = locations[i];

                if (location.ends_with('Z'))
                {
                    if (initialZ[i] == -1)
                    {
                        // Reached 'Z' ending node for first time
                        initialZ[i] = stepsTaken;
                        firstZLocations[i] = location;
                    }
                    else if (periodBetweenZ[i] == -1)
                    {
                        // Reached 'Z' ending node second time
                        periodBetweenZ[i] = stepsTaken - initialZ[i];
                        assert(firstZLocations[i] == location);
                        assert(periodBetweenZ[i] == initialZ[i]);
                    }
                }

                location = (map.directions[directionIndex] == 'L')
                    ? map.connections[location].first
                    : map.connections[location].second;
            }

            stepsTaken++;
            directionIndex++;
        }

        std::cout << '\n';
        for (size_t i = 0; i < initialZ.size(); i++)
        {
            std::cout << "Location " << i << " has initial " << initialZ[i] << " and period " << periodBetweenZ[i] << '\n';
        }

        // Now we have the initials and periods we can try to solve for intersection on 'Z' of all loops
        // Because of the specifics of the input case, we can solve by finding least common multiple, but this feels weird since it
        // doesn't seem correct for all possible inputs under the ruleset...
        long long lcm{ 1 };
        for (const int i : initialZ)
        {
            lcm = std::lcm(lcm, i);
        }

        return lcm;
    }
}