#pragma once

#include <nlohmann/json.hpp>
#include "crochet/models/Pattern.hpp"
#include "crochet/models/Preferences.hpp"
#include "crochet/engine/RecommendationResult.hpp"

namespace crochet
{

    inline std::string difficultyToString(Difficulty d)
    {
        switch (d)
        {
        case Difficulty::Beginner:
            return "Beginner";
        case Difficulty::Intermediate:
            return "Intermediate";
        case Difficulty::Advanced:
            return "Advanced";
        }
        return "Unknown";
    }

    inline Difficulty stringToDifficulty(const std::string &s)
    {
        if (s == "Beginner")
            return Difficulty::Beginner;
        if (s == "Intermediate")
            return Difficulty::Intermediate;
        if (s == "Advanced")
            return Difficulty::Advanced;
        throw std::runtime_error("Unknown difficulty: " + s);
    }

    inline std::string projectTypeToString(ProjectType t)
    {
        switch (t)
        {
        case ProjectType::Amigurumi:
            return "Amigurumi";
        case ProjectType::Clothing:
            return "Clothing";
        case ProjectType::HomeDecor:
            return "HomeDecor";
        case ProjectType::Accessories:
            return "Accessories";
        case ProjectType::Other:
            return "Other";
        }
        return "Unknown";
    }

    inline ProjectType stringToProjectType(const std::string &s)
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

    inline nlohmann::json patternToJson(const Pattern &p)
    {
        return {
            {"id", p.id},
            {"name", p.name},
            {"category", projectTypeToString(p.category)},
            {"difficulty", difficultyToString(p.difficulty)},
            {"estimatedTimeHours", p.estimatedTimeHours},
            {"tags", p.tags},
            {"instructionSteps", p.instructionSteps}};
    }

    inline nlohmann::json resultToJson(const RecommendationResult &r)
    {
        return {
            {"pattern", patternToJson(r.pattern)},
            {"score", r.score},
            {"explanation", r.explanation}};
    }

    inline Preferences jsonToPreferences(const nlohmann::json &j)
    {
        Preferences prefs;
        prefs.desiredDifficulty = stringToDifficulty(j.at("desiredDifficulty").get<std::string>());
        prefs.desiredType = stringToProjectType(j.at("desiredType").get<std::string>());
        prefs.desiredStyles = j.value("desiredStyles", std::vector<std::string>{});
        prefs.desiredColors = j.value("desiredColors", std::vector<std::string>{});
        prefs.maxTimeHours = j.at("maxTimeHours").get<double>();
        if (j.contains("weights"))
        {
            for (auto &[key, value] : j.at("weights").items())
            {
                prefs.weights[key] = value.get<double>();
            }
        }
        return prefs;
    }
    inline nlohmann::json ideaToJson(const GeneratedIdea &idea)
    {
        return {
            {"name", idea.name},
            {"description", idea.description},
            {"instructionSteps", idea.instructionSteps}};
    }

} // namespace crochet