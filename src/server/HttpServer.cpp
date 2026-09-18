#include "crochet/server/HttpServer.hpp"
#include "crochet/data/JsonConverters.hpp"
#include <httplib.h>
#include <algorithm>
#include <iostream>

namespace crochet
{

    HttpServer::HttpServer(std::vector<Pattern> patterns)
        : patterns_(std::move(patterns)),
          recommender_(std::make_unique<Recommender>())
    {
        try
        {
            geminiClient_ = std::make_unique<GeminiClient>();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Gemini client unavailable: " << e.what() << std::endl;
            std::cerr << "AI features will be disabled." << std::endl;
        }
    }

    void HttpServer::run(int port)
    {
        httplib::Server svr;
        svr.set_default_headers({{"Access-Control-Allow-Origin", "*"}});
        svr.Options(R"(.*)", [](const httplib::Request &, httplib::Response &res)
                    {
            res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.status = 200; });

        svr.Post("/recommend", [this](const httplib::Request &req, httplib::Response &res)
                 {
        try {
            auto body = nlohmann::json::parse(req.body);
            Preferences prefs = jsonToPreferences(body);

            auto results = recommender_->recommend(patterns_, prefs);

            nlohmann::json response = nlohmann::json::array();
            for (const auto& r : results) {
                response.push_back(resultToJson(r));
            }

            res.set_content(response.dump(), "application/json");
        } catch (const nlohmann::json::exception& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", "Invalid JSON: " + std::string(e.what())}}.dump(),
                "application/json"
            );
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", e.what()}}.dump(),
                "application/json"
            );
        } });

        svr.Post("/variation", [this](const httplib::Request &req, httplib::Response &res)
                 {
        if (!geminiClient_) {
            res.status = 503;
            res.set_content(nlohmann::json{{"error", "AI features are not configured"}}.dump(), "application/json");
            return;
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            int patternId = body.at("patternId").get<int>();
            Preferences prefs = jsonToPreferences(body.at("preferences"));

            auto it = std::find_if(patterns_.begin(), patterns_.end(),
                [patternId](const Pattern& p) { return p.id == patternId; });

            if (it == patterns_.end()) {
                res.status = 404;
                res.set_content(nlohmann::json{{"error", "Pattern not found"}}.dump(), "application/json");
                return;
            }

            auto variation = geminiClient_->generateVariation(*it, prefs);
            if (!variation) {
                res.status = 502;
                res.set_content(nlohmann::json{{"error", "AI service temporarily unavailable"}}.dump(), "application/json");
                return;
            }

            res.set_content(nlohmann::json{{"variation", *variation}}.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(nlohmann::json{{"error", e.what()}}.dump(), "application/json");
        } });

        svr.Get("/patterns", [this](const httplib::Request &, httplib::Response &res)
                {
        nlohmann::json response = nlohmann::json::array();
        for (const auto& p : patterns_) {
            response.push_back(patternToJson(p));
        }
        res.set_content(response.dump(), "application/json"); });

        svr.Post("/ideas", [this](const httplib::Request &req, httplib::Response &res)
                 {
        if (!geminiClient_) {
            res.status = 503;
            res.set_content(nlohmann::json{{"error", "AI features are not configured"}}.dump(), "application/json");
            return;
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            Preferences prefs = jsonToPreferences(body);
            int count = body.value("count", 3);

            auto ideas = geminiClient_->generateIdeas(prefs, count);
            if (!ideas) {
                res.status = 502;
                res.set_content(nlohmann::json{{"error", "AI service temporarily unavailable"}}.dump(), "application/json");
                return;
            }

            nlohmann::json response = nlohmann::json::array();
            for (const auto& idea : *ideas) {
                response.push_back(ideaToJson(idea));
            }
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(nlohmann::json{{"error", e.what()}}.dump(), "application/json");
        } });

        std::cout << "Crochet++ server running on http://localhost:" << port << std::endl;
        svr.listen("0.0.0.0", port);
    }

} // namespace crochet