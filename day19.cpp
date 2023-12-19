#include "day19.h"
#include "helpers.h"

#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace day19
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 19 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day19_example.txt" : "inputs/day19_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct Part
    {
        // x, m, a, s
        std::array<long long, 4> categories{};

        Part(const std::string& inputLine)
        {
            std::stringstream ss{};
            ss << inputLine;

            ss.ignore(3);
            ss >> categories[0];
            ss.ignore(3);
            ss >> categories[1];
            ss.ignore(3);
            ss >> categories[2];
            ss.ignore(3);
            ss >> categories[3];
        }

        [[nodiscard]] long long score() const
        {
            return categories[0] + categories[1] + categories[2] + categories[3];
        }
    };

    struct Rule
    {
        bool hasCondition{};
        size_t conditionIndex{};
        bool lessThan{};
        int compareValue{};
        std::string target{};

        Rule(const std::string_view ruleString)
        {
            if(ruleString.find(':') == std::string::npos)
            {
                // Fallback case
                hasCondition = false;
                target = ruleString;
            }
            else
            {
                // Conditional case
                hasCondition = true;
                switch (ruleString[0])
                {
                case 'x':
                    conditionIndex = 0;
                    break;

                case 'm':
                    conditionIndex = 1;
                    break;

                case 'a':
                    conditionIndex = 2;
                    break;

                case 's':
                    conditionIndex = 3;
                    break;

                default:
                    assert(false);
                }

                lessThan = ruleString[1] == '<';

                std::stringstream ss{};
                ss << ruleString;
                ss.ignore(2);
                ss >> compareValue;

                const size_t sepPos{ ruleString.find(':') };
                target = ruleString.substr(sepPos + 1, ruleString.size() - sepPos - 1);
            }
        }
    };

    struct Workflow
    {
        std::string name;
        std::vector<Rule> rules;

        Workflow()
        = default;

        Workflow(const std::string_view inputLine)
        {
            const auto bracketPos{ inputLine.find('{') };
            name = inputLine.substr(0, bracketPos);
            const std::string rulesString{ inputLine.substr(bracketPos + 1, inputLine.size() - bracketPos - 2) };
            const auto ruleStrings{ splitStringBySeperator(rulesString, ',') };

            for (const auto& ruleString : ruleStrings)
            {
                rules.emplace_back(ruleString);
            }
        }
    };

    struct RatingsRegion
    {
        std::array<std::pair<int, int>, 4> rangePerCategory{};
        std::string currentWorkflowName{};

        [[nodiscard]] long long nrOfOptions() const
        {
            long long options{ 1 };
            for (size_t i= 0; i<4;i++)
            {
                options *= static_cast<long long>(rangePerCategory[i].second - rangePerCategory[i].first + 1);
            }
            return options;
        }
    };

    struct Puzzle
    {
        std::vector<Part> parts{};
        std::vector<Workflow> flows{};
        std::unordered_map<std::string, Workflow> flowLookup{};

        std::vector<Part> acceptedParts{};
        std::vector<RatingsRegion> acceptedRatingRegions{};

        void parseInput(std::ifstream& file)
        {
            std::string line;
            std::getline(file, line);

            while(!line.empty())
            {
                Workflow f(line);
                flows.push_back(f);
                flowLookup[f.name] = f;
                std::getline(file, line);
            }

            while(!file.eof())
            {
                std::getline(file, line);
                parts.emplace_back(line);
            }
        }

        void runPartsThroughFlows()
        {
            for (auto part : parts)
            {
                std::string currentFlowName{ "in" };

                while(currentFlowName != "A" && currentFlowName != "R")
                {
                    const Workflow& f{ flowLookup[currentFlowName] };
                    for (const auto& rule : f.rules)
                    {
                        if (!rule.hasCondition)
                        {
                            currentFlowName = rule.target;
                            break;
                        }

                        if (rule.lessThan 
                            ? part.categories[rule.conditionIndex] < rule.compareValue
                            : part.categories[rule.conditionIndex] > rule.compareValue)
                        {
                            currentFlowName = rule.target;
                            break;
                        }
                    }
                }

                if (currentFlowName == "A")
                {
                    acceptedParts.push_back(part);
                }
            }
        }

        void determineRatingRegions()
        {
            const RatingsRegion initialFullRegion{
                std::array{std::pair{1, 4000}, std::pair{1, 4000}, std::pair{1, 4000}, std::pair{1, 4000}},
                "in"
            };

            std::vector<RatingsRegion> regionsToProcess{ initialFullRegion };

            while(!regionsToProcess.empty())
            {
                RatingsRegion region{ regionsToProcess.back() };
                regionsToProcess.pop_back();

                if(region.currentWorkflowName == "R")
                {
                    // Rejected
                    continue;
                }

                if (region.currentWorkflowName == "A")
                {
                    // Accepted
                    acceptedRatingRegions.push_back(region);
                    continue;
                }

                // Applied to a workflow
                const Workflow& flow{ flowLookup[region.currentWorkflowName] };
                for (const auto& rule : flow.rules)
                {
                    if (!rule.hasCondition)
                    {
                        // Send full remainder of region to target
                        regionsToProcess.emplace_back(region.rangePerCategory, rule.target);
                        break;
                    }

                    if (rule.lessThan)
                    {
                        if (region.rangePerCategory[rule.conditionIndex].second < rule.compareValue)
                        {
                            // Entire range falls in condition
                            regionsToProcess.emplace_back(region.rangePerCategory, rule.target);
                            break;
                        }

                        if (region.rangePerCategory[rule.conditionIndex].first > rule.compareValue)
                        {
                            // Entire range falls out of condition
                            continue;
                        }

                        // Have to split range up
                        RatingsRegion regionInCondition{ region.rangePerCategory, rule.target };
                        regionInCondition.rangePerCategory[rule.conditionIndex].second = rule.compareValue - 1;
                        regionsToProcess.push_back(regionInCondition);

                        // Update current region to remainder
                        region.rangePerCategory[rule.conditionIndex].first = rule.compareValue;
                    }
                    else
                    {
                        if (region.rangePerCategory[rule.conditionIndex].first > rule.compareValue)
                        {
                            // Entire range falls in condition
                            regionsToProcess.emplace_back(region.rangePerCategory, rule.target);
                            break;
                        }

                        if (region.rangePerCategory[rule.conditionIndex].second < rule.compareValue)
                        {
                            // Entire range falls out of condition
                            continue;
                        }

                        // Have to split range up
                        RatingsRegion regionInCondition{ region.rangePerCategory, rule.target };
                        regionInCondition.rangePerCategory[rule.conditionIndex].first = rule.compareValue + 1;
                        regionsToProcess.push_back(regionInCondition);

                        // Update current region to remainder
                        region.rangePerCategory[rule.conditionIndex].second = rule.compareValue;
                    }
                }
            }
        }

        [[nodiscard]] long long scoreAcceptedParts() const
        {
            long long sum{};
            for (const auto& acceptedPart : acceptedParts)
            {
                sum += acceptedPart.score();
            }

            return sum;
        }

        [[nodiscard]] long long scoreRatingPossibilities() const
        {
            long long sum{};
            for (const auto& acceptedRegion: acceptedRatingRegions)
            {
                sum += acceptedRegion.nrOfOptions();
            }

            return sum;
        }
    };

    long long solvePart1(std::ifstream& file)
    {
        Puzzle puzzle{};
        puzzle.parseInput(file);
        puzzle.runPartsThroughFlows();
        return puzzle.scoreAcceptedParts();
    }

    long long solvePart2(std::ifstream& file)
    {
        Puzzle puzzle{};
        puzzle.parseInput(file);
        puzzle.determineRatingRegions();
        return puzzle.scoreRatingPossibilities();
    }
}