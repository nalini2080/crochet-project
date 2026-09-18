#pragma once

#include <string>
#include <vector>

namespace crochet
{

    enum class Difficulty
    {
        Beginner,
        Intermediate,
        Advanced
    };

    enum class ProjectType
    {
        Amigurumi,
        Clothing,
        HomeDecor,
        Accessories,
        Other
    };

    struct Pattern
    {
        int id;
        std::string name;
        ProjectType category;
        Difficulty difficulty;
        double estimatedTimeHours;
        std::vector<std::string> tags;
        std::vector<std::string> instructionSteps;
    };

} // namespace crochet
