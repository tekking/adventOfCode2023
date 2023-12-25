#include "day25.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace day25
{
    long long solvePart1(std::ifstream& file);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 25 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day25_example.txt" : "inputs/day25_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct Puzzle
    {
        std::unordered_map<std::string, size_t> indexLookup{};
        std::vector<std::vector<bool>> connectedLookup{};

        void parseInput(std::ifstream& file)
        {
            size_t indexCounter{};
            std::vector<std::string> lines{};

            // Double loop to first determine all indices, then fill out connected lookup
            while(!file.eof())
            {
                std::string line;
                std::getline(file, line);
                lines.push_back(line);

                auto compName{ line.substr(0, 3) };
                if (!indexLookup.contains(compName))
                {
                    indexLookup[compName] = indexCounter;
                    indexCounter++;
                }

                const auto connectedComps{ splitStringBySeperator(line.substr(5, line.size() - 5), ' ')};
                for (const std::string& connectedComp : connectedComps)
                {
                    if (!indexLookup.contains(connectedComp))
                    {
                        indexLookup[connectedComp] = indexCounter;
                        indexCounter++;
                    }
                }                
            }

            connectedLookup = std::vector(indexCounter, std::vector(indexCounter, false));

            for (const auto& line : lines)
            {
                auto compName{ line.substr(0, 3) };
                const auto indexOfComp{ indexLookup[compName] };

                const auto connectedComps{ splitStringBySeperator(line.substr(5, line.size() - 5), ' ') };
                for (const std::string& connectedComp : connectedComps)
                {
                    const auto indexOfConnectedComp{ indexLookup[connectedComp] };

                    connectedLookup[indexOfComp][indexOfConnectedComp] = true;
                    connectedLookup[indexOfConnectedComp][indexOfComp] = true;
                }
            }
        }

        void printConnectedLookup() const
        {
            std::cout << '\n';

            for (const auto& row : connectedLookup)
            {
                for (const auto connection : row)
                {
                    std::cout << (connection ? '#': '.');
                }
                std::cout << '\n';
            }
        }

        [[nodiscard]] std::pair<bool, std::vector<size_t>> findShortestPath(const size_t s, const size_t t, const std::vector<std::vector<bool>>& adjacency) const
        {
            std::queue<size_t> queue{};
            queue.push(t);
            std::vector<size_t> pathLookup(adjacency.size(), -1);
            std::vector<bool> processed(adjacency.size(), false);

            while(!queue.empty())
            {
                const size_t curNode{ queue.front() };
                queue.pop();

                if (processed[curNode])
                {
                    continue;
                }

                processed[curNode] = true;

                if (curNode == s)
                {
                    break;
                }

                for(size_t i =0; i < adjacency.size(); i++)
                {
                    if (adjacency[curNode][i])
                    {
                        if (pathLookup[i] == static_cast<size_t>(-1))
                        {
                            pathLookup[i] = curNode;
                        }

                        queue.push(i);
                    }
                }
            }

            if(pathLookup[s] == static_cast<size_t>(-1))
            {
                return std::pair{ false, std::vector<size_t>{}};
            }

            // Reconstruct path:
            std::vector<size_t> path{};
            size_t n{ s };
            while (n != t)
            {
                auto prev{ pathLookup[n] };
                path.push_back(prev);
                n = prev;
            }

            return std::pair{ true, path };
        }

        // Returns both the max flow & remaining adjacency matrix
        [[nodiscard]] std::pair<long long, std::vector<std::vector<bool>>>  findMaxFlowBetweenNodes(const size_t s, const size_t t) const
        {
            std::vector<std::vector<bool>> remainingAdjacency = connectedLookup;
            long long maxFlow{};

            while(true)
            {
                // Find a path
                auto [found, path] = findShortestPath(s, t, remainingAdjacency);

                if (!found)
                {
                    break;
                }

                // Update flow & remaining edges
                maxFlow++;

                size_t curNode{ s };
                for (const size_t node : path)
                {
                    remainingAdjacency[curNode][node] = false;
                    remainingAdjacency[node][curNode] = false;
                    curNode = node;
                }
            }

            return std::pair{ maxFlow, remainingAdjacency };
        }

        [[nodiscard]] long long countNumberOfReachableNodes(size_t s, std::vector<std::vector<bool>> adjacencyMatrix) const
        {
            std::vector<bool> reached(adjacencyMatrix.size(), false);

            std::queue<size_t> fillQueue{};
            fillQueue.push(s);

            while(!fillQueue.empty())
            {
                const size_t node{ fillQueue.front() };
                fillQueue.pop();

                if (reached[node])
                {
                    continue;
                }

                reached[node] = true;
                for (size_t i = 0; i < adjacencyMatrix.size(); i++)
                {
                    if (adjacencyMatrix[node][i])
                    {
                        fillQueue.push(i);
                    }
                }
            }

            return std::ranges::count_if(reached, [](bool b) {return b; });
        }

        [[nodiscard]] long long scoreSeperatedGroups() const
        {
            // Just loop over possible combinations of s & t, and score when we find one with max-flow 3.
            // Min cut should always exist with [0] on one side, so don't actually need to loop over s
            size_t s{ 0 };
            for (size_t t = s + 1; t < connectedLookup.size(); t++)
            {
                auto [maxFlow, remainingAdjacency] = findMaxFlowBetweenNodes(s, t);

                if (maxFlow == 3)
                {
                    // Found the cut
                    // Check sizes of each partition by doing floodfill from s
                    const auto reachableFromS = countNumberOfReachableNodes(s, remainingAdjacency);
                    const auto reachableFromT = countNumberOfReachableNodes(t, remainingAdjacency);

                    // Assumption: s & t split into two groups that connect entire graph together.
                    assert(reachableFromS + reachableFromT == static_cast<long long>(connectedLookup.size()));

                    return reachableFromS * reachableFromT;
                }
            }

            assert(false);
            return 0;
        }
    };

    long long solvePart1(std::ifstream& file)
    {
        Puzzle puzzle{};
        puzzle.parseInput(file);
        // puzzle.printConnectedLookup();
        return puzzle.scoreSeperatedGroups();
    }

    long long solvePart2(std::ifstream& file)
    {
        return 0;
    }
}