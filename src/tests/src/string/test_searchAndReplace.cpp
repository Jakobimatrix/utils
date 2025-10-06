/**
 * @file test_searchAndReplace.cpp
 * @brief contains tests for the search and replace functionality in utils_lib/string/searchAndReplace.hpp
 *
 * @date 29.05.2025
 * @author Jakob Wandel
 **/


#include <catch2/catch_test_macros.hpp>
#include <utils/string/searchAndReplace.hpp>
#include <string>

TEST_CASE("Basic replacement") {
  std::string stringVar = "hello world";
  util::replaceSubstring(&stringVar, "world", "there");
  REQUIRE(stringVar == "hello there");
}

TEST_CASE("No match found") {
  std::string stringVar = "hello world";
  util::replaceSubstring(&stringVar, "foo", "bar");
  REQUIRE(stringVar == "hello world");
}

TEST_CASE("Empty string input") {
  std::string stringVar;
  util::replaceSubstring(&stringVar, "a", "b");
  REQUIRE(stringVar.empty());
}

TEST_CASE("Replace with empty string") {
  std::string stringVar = "abcabc";
  util::replaceSubstring(&stringVar, "b", "");
  REQUIRE(stringVar == "acac");
}

TEST_CASE("Replace empty substring (should do nothing)") {
  std::string stringVar = "abc";
  util::replaceSubstring(&stringVar, "", "x");
  REQUIRE(stringVar == "abc");
}

TEST_CASE("Multiple occurrences") {
  std::string stringVar = "foo bar foo bar";
  util::replaceSubstring(&stringVar, "foo", "baz");
  REQUIRE(stringVar == "baz bar baz bar");
}

TEST_CASE("Overlapping substrings") {
  std::string stringVar = "aaa";
  util::replaceSubstring(&stringVar, "aa", "b");
  REQUIRE(stringVar == "ba");
}

TEST_CASE("Case sensitivity") {
  std::string stringVar = "Hello hello";
  util::replaceSubstring(&stringVar, "hello", "hi");
  REQUIRE(stringVar == "Hello hi");
}

TEST_CASE("Start position skips initial matches") {
  std::string stringVar = "abcabcabc";
  util::replaceSubstring(&stringVar, "abc", "x", 3);
  REQUIRE(stringVar == "abcxx");
}

TEST_CASE("Replace substring with itself (should not loop infinitely)") {
  std::string stringVar = "repeat";
  util::replaceSubstring(&stringVar, "repeat", "repeat");
  REQUIRE(stringVar == "repeat");
}
