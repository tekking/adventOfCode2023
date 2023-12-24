#include "day24.h"
#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace day24
{
    long long solvePart1(std::ifstream& file, bool example);
    long long solvePart2(std::ifstream& file);

    void run_day(bool example)
    {
        std::cout << "Running day 24 " << (example ? "(example)" : "") << '\n';

        const std::string fileName{ example ? "inputs/day24_example.txt" : "inputs/day24_real.txt" };
        std::ifstream file{ fileName };

        std::cout << "Part 1 answer: " << solvePart1(file, example) << '\n';

        file.close();
        file.open(fileName);

        std::cout << "Part 2 answer: " << solvePart2(file) << '\n';
    }

    struct Point
    {
        double x{};
        double y{};
        double z{};

        [[nodiscard]] Point operator+(const Point& other) const
        {
            return Point{ x + other.x, y + other.y, z + other.z };
        }

        [[nodiscard]] Point operator-(const Point& other) const
        {
            return Point{ x - other.x, y - other.y, z - other.z };
        }

        [[nodiscard]] Point operator*(const double factor) const
        {
            return Point{ factor * x, factor * y, factor * z };
        }
    };

    struct Hailstone
    {
        Point origin{};
        Point velocity{};

        [[nodiscard]] Point afterOneInterval() const
        {
            return Point{ origin.x + velocity.x, origin.y + velocity.y, origin.z + velocity.z };
        }

        [[nodiscard]] std::pair<bool, Point> intersectPointInTwoDimensions(const Hailstone& other) const
        {
            const Point secondPoint{ afterOneInterval() };
            const Point otherSecondPoint{ other.afterOneInterval() };

            const double denominator = (origin.x - secondPoint.x) * (other.origin.y - otherSecondPoint.y) - (origin.y - secondPoint.y) * (other.origin.x - otherSecondPoint.x);

            if (std::abs(denominator) < 0.000001)
            {
                // No intersection
                return std::pair{ false, Point{} };
            }

            const double xNumerator = (origin.x * secondPoint.y - origin.y * secondPoint.x) * (other.origin.x - otherSecondPoint.x) - (origin.x - secondPoint.x) * (other.origin.x * otherSecondPoint.y - other.origin.y * otherSecondPoint.x);
            const double yNumerator = (origin.x * secondPoint.y - origin.y * secondPoint.x) * (other.origin.y - otherSecondPoint.y) - (origin.y - secondPoint.y) * (other.origin.x * otherSecondPoint.y - other.origin.y * otherSecondPoint.x);

            double x = xNumerator / denominator;
            double y = yNumerator / denominator;

            return std::pair{ true, Point{x, y, 0} };
        }

        [[nodiscard]] bool isFutureCoincidentWithOnXY(const Hailstone& other) const
        {
            const Point secondPoint{ afterOneInterval() };
            const Point otherSecondPoint{ other.afterOneInterval() };
            const double denominator = (origin.x - secondPoint.x) * (other.origin.y - otherSecondPoint.y) - (origin.y - secondPoint.y) * (other.origin.x - otherSecondPoint.x);

            if (std::abs(denominator) >= 0.000001)
            {
                return false;
            }

            // parallel or coincident
            // Check for coincident case
            auto dx = origin.x - other.origin.x;
            auto dy = origin.y - other.origin.y;

            auto dt = dx / velocity.x;
            if (std::abs(dt * velocity.y - dy) < 0.00001)
            {
                if (dt >= 0)
                {
                    return true;
                }
            }

            // In other direction
            dx = other.origin.x - origin.x;
            dy = other.origin.y - origin.y;

            dt = dx / other.velocity.x;
            if (std::abs(dt * other.velocity.y - dy) < 0.00001)
            {
                if (dt >= 0)
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] std::pair<bool, Point> futureIntersectPointInTwoDimensions(const Hailstone& other) const
        {
            const Point secondPoint{ afterOneInterval() };
            const Point otherSecondPoint{ other.afterOneInterval() };

            const double denominator = (origin.x - secondPoint.x) * (other.origin.y - otherSecondPoint.y) - (origin.y - secondPoint.y) * (other.origin.x - otherSecondPoint.x);

            if (std::abs(denominator) < 0.000001)
            {
                return std::pair{ false, Point{} };
            }

            const double tNumerator = (origin.x - other.origin.x) * (other.origin.y - otherSecondPoint.y) - (origin.y - other.origin.y) * (other.origin.x - otherSecondPoint.x);
            const double otherTNumerator = (origin.x - other.origin.x) * (origin.y - secondPoint.y) - (origin.y - other.origin.y) * (origin.x - secondPoint.x);

            const double t = tNumerator / denominator;
            const double otherT = otherTNumerator / denominator;

            if (t < 0 || otherT < 0)
            {
                // In the past
                return std::pair{ false, Point{} };
            }

            Point intersection{ origin + velocity * t};

            return std::pair{ true, intersection };
        }

        [[nodiscard]] bool isFutureCoincidentWithOnXZ(const Hailstone& other) const
        {
            const Point secondPoint{ afterOneInterval() };
            const Point otherSecondPoint{ other.afterOneInterval() };
            const double denominator = (origin.x - secondPoint.x) * (other.origin.z - otherSecondPoint.z) - (origin.z - secondPoint.z) * (other.origin.x - otherSecondPoint.x);

            if (std::abs(denominator) >= 0.000001)
            {
                return false;
            }

            // parallel or coincident
            // Check for coincident case
            auto dx = origin.x - other.origin.x;
            auto dz = origin.z - other.origin.z;

            auto dt = dx / velocity.x;
            if (std::abs(dt * velocity.z - dz) < 0.00001)
            {
                if (dt >= 0)
                {
                    return true;
                }
            }

            // In other direction
            dx = other.origin.x - origin.x;
            dz = other.origin.z - origin.z;

            dt = dx / other.velocity.x;
            if (std::abs(dt * other.velocity.z - dz) < 0.00001)
            {
                if (dt >= 0)
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] std::pair<bool, Point> futureIntersectPointInTwoDimensionsOnXAndZ(const Hailstone& other) const
        {
            const Point secondPoint{ afterOneInterval() };
            const Point otherSecondPoint{ other.afterOneInterval() };

            const double denominator = (origin.x - secondPoint.x) * (other.origin.z - otherSecondPoint.z) - (origin.z - secondPoint.z) * (other.origin.x - otherSecondPoint.x);

            if (std::abs(denominator) < 0.000001)
            {
                return std::pair{ false, Point{} };
            }

            const double tNumerator = (origin.x - other.origin.x) * (other.origin.z - otherSecondPoint.z) - (origin.z - other.origin.z) * (other.origin.x - otherSecondPoint.x);
            const double otherTNumerator = (origin.x - other.origin.x) * (origin.z - secondPoint.z) - (origin.z - other.origin.z) * (origin.x - secondPoint.x);

            const double t = tNumerator / denominator;
            const double otherT = otherTNumerator / denominator;

            if (t < 0 || otherT < 0)
            {
                // In the past
                return std::pair{ false, Point{} };
            }

            Point intersection{ origin + velocity * t };

            return std::pair{ true, intersection };
        }
    };

    bool doublesAreSignificantlyDifferent(const double a, const double b)
    {
        const auto diff{ std::abs(a - b) };
        if (diff < 0.00001)
        {
            return false;
        }

        if (diff < (std::max(a, b) * 0.00000001))
        {
            return false;
        }

        return true;
    }

    struct Storm
    {
        std::vector<Hailstone> stones{};

        void parseInput(std::ifstream& file)
        {
            while (!file.eof())
            {
                std::string line;
                std::getline(file, line);

                std::stringstream ss{};
                ss << line;

                double x, y, z;
                ss >> x;
                ss.ignore(1);
                ss >> y;
                ss.ignore(1);
                ss >> z;

                Point origin{ x, y, z };

                ss.ignore(3);
                ss >> x;
                ss.ignore(1);
                ss >> y;
                ss.ignore(1);
                ss >> z;

                Point velocity{ x, y, z };
                stones.emplace_back(origin, velocity);
            }
        }

        [[nodiscard]] long long countInterectionsInArea(const double areaLower, const double areaUpper) const
        {
            long long count{};
            for (size_t i = 0; i < stones.size(); i++)
            {
                for (size_t j = i + 1; j < stones.size(); j++)
                {
                    auto& a{ stones[i] };
                    auto& b{ stones[j] };
                    const auto [exists, p] { a.futureIntersectPointInTwoDimensions(b) };
                    if (exists && p.x >= areaLower && p.x <= areaUpper && p.y >= areaLower && p.y <= areaUpper)
                    {
                        count++;
                    }
                }
            }

            return count;
        }

        [[nodiscard]] long long solveBreakingThrow() const
        {
            // Approach is to solve for x+y & x+z seperately
            // We approach solving by reframing coordinates, at t=0 we use the 'normal' coordinate frame
            // but after we use the coordinate frame shifted by breaking rock velocity * t.
            // Effect is that at any t, the breaking rock is at it's starting location (it effectively has v=0 in
            // this moving frame).
            // All falling hailstone gets their velocity modified by rock velocity
            // Any valid solution should have all hailstone 'go' through the now static location of the throwing rock
            // but we don't know where it is. So instead check if there is a single point all hailstone intersect,
            // by just taking intersection point for each pair & validating if all intersect & at the same spot.
            //
            // Running the above for 'reasonable' ranges of vx, vy & vz, we can check if any combination would be valid
            // This works well for example & real input, resulting in exactly 1 combination for both
            //
            // Because the rock is static in our moving frame, including at t=0 & our frame is identical to regular frame at t=0,
            // the static location of our rock is equal to the location of rock at t=0 (origin) & we know this location in our moving frame
            // from the intersection checks.

            // Note below code could be refactored a fair bit since logic on x+y & x+z is identical apart from the coordinate used.
            std::vector<std::pair<std::pair<double, double>, Point>> possibleXYVelocity{};
            for (int vx = -500; vx < 500; vx++)
            {
                for (int vy = -500; vy < 500; vy++)
                {
                    if (vy == 0 || vx == 0)
                    {
                        continue;
                    }

                    Point breakingVelocity{ static_cast<double>(vx), static_cast<double>(vy), 0 };
                    bool failed{ false };
                    bool intersectFound{ false };
                    Point intersectPoint{};
                    long long pairsSucceeded{};

                    for (size_t i = 0; i < stones.size(); i++)
                    {
                        for (size_t j = i + 1; j < stones.size(); j++)
                        {
                            auto a{ stones[i] };
                            auto b{ stones[j] };

                            // Adjust velocities by possible breaking stone velocity
                            a.velocity = a.velocity - breakingVelocity;
                            b.velocity = b.velocity - breakingVelocity;

                            if (a.isFutureCoincidentWithOnXY(b))
                            {
                                // annoying case, for now assume it hits the general intercept point
                                pairsSucceeded++;
                                continue;
                            }

                            const auto [exists, p] { a.futureIntersectPointInTwoDimensions(b) };

                            if (!exists)
                            {
                                failed = true;
                                break;
                            }

                            if (!intersectFound)
                            {
                                intersectFound = true;
                                intersectPoint = p;
                                pairsSucceeded++;
                                continue;
                            }

                            // Ran into significant issue with double accuracy here...
                            if (doublesAreSignificantlyDifferent(intersectPoint.x, p.x)
                                || doublesAreSignificantlyDifferent(intersectPoint.y, p.y))
                            {
                                failed = true;
                                break;
                            }

                            pairsSucceeded++;
                        }

                        if (failed)
                        {
                            break;
                        }
                    }

                    if (!failed)
                    {
                        possibleXYVelocity.emplace_back(std::pair{ vx, vy }, intersectPoint);
                    }
                }
            }

            std::vector<std::pair<std::pair<double, double>, Point>> possibleXZVelocity{};

            for (int vx = -500; vx < 500; vx++)
            {
                for (int vz = -500; vz < 500; vz++)
                {
                    if (vz == 0 || vx == 0)
                    {
                        continue;
                    }

                    Point breakingVelocity{ static_cast<double>(vx), 0, static_cast<double>(vz) };
                    bool failed{ false };
                    bool intersectFound{ false };
                    Point intersectPoint{};
                    long long pairsSucceeded{};

                    for (size_t i = 0; i < stones.size(); i++)
                    {
                        for (size_t j = i + 1; j < stones.size(); j++)
                        {
                            auto a{ stones[i] };
                            auto b{ stones[j] };

                            // Adjust velocities by possible breaking stone velocity
                            a.velocity = a.velocity - breakingVelocity;
                            b.velocity = b.velocity - breakingVelocity;

                            if (a.isFutureCoincidentWithOnXZ(b))
                            {
                                // annoying case, for now assume it hits the general intercept point
                                pairsSucceeded++;
                                continue;
                            }

                            const auto [exists, p] { a.futureIntersectPointInTwoDimensionsOnXAndZ(b) };

                            if (!exists)
                            {
                                failed = true;
                                break;
                            }

                            if (!intersectFound)
                            {
                                intersectFound = true;
                                intersectPoint = p;
                                pairsSucceeded++;
                                continue;
                            }

                            // Running into significant issue with double accuracy here...
                            if (doublesAreSignificantlyDifferent(intersectPoint.x, p.x)
                                || doublesAreSignificantlyDifferent(intersectPoint.z, p.z))
                            {
                                failed = true;
                                break;
                            }

                            pairsSucceeded++;
                        }

                        if (failed)
                        {
                            break;
                        }
                    }

                    if (!failed)
                    {
                        possibleXZVelocity.emplace_back(std::pair{ vx, vz }, intersectPoint);
                    }
                }
            }

            assert(possibleXYVelocity.size() == 1);
            assert(possibleXZVelocity.size() == 1);
            assert(!doublesAreSignificantlyDifferent(possibleXYVelocity[0].first.first, possibleXZVelocity[0].first.first));
            assert(!doublesAreSignificantlyDifferent(possibleXYVelocity[0].second.x, possibleXZVelocity[0].second.x));

            Point solutionVelocity{ possibleXYVelocity[0].first.first, possibleXYVelocity[0].first.second, possibleXZVelocity[0].first.first };
            Point solutionOrigin{ possibleXYVelocity[0].second.x, possibleXYVelocity[0].second.y, possibleXZVelocity[0].second.z };

            double sumOfCoordinates{ solutionOrigin.x + solutionOrigin.y + solutionOrigin.z };
            return static_cast<long long>(sumOfCoordinates);
        }
    };

    long long solvePart1(std::ifstream& file, bool example)
    {
        Storm storm{};
        storm.parseInput(file);
        return storm.countInterectionsInArea(example ? 7.0 : 200000000000000.0, example ? 27.0 : 400000000000000.0);
    }

    long long solvePart2(std::ifstream& file)
    {
        Storm storm{};
        storm.parseInput(file);
        return storm.solveBreakingThrow();
    }
}