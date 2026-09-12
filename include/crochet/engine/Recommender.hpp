#pragma once

#include <vector>
#include "crochet/models/Pattern.hpp"
#include "crochet/models/Preferences.hpp"
#include "crochet/engine/RecommendationResult.hpp"
#include <string>

namespace crochet
{

    class Recommender
    {
    public:
        std::vector<RecommendationResult> recommend(
            const std::vector<Pattern> &patterns,
            const Preferences &prefs);

    private:
        bool passesHardFilters(const Pattern &p, const Preferences &prefs);
        double scorePattern(const Pattern &p, const Preferences &prefs);
        std::string explainScore(const Pattern &p, const Preferences &prefs);
    };

} // namespace crochet