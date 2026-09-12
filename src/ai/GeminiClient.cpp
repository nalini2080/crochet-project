#include "crochet/ai/GeminiClient.hpp"
#include "crochet/data/JsonConverters.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace crochet
{

    // libcurl calls this repeatedly as response data arrives; we accumulate it into a string
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

    std::optional<std::string> GeminiClient::callGeminiApi(const std::string &prompt)
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
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

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

} // namespace crochet