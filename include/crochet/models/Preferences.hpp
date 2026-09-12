#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Pattern.hpp"

namespace crochet {

struct Preferences {
    Difficulty desiredDifficulty;
    ProjectType desiredType;
    std::vector<std::string> desiredStyles;
    std::vector<std::string> desiredColors;
    double maxTimeHours;
    std::unordered_map<std::string, double> weights;
};

} // namespace crochet
