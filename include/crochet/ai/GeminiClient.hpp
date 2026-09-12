#pragma once

#include <string>
#include <optional>
#include "crochet/models/Pattern.hpp"
#include "crochet/models/Preferences.hpp"

namespace crochet
{

    class GeminiClient
    {
    public:
        GeminiClient();
        std::optional<std::string> generateVariation(const Pattern &pattern, const Preferences &prefs);

    private:
        std::string apiKey_;
        std::optional<std::string> callGeminiApi(const std::string &prompt);
    };

} // namespace crochet