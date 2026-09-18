#pragma once

#include <string>
#include <vector>
#include <optional>
#include "crochet/models/Pattern.hpp"
#include "crochet/models/Preferences.hpp"

namespace crochet
{

    struct GeneratedIdea
    {
        std::string name;
        std::string description;
    };

    class GeminiClient
    {
    public:
        GeminiClient();
        std::optional<std::string> generateVariation(const Pattern &pattern, const Preferences &prefs);
        std::optional<std::vector<GeneratedIdea>> generateIdeas(const Preferences &prefs, int count);

    private:
        std::string apiKey_;
        std::optional<std::string> callGeminiApi(const std::string &prompt, int retriesLeft = 2);
    };

} // namespace crochet