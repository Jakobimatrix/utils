/**
 * @file test_variadicFunction.cpp
 * @brief contains tests for VariadicFunction class
 * @date 2025.08.24
 * @author Jakob Wandel
 * @version 1.0
 **/


#include <catch2/catch_test_macros.hpp>

#include <utils/templates/variadicFunction.hpp>

#include <string>

namespace {

// some helper functions
void noArgsFunc() {
  // does nothing, but should not crash
}

void intFunc(int& ref, int add) { ref += add; }

void stringFunc(std::string& out, const std::string& prefix, int number) {
  out = prefix + std::to_string(number);
}

}  // namespace

// NOLINTBEGIN (readability-magic-numbers) // this tests inwolves random numbers choosen by the gods of magic

TEST_CASE("VariadicFunction with no arguments", "[VariadicFunction]") {
  util::VariadicFunction<> variadic_function(noArgsFunc);
  REQUIRE_NOTHROW(variadic_function.call());  // should just run
}

TEST_CASE("VariadicFunction with integers", "[VariadicFunction]") {
  int value = 10;
  util::VariadicFunction<int&, int> variadic_function(intFunc, value, 5);

  variadic_function.call();
  REQUIRE(value == 15);  // ref should be modified
}

TEST_CASE("VariadicFunction with strings", "[VariadicFunction]") {
  std::string result;
  util::VariadicFunction<std::string&, std::string, int> variadic_function(
    stringFunc, result, "Test-", 42);

  variadic_function.call();
  REQUIRE(result == "Test-42");
}

TEST_CASE("Polymorphic behavior via base pointer", "[VariadicFunction]") {
  std::unique_ptr<util::VirtualCall> callPtr =
    std::make_unique<util::VariadicFunction<int&, int>>(intFunc, 5, 3);

  // Note: int is passed by value here, so effect is not visible externally,
  // but call() must still succeed through the base pointer.
  REQUIRE_NOTHROW(callPtr->call());
}

TEST_CASE("Multiple calls reuse arguments", "[VariadicFunction]") {
  int value = 1;
  util::VariadicFunction<int&, int> variadic_function(intFunc, value, 2);

  variadic_function.call();  // 1 + 2 = 3
  variadic_function.call();  // 3 + 2 = 5
  REQUIRE(value == 5);
}
// NOLINTEND (readability-magic-numbers)
