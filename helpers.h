#pragma once
#include <string>
#include <vector>

std::vector<int> parseLineOfNumbers(const std::string_view line);
std::vector<long long> parseLineOfNumbersToLongLong(const std::string_view line);
std::vector<int> parseLineOfSymbolSeperatedNumbers(const std::string_view line);
std::vector<std::string> splitStringBySeperator(std::string_view string, char seperator);
