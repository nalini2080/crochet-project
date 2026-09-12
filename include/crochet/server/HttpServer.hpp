#pragma once

#include <memory>
#include "crochet/data/PatternRepository.hpp"
#include "crochet/engine/Recommender.hpp"

#include "crochet/ai/GeminiClient.hpp"

namespace crochet
{

    class HttpServer
    {
    public:
        HttpServer(std::vector<Pattern> patterns);
        void run(int port);

    private:
        std::vector<Pattern> patterns_;
        std::unique_ptr<Recommender> recommender_;
        std::unique_ptr<GeminiClient> geminiClient_;
    };

} // namespace crochet