#include <iostream>
#include "crochet/data/PatternRepository.hpp"
#include "crochet/engine/Recommender.hpp"

int main()
{
    crochet::PatternRepository repo;

    try
    {
        auto patterns = repo.loadFromFile("../data/patterns.json");

        crochet::Preferences prefs;
        prefs.desiredDifficulty = crochet::Difficulty::Intermediate;
        prefs.desiredType = crochet::ProjectType::HomeDecor;
        prefs.desiredStyles = {"floral", "colorful"};
        prefs.desiredColors = {};
        prefs.maxTimeHours = 25.0;
        prefs.weights = {{"style", 2.0}, {"time", 1.0}};

        crochet::Recommender recommender;
        auto results = recommender.recommend(patterns, prefs);

        std::cout << "Found " << results.size() << " matching patterns:\n";
        for (const auto &r : results)
        {
            std::cout << " - " << r.pattern.name
                      << " | score: " << r.score
                      << " | " << r.explanation << "\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}