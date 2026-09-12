#include "crochet/engine/Recommender.hpp"
#include <algorithm>

namespace crochet
{

    bool Recommender::passesHardFilters(const Pattern &p, const Preferences &prefs)
    {
        if (p.category != prefs.desiredType)
        {
            return false;
        }
        if (p.difficulty > prefs.desiredDifficulty)
        {
            return false;
        }
        return true;
    }

    double Recommender::scorePattern(const Pattern &p, const Preferences &prefs)
    {
        double totalScore = 0.0;
        double totalWeight = 0.0;

        // Style match — only counts if the user actually specified desired styles
        if (!prefs.desiredStyles.empty())
        {
            double styleWeight = prefs.weights.count("style") ? prefs.weights.at("style") : 1.0;
            int styleMatches = 0;
            for (const auto &style : prefs.desiredStyles)
            {
                if (std::find(p.tags.begin(), p.tags.end(), style) != p.tags.end())
                {
                    styleMatches++;
                }
            }
            double styleScore = static_cast<double>(styleMatches) / prefs.desiredStyles.size();
            totalScore += styleScore * styleWeight;
            totalWeight += styleWeight;
        }

        // Time match — always relevant, since maxTimeHours always has a value
        double timeWeight = prefs.weights.count("time") ? prefs.weights.at("time") : 1.0;
        double timeScore = p.estimatedTimeHours <= prefs.maxTimeHours ? 1.0 : 0.0;
        totalScore += timeScore * timeWeight;
        totalWeight += timeWeight;

        return totalWeight > 0.0 ? totalScore / totalWeight : 0.0;
    }

    std::string Recommender::explainScore(const Pattern &p, const Preferences &prefs)
    {
        std::string explanation = "Matches your ";
        if (p.category == prefs.desiredType)
        {
            explanation += "project type";
        }
        return explanation + ".";
    }

    std::vector<RecommendationResult> Recommender::recommend(
        const std::vector<Pattern> &patterns,
        const Preferences &prefs)
    {
        std::vector<RecommendationResult> results;

        for (const auto &p : patterns)
        {
            if (!passesHardFilters(p, prefs))
            {
                continue;
            }
            RecommendationResult r;
            r.pattern = p;
            r.score = scorePattern(p, prefs);
            r.explanation = explainScore(p, prefs);
            results.push_back(r);
        }

        std::sort(results.begin(), results.end(),
                  [](const RecommendationResult &a, const RecommendationResult &b)
                  {
                      return a.score > b.score;
                  });

        return results;
    }

} // namespace crochet