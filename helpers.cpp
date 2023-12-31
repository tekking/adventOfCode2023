#include <sstream>
#include <string>
#include <vector>

// Parses a string of whitespace seperated integers into a vector.
std::vector<int> parseLineOfNumbers(const std::string_view line)
{
    std::stringstream lineStream;
    int nextNr;

    std::vector<int> result{};
    lineStream << line;
    while (true)
    {
        lineStream >> nextNr; // Resharper seems to think this is invalid...
        if (lineStream.fail())
        {
            break;
        }

        result.push_back(nextNr);
    }

    return result;
}

// Parses a string of whitespace seperated long long integers into a vector.
std::vector<long long> parseLineOfNumbersToLongLong(const std::string_view line)
{
    std::stringstream lineStream;
    long long nextNr;

    std::vector<long long> result{};
    lineStream << line;
    while (true)
    {
        lineStream >> nextNr; // Resharper seems to also think this is invalid...
        if (lineStream.fail())
        {
            break;
        }

        result.push_back(nextNr);
    }

    return result;
}

// Parses a string of integers seperated by a single non-whitespace symbol (such as ',') into a vector.
std::vector<int> parseLineOfSymbolSeperatedNumbers(const std::string_view line)
{
    std::stringstream lineStream;
    int nextNr;

    std::vector<int> result{};
    lineStream << line;
    while (true)
    {
        lineStream >> nextNr; // Resharper seems to think this is invalid...
        if (lineStream.fail())
        {
            break;
        }

        result.push_back(nextNr);
        lineStream.ignore(1);
    }

    return result;
}

std::vector<std::string> splitStringBySeperator(std::string_view string, char seperator)
{
    std::vector<std::string> result{};
    std::string::size_type index;
    while((index = string.find(seperator)) != std::string::npos)
    {
        result.emplace_back(string.substr(0, index));
        string.remove_prefix(index + 1);
    }

    result.emplace_back(string);

    return result;
}