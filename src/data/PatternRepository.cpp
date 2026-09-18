#include "crochet/data/PatternRepository.hpp"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace crochet
{

    Difficulty parseDifficulty(const std::string &s)
    {
        if (s == "Beginner")
            return Difficulty::Beginner;
        if (s == "Intermediate")
            return Difficulty::Intermediate;
        if (s == "Advanced")
            return Difficulty::Advanced;
        throw std::runtime_error("Unknown difficulty: " + s);
    }

    ProjectType parseProjectType(const std::string &s)
    {
        if (s == "Amigurumi")
            return ProjectType::Amigurumi;
        if (s == "Clothing")
            return ProjectType::Clothing;
        if (s == "HomeDecor")
            return ProjectType::HomeDecor;
        if (s == "Accessories")
            return ProjectType::Accessories;
        return ProjectType::Other;
    }

    std::vector<Pattern> PatternRepository::loadFromFile(const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open file: " + filepath);
        }

        nlohmann::json data = nlohmann::json::parse(file);

        std::vector<Pattern> patterns;
        for (const auto &item : data)
        {
            Pattern p;
            p.id = item.at("id").get<int>();
            p.name = item.at("name").get<std::string>();
            p.category = parseProjectType(item.at("category").get<std::string>());
            p.difficulty = parseDifficulty(item.at("difficulty").get<std::string>());
            p.estimatedTimeHours = item.at("estimatedTimeHours").get<double>();
            p.tags = item.at("tags").get<std::vector<std::string>>();
            p.instructionSteps = item.at("instructionSteps").get<std::vector<std::string>>();
            patterns.push_back(p);
        }

        return patterns;
    }

} // namespace crochet