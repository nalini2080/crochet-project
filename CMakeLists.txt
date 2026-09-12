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

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
# Prevent GoogleTest from overriding our compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.3.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(httplib)

enable_testing()

add_executable(crochet main.cpp src/data/PatternRepository.cpp src/engine/Recommender.cpp src/server/HttpServer.cpp)
target_include_directories(crochet PRIVATE include)
target_link_libraries(crochet PRIVATE nlohmann_json::nlohmann_json httplib::httplib)

add_executable(crochet_tests
    tests/test_recommender.cpp
    src/engine/Recommender.cpp
)
target_include_directories(crochet_tests PRIVATE include)
target_link_libraries(crochet_tests PRIVATE gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(crochet_tests)