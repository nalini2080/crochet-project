#include "crochet/ai/GeminiClient.hpp"
#include "crochet/data/JsonConverters.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

namespace crochet
{

    static size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *out)
    {
        size_t totalSize = size * nmemb;
        out->append(static_cast<char *>(contents), totalSize);
        return totalSize;
    }

    GeminiClient::GeminiClient()
    {
        const char *key = std::getenv("GEMINI_API_KEY");
        if (key == nullptr)
        {
            throw std::runtime_error("GEMINI_API_KEY environment variable is not set");
        }
        apiKey_ = key;
    }

    std::optional<std::string> GeminiClient::callGeminiApi(const std::string &prompt, int retriesLeft)
    {
        CURL *curl = curl_easy_init();
        if (!curl)
        {
            return std::nullopt;
        }

        std::string url = "https://generativelanguage.googleapis.com/v1beta/models/"
                          "gemini-3.6-flash:generateContent?key=" +
                          apiKey_;

        nlohmann::json requestBody = {
            {"contents", {{{"parts", {{{"text", prompt}}}}}}}};
        std::string requestBodyStr = requestBody.dump();

        std::string responseBuffer;
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBodyStr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        CURLcode res = curl_easy_perform(curl);

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "Gemini API request failed: " << curl_easy_strerror(res) << std::endl;
            return std::nullopt;
        }

        if (httpCode == 503 && retriesLeft > 0)
        {
            std::cerr << "Gemini API overloaded, retrying (" << retriesLeft << " left)..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            return callGeminiApi(prompt, retriesLeft - 1);
        }

        if (httpCode != 200)
        {
            std::cerr << "Gemini API returned HTTP " << httpCode << ": " << responseBuffer << std::endl;
            return std::nullopt;
        }

        try
        {
            auto responseJson = nlohmann::json::parse(responseBuffer);
            return responseJson.at("candidates").at(0).at("content").at("parts").at(0).at("text").get<std::string>();
        }
        catch (const nlohmann::json::exception &e)
        {
            std::cerr << "Failed to parse Gemini response: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<std::string> GeminiClient::generateVariation(const Pattern &pattern, const Preferences &prefs)
    {
        std::ostringstream prompt;
        prompt << "Create a beginner-friendly variation description for this crochet pattern.\n"
               << "Pattern name: " << pattern.name << "\n"
               << "Difficulty: " << difficultyToString(pattern.difficulty) << "\n"
               << "Tags: ";
        for (const auto &tag : pattern.tags)
            prompt << tag << " ";
        prompt << "\nUser's preferred colors: ";
        for (const auto &color : prefs.desiredColors)
            prompt << color << " ";
        prompt << "\nWrite a short, friendly 2-3 sentence variation suggestion.";

        return callGeminiApi(prompt.str());
    }

    std::optional<std::vector<GeneratedIdea>> GeminiClient::generateIdeas(const Preferences &prefs, int count)
    {
        std::ostringstream prompt;
        prompt << "Suggest " << count << " short, original crochet project ideas.\n"
               << "Project type: " << projectTypeToString(prefs.desiredType) << "\n"
               << "Difficulty: " << difficultyToString(prefs.desiredDifficulty) << "\n"
               << "Preferred styles: ";
        for (const auto &s : prefs.desiredStyles)
            prompt << s << " ";
        prompt << "\nBe concise. Respond with ONLY a raw JSON array, no markdown formatting, no code fences. "
               << "Each element must be an object with exactly three fields: "
               << "\"name\" (string), \"description\" (one short sentence), and "
               << "\"instructionSteps\" (an array of exactly 4 brief steps, one sentence each).";

        auto raw = callGeminiApi(prompt.str());
        if (!raw)
        {
            return std::nullopt;
        }

        try
        {
            std::string text = *raw;
            auto start = text.find('[');
            auto end = text.rfind(']');
            if (start == std::string::npos || end == std::string::npos)
            {
                return std::nullopt;
            }
            std::string jsonSlice = text.substr(start, end - start + 1);

            auto parsed = nlohmann::json::parse(jsonSlice);
            std::vector<GeneratedIdea> ideas;
            for (const auto &item : parsed)
            {
                GeneratedIdea idea;
                idea.name = item.at("name").get<std::string>();
                idea.description = item.at("description").get<std::string>();
                idea.instructionSteps = item.value("instructionSteps", std::vector<std::string>{});
                ideas.push_back(idea);
            }
            return ideas;
        }
        catch (const nlohmann::json::exception &e)
        {
            std::cerr << "Failed to parse generated ideas: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

} // namespace crochet