#include "day05.h"
#include "helpers.h"

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace day05
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 05 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day05_example.txt" : "inputs/day05_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct range
    {
        long long start{};
        long long end{};
    };

    struct rangeMapping
    {
        range source{};
        long long offSet{};
    };

    // Parses the mapping section of input to a list of mapping sections,
    // each mapping section contains a list of range-mappings, where each range-mapping
    // consists of a source range and an offset by which mapping happens.
    std::vector<std::vector<rangeMapping>> parseMappings(std::ifstream& file)
    {
        std::string line;
        std::vector<std::vector<rangeMapping>> mappings(0);
        while (!file.eof())
        {
            // Skip over non-number lines till we get to next mapping section numbers
            do
            {
                std::getline(file, line);
            } while (!std::isdigit(line[0]));

            // Create a list of mapping ranges:
            std::vector<rangeMapping> mappingRangesOfMapping(0);

            while (std::isdigit(line[0]))
            {
                long long targetStart;
                long long sourceStart;
                long long length;
                std::stringstream lineStream;
                lineStream << line;
                lineStream >> targetStart;
                lineStream >> sourceStart;
                lineStream >> length;

                const range sourceRange{ sourceStart, sourceStart + length };
                const rangeMapping mapping{ sourceRange, targetStart - sourceStart };
                mappingRangesOfMapping.push_back(mapping);

                if (file.eof())
                {
                    break;
                }

                std::getline(file, line);
            }

            mappings.push_back(mappingRangesOfMapping);
        }

        return mappings;
    }

    // Parses input into a pair, first of which is list of seed numbers, second is a list
    // of mappings, where each mapping is a a list of mapping-ranges, which consist of three integers
    // (start target, start source, length) each.
    std::pair<std::vector<long long>, std::vector<std::vector<rangeMapping>>> parseInputPart1(std::ifstream& file)
    {
        // First parse the seeds
        std::string line;
        std::getline(file, line);
        const size_t breakPos = line.find(' ');
        std::vector<long long> seeds{
            parseLineOfNumbersToLongLong(line.substr(breakPos, line.size() - breakPos)) };

        // Then parse the mappings
        auto mappings{ parseMappings(file) };
        

        return { seeds, mappings };
    }

    // Parses input into a pair, first of which is list of seed ranges, second is a list
    // of mappings, where each mapping is a a list of mapping-ranges, which consist of three integers
    // (start target, start source, length) each.
    std::pair<std::vector<range>, std::vector<std::vector<rangeMapping>>> parseInputPart2(std::ifstream& file)
    {
        // First parse the seeds
        std::string line;
        std::getline(file, line);
        const size_t breakPos = line.find(' ');
        const auto seedInputs{
            parseLineOfNumbersToLongLong(line.substr(breakPos, line.size() - breakPos)) };
        std::vector<range> seedRanges;
        const size_t numberOfRanges = seedInputs.size() / 2;
        for (size_t i = 0; i < numberOfRanges; i++)
        {
            // Each two numbers form a seedrange together;
            range seedRange{ seedInputs[i * 2], seedInputs[i * 2] + seedInputs[i * 2 + 1] };
            seedRanges.push_back(seedRange);
        }

        // Then parse the mappings
        auto mappings{ parseMappings(file) };

        return { seedRanges, mappings };
    }

    long long solvePart1(std::ifstream& file)
    {
        // Parse the input
        auto [seeds, mappings] = parseInputPart1(file);

        // loop over each step in the 'mapping' proces
        for (auto& fullMapping : mappings)
        {
            // For each mapping update each seed
            for (auto& value : seeds)
            {
                // Check each range for the correct one
                for (const auto& mappingRange : fullMapping)
                {
                    if (value > mappingRange.source.start && value < mappingRange.source.end)
                    {
                        // Update the seed by the difference in start of range.
                        value += mappingRange.offSet;
                        break;
                    }
                }
            }
        }

        // Have final mappings of each seed now, just take min
        return std::ranges::min(seeds);
    }

    long long solvePart2(std::ifstream& file)
    {
        // Parse the input
        auto [seedRanges, mappings] = parseInputPart2(file);
        
        // loop over each step in the 'mapping' proces
        for (auto& fullMapping : mappings)
        {
            // Keep track both of mapped new ranges, and remainder of current ranges
            std::vector<range> newRanges(0);
            std::vector<range> remainingRanges = seedRanges;

            // Loop over the range-mappings of the mapping section
            for (auto& mappingRange : fullMapping)
            {
                std::vector<range> newRemainingRanges(0);

                // Apply to each current remaing range
                for(auto& remainingRange: remainingRanges)
                {
                    // The result of applying mapping to seedRange can be seen as three outputs:
                    // 1. the part of seedrange below start of source-start (remains)
                    // 2. the part of seedrange between source-start and source-end (shifted)
                    // 3. the part of seedrange above source-end (remains)

                    range section1{ std::min(remainingRange.start, mappingRange.source.start),
                        std::min(remainingRange.end, mappingRange.source.start) };

                    if (section1.end - section1.start > 0)
                    {
                        newRemainingRanges.push_back(section1);
                    }

                    range section2{ std::max(remainingRange.start, mappingRange.source.start),
                    std::min(remainingRange.end, mappingRange.source.end) };

                    if (section2.end - section2.start > 0)
                    {
                        range mappedSection2{
                            section2.start + mappingRange.offSet, section2.end + mappingRange.offSet };
                        newRanges.push_back(mappedSection2);
                    }

                    range section3{ std::max(remainingRange.start, mappingRange.source.end),
                        std::max(remainingRange.end, mappingRange.source.end) };

                    if (section3.end - section3.start)
                    {
                        newRemainingRanges.push_back(section3);
                    }
                }

                // The newly remaining ranges after 'cutting out' parts with the current mapping.
                remainingRanges = newRemainingRanges;
            }

            // Add any remainders after all 'cutting out' to new ranges for next mapping section:
            for (range remainingRange : remainingRanges)
            {
                newRanges.push_back(remainingRange);
            }

            // Replace seedRanges with the results of the current mapping section.
            seedRanges = newRanges;
        }
        
        // Have final mappings of each seed now, just take min
        // Min here is lowest start of a range.
        auto minOfRanges = [](range a, range b) { return (a.start < b.start); };
        return std::ranges::min(seedRanges, minOfRanges).start;
    }
}