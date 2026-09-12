#include <iostream>
#include "crochet/data/PatternRepository.hpp"
#include "crochet/server/HttpServer.hpp"

int main()
{
    crochet::PatternRepository repo;

    try
    {
        auto patterns = repo.loadFromFile("../data/patterns.json");
        crochet::HttpServer server(patterns);
        server.run(8080);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}