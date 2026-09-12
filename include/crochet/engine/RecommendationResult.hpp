#pragma once

#include <string>
#include "crochet/models/Pattern.hpp"

namespace crochet
{

    struct RecommendationResult
    {
        Pattern pattern;
        double score;
        std::string explanation;
    };

} // namespace crochet