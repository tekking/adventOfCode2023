#include "day20.h"
#include "helpers.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace day20
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 20 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day20_example.txt" : "inputs/day20_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();

        // Kind of weird to have a part 2 that doesn't function for the examples....
        if (!example)
        {
            file.open(fileName);

            std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
        }
    }

    enum class ModuleType
    {
        broadcaster,
        flipFlop,
        conjunction,
        other
    };

    struct Pulse
    {
        std::string source{};
        std::string target{};
        bool high{};
    };

    struct Module
    {
        std::string name{};
        ModuleType type{};
        std::vector<std::string> targets{};

        bool flipFlopOn{};

        std::unordered_map<std::string, bool> conjunctionSourcesHigh{};

        Module() = default;

        Module(std::string_view string)
        {
            std::stringstream ss{};
            ss << string;

            std::string section{};
            ss >> section;

            if (section[0] == '%')
            {
                type = ModuleType::flipFlop;
                name = section.substr(1, section.size() - 1);
            }
            else if (section[0] == '&')
            {
                type = ModuleType::conjunction;
                name = section.substr(1, section.size() - 1);
            }
            else if (section == "broadcaster")
            {
                type = ModuleType::broadcaster;
                name = section;
            }
            else
            {
                type = ModuleType::other;
                name = section;
            }

            ss >> section;
            while(!ss.eof())
            {
                ss >> section;
                if (section.ends_with(','))
                {
                    targets.push_back(section.substr(0, section.size() - 1));
                }
                else
                {
                    targets.push_back(section);
                }
            }
        }

        void initializeConjuntionSources(const std::vector<std::string>& sources)
        {
            for (const auto& source : sources)
            {
                conjunctionSourcesHigh.emplace(source, false);
            }
        }

        std::vector<Pulse> processPulse(const Pulse& p)
        {
            std::vector<Pulse> results{};

            if (type == ModuleType::broadcaster)
            {
                for (const std::string& target : targets)
                {
                    results.push_back(Pulse{name, target, p.high});
                }
            }
            else if (type == ModuleType::flipFlop)
            {
                if (!p.high)
                {
                    flipFlopOn = !flipFlopOn;
                    for (const std::string& target : targets)
                    {
                        results.push_back(Pulse{ name, target, flipFlopOn });
                    }
                }
            }
            else if (type == ModuleType::conjunction)
            {
                conjunctionSourcesHigh[p.source] = p.high;

                const bool conjunctionOutputIsHigh = !std::ranges::all_of(conjunctionSourcesHigh, [](const std::pair<std::string, bool>& source) {return source.second; });
                for (const std::string& target : targets)
                {
                    results.push_back(Pulse{ name, target, conjunctionOutputIsHigh });
                }
            }

            return results;
        }
    };

    struct System
    {
        std::unordered_map<std::string, Module> moduleLookup{};

        void parseInput(std::ifstream& file)
        {
            while(!file.eof())
            {
                std::string line;
                std::getline(file, line);
                Module m{ line };
                moduleLookup.emplace(m.name, m);
            }

            // Set up the source memory for each conjunction module:
            for (auto& module : moduleLookup | std::views::values)
            {
                if (module.type == ModuleType::conjunction)
                {
                    std::vector<std::string> sources{};
                    for (const auto& otherModule : moduleLookup | std::views::values)
                    {
                        if (std::ranges::find(otherModule.targets, module.name) != otherModule.targets.end())
                        {
                            sources.push_back(otherModule.name);
                        }
                    }

                    module.initializeConjuntionSources(sources);
                }
            }
        }

        long long scoreButtonPresses(const int count)
        {
            long long totalHighPulses{};
            long long totalLowPulses{};

            for (int i =0; i<count;i++)
            {
                std::queue<Pulse> pulseQueue{};
                pulseQueue.emplace("button", "broadcaster", false);

                while(!pulseQueue.empty())
                {
                    const Pulse& p{ pulseQueue.front() };
                    if (p.high)
                    {
                        totalHighPulses++;
                    }
                    else
                    {
                        totalLowPulses++;
                    }

                    auto newPulses{ moduleLookup[p.target].processPulse(p) };
                    pulseQueue.pop();
                    for (const auto& newPulse : newPulses)
                    {
                        pulseQueue.push(newPulse);
                    }
                }
            }

            return totalHighPulses * totalLowPulses;
        }

        long long buttonPressesForRx()
        {
            const std::string sourceOfRxName = "df";
            const auto& sourceOfRx{ moduleLookup[sourceOfRxName] };
            std::unordered_map<std::string, long long> firstValidAtForSourcesOfDf{};

            long long presses{};
            bool stillLookingForCycles{ true };
            while(stillLookingForCycles)
            {
                presses++;

                std::queue<Pulse> pulseQueue{};
                pulseQueue.emplace("button", "broadcaster", false);

                while (!pulseQueue.empty())
                {
                    const Pulse& p{ pulseQueue.front() };
                    if (p.target == "rx" && !p.high)
                    {
                        return presses;
                    }

                    for (const auto& conjunctionSourcesHigh : sourceOfRx.conjunctionSourcesHigh)
                    {
                        if (conjunctionSourcesHigh.second)
                        {
                            if (!firstValidAtForSourcesOfDf.contains(conjunctionSourcesHigh.first))
                            {
                                firstValidAtForSourcesOfDf[conjunctionSourcesHigh.first] = presses;

                                if (firstValidAtForSourcesOfDf.size() == 4)
                                {
                                    stillLookingForCycles = false;
                                }
                            }
                        }
                    }

                    auto newPulses{ moduleLookup[p.target].processPulse(p) };
                    pulseQueue.pop();
                    for (const auto& newPulse : newPulses)
                    {
                        pulseQueue.push(newPulse);
                    }
                }
            }

            // This works for puzzle input, but seems unlikely to work/hard to prove to work for any valid problem input?
            long long lcm{ 1 };
            for (const long long i : firstValidAtForSourcesOfDf | std::views::values)
            {
                lcm = std::lcm(lcm, i);
            }

            return lcm;
        }
    };


    long long solvePart1(std::ifstream& file)
    {
        System system{};
        system.parseInput(file);
        return system.scoreButtonPresses(1000);
    }

    long long solvePart2(std::ifstream& file)
    {
        System system{};
        system.parseInput(file);
        return system.buttonPressesForRx();
    }
}