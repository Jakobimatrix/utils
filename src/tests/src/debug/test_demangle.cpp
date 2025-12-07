/**
 * @file test_demangle.cpp
 * @brief contains tests for demangling types
 * @date 2025.08.24
 * @author Jakob Wandel
 **/

#include <catch2/catch_test_macros.hpp>

#include <utils/debug/demangle.hpp>

#include <string>


TEST_CASE("demangle fundamental types", "[demangle]") {
  using util::demangle;

  REQUIRE(demangle(typeid(int).name()) == "int");
  REQUIRE(demangle(typeid(double).name()) == "double");
  REQUIRE(demangle(typeid(char).name()) == "char");
}

TEST_CASE("std::string", "[demangle]") {
  using util::demangle;

  REQUIRE(demangle(typeid(std::string).name()) == "std::string");
}

TEST_CASE("demangle pointers", "[demangle]") {
  using util::demangle;

  REQUIRE(demangle(typeid(int*).name()) == "int*");
  REQUIRE(demangle(typeid(std::string*).name()) == "std::string*");
}
TEST_CASE("demangle unknown type", "[demangle]") {
  using util::demangle;

  const char* unknown = "mangled_name";
  REQUIRE(demangle(unknown) == std::string(unknown));
}
