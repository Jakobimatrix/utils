/**
 * @file test_searchAndReplace.cpp
 * @brief contains tests for the search and replace functionality in utils_lib/string/searchAndReplace.hpp
 *
 * @date 29.05.2025
 * @author Jakob Wandel
 * @version 1.0
 **/


#include <catch2/catch_test_macros.hpp>
#include <utils/string/searchAndReplace.hpp>
#include <string>

TEST_CASE("Basic replacement") {
  std::string s = "hello world";
  util::replaceSubstring(&s, "world", "there");
  REQUIRE(s == "hello there");
}

TEST_CASE("No match found") {
  std::string s = "hello world";
  util::replaceSubstring(&s, "foo", "bar");
  REQUIRE(s == "hello world");
}

TEST_CASE("Empty string input") {
  std::string s = "";
  util::replaceSubstring(&s, "a", "b");
  REQUIRE(s == "");
}

TEST_CASE("Replace with empty string") {
  std::string s = "abcabc";
  util::replaceSubstring(&s, "b", "");
  REQUIRE(s == "acac");
}

TEST_CASE("Replace empty substring (should do nothing)") {
  std::string s = "abc";
  util::replaceSubstring(&s, "", "x");
  REQUIRE(s == "abc");
}

TEST_CASE("Multiple occurrences") {
  std::string s = "foo bar foo bar";
  util::replaceSubstring(&s, "foo", "baz");
  REQUIRE(s == "baz bar baz bar");
}

TEST_CASE("Overlapping substrings") {
  std::string s = "aaa";
  util::replaceSubstring(&s, "aa", "b");
  REQUIRE(s == "ba");
}

TEST_CASE("Case sensitivity") {
  std::string s = "Hello hello";
  util::replaceSubstring(&s, "hello", "hi");
  REQUIRE(s == "Hello hi");
}

TEST_CASE("Start position skips initial matches") {
  std::string s = "abcabcabc";
  util::replaceSubstring(&s, "abc", "x", 3);
  REQUIRE(s == "abcxx");
}

TEST_CASE("Replace substring with itself (should not loop infinitely)") {
  std::string s = "repeat";
  util::replaceSubstring(&s, "repeat", "repeat");
  REQUIRE(s == "repeat");
}
