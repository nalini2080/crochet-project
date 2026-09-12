#include "crochet/server/HttpServer.hpp"
#include "crochet/data/JsonConverters.hpp"
#include <httplib.h>
#include <iostream>

namespace crochet
{

    HttpServer::HttpServer(std::vector<Pattern> patterns)
        : patterns_(std::move(patterns)),
          recommender_(std::make_unique<Recommender>()) {}

    void HttpServer::run(int port)
    {
        httplib::Server svr;

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

        std::cout << "Crochet++ server running on http://localhost:" << port << std::endl;
        svr.listen("0.0.0.0", port);
    }

} // namespace crochet