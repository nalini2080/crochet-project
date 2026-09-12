#include <gtest/gtest.h>
#include "crochet/engine/Recommender.hpp"

using namespace crochet;

// Helper to build a minimal valid Pattern for tests
Pattern makePattern(int id, ProjectType type, Difficulty diff, double hours,
                    std::vector<std::string> tags)
{
    Pattern p;
    p.id = id;
    p.name = "Test Pattern " + std::to_string(id);
    p.category = type;
    p.difficulty = diff;
    p.estimatedTimeHours = hours;
    p.tags = tags;
    return p;
}

TEST(RecommenderTest, FiltersOutWrongCategory)
{
    Recommender recommender;
    std::vector<Pattern> patterns = {
        makePattern(1, ProjectType::Amigurumi, Difficulty::Beginner, 2.0, {})};

    Preferences prefs;
    prefs.desiredType = ProjectType::HomeDecor;
    prefs.desiredDifficulty = Difficulty::Advanced;
    prefs.maxTimeHours = 10.0;

    auto results = recommender.recommend(patterns, prefs);

    EXPECT_TRUE(results.empty());
}

TEST(RecommenderTest, FiltersOutTooHardDifficulty)
{
    Recommender recommender;
    std::vector<Pattern> patterns = {
        makePattern(1, ProjectType::HomeDecor, Difficulty::Advanced, 2.0, {})};

    Preferences prefs;
    prefs.desiredType = ProjectType::HomeDecor;
    prefs.desiredDifficulty = Difficulty::Beginner;
    prefs.maxTimeHours = 10.0;

    auto results = recommender.recommend(patterns, prefs);

    EXPECT_TRUE(results.empty());
}

TEST(RecommenderTest, PassesWhenDifficultyIsEasierThanRequested)
{
    Recommender recommender;
    std::vector<Pattern> patterns = {
        makePattern(1, ProjectType::HomeDecor, Difficulty::Beginner, 2.0, {})};

    Preferences prefs;
    prefs.desiredType = ProjectType::HomeDecor;
    prefs.desiredDifficulty = Difficulty::Advanced;
    prefs.maxTimeHours = 10.0;

    auto results = recommender.recommend(patterns, prefs);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].pattern.id, 1);
}

TEST(RecommenderTest, EmptyDatasetReturnsEmptyResults)
{
    Recommender recommender;
    std::vector<Pattern> patterns;

    Preferences prefs;
    prefs.desiredType = ProjectType::HomeDecor;
    prefs.desiredDifficulty = Difficulty::Advanced;
    prefs.maxTimeHours = 10.0;

    auto results = recommender.recommend(patterns, prefs);

    EXPECT_TRUE(results.empty());
}

TEST(RecommenderTest, HigherStyleMatchScoresHigher)
{
    Recommender recommender;
    std::vector<Pattern> patterns = {
        makePattern(1, ProjectType::HomeDecor, Difficulty::Beginner, 2.0, {"floral"}),
        makePattern(2, ProjectType::HomeDecor, Difficulty::Beginner, 2.0, {})};

    Preferences prefs;
    prefs.desiredType = ProjectType::HomeDecor;
    prefs.desiredDifficulty = Difficulty::Advanced;
    prefs.desiredStyles = {"floral"};
    prefs.maxTimeHours = 10.0;
    prefs.weights = {{"style", 1.0}, {"time", 1.0}};

    auto results = recommender.recommend(patterns, prefs);

    ASSERT_EQ(results.size(), 2);
    // Results are sorted descending by score, so pattern 1 (matches style) should come first
    EXPECT_EQ(results[0].pattern.id, 1);
    EXPECT_GT(results[0].score, results[1].score);
}

TEST(RecommenderTest, ExceedsMaxTimeScoresLower)
{
    Recommender recommender;
    std::vector<Pattern> patterns = {
        makePattern(1, ProjectType::HomeDecor, Difficulty::Beginner, 50.0, {})};

    Preferences prefs;
    prefs.desiredType = ProjectType::HomeDecor;
    prefs.desiredDifficulty = Difficulty::Advanced;
    prefs.maxTimeHours = 10.0;
    prefs.weights = {{"time", 1.0}};

    auto results = recommender.recommend(patterns, prefs);

    ASSERT_EQ(results.size(), 1);
    EXPECT_DOUBLE_EQ(results[0].score, 0.0);
}