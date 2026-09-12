cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CrochetPlusPlus)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(json)

add_executable(crochet main.cpp src/data/PatternRepository.cpp src/engine/Recommender.cpp)
target_include_directories(crochet PRIVATE include)
target_link_libraries(crochet PRIVATE nlohmann_json::nlohmann_json)