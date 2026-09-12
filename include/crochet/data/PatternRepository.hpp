#pragma once

#include <string>
#include <vector>
#include "crochet/models/Pattern.hpp"

namespace crochet
{

    class PatternRepository
    {
    public:
        std::vector<Pattern> loadFromFile(const std::string &filepath);
    };

} // namespace crochet