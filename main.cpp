#include <iostream>
#include "crochet/data/PatternRepository.hpp"

int main()
{
    crochet::PatternRepository repo;

    try
    {
        auto patterns = repo.loadFromFile("../data/patterns.json");
        std::cout << "Loaded " << patterns.size() << " patterns:\n";
        for (const auto &p : patterns)
        {
            std::cout << " - " << p.name << " (" << p.estimatedTimeHours << " hrs)\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to load patterns: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}