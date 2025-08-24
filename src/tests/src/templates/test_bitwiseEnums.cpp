/**
 * @file test_bitwiseEnums.cpp
 * @brief contains tests for the Bitwise Enums
 *
 * @date 2025
 * @author Jakob Wandel
 * @version 1.0
 **/


#include <catch2/catch_test_macros.hpp>

#include <utils/templates/BitwiseEnums.hpp>

#include <array>
#include <cstdint>

enum class DBG_LVL : uint8_t {
  NONE     = 0x0,
  INFO     = 0x1,
  WARN     = 0x2,
  ERROR    = 0x4,
  CRITICAL = 0x8
};

TEST_CASE("BitwiseEnums operators work correctly", "[BitwiseEnums]") {
  using T = DBG_LVL;
  using namespace enum_operations;

  constexpr T none     = T::NONE;
  constexpr T info     = T::INFO;
  constexpr T warn     = T::WARN;
  constexpr T error    = T::ERROR;
  constexpr T critical = T::CRITICAL;

  // SECTION("operator| and operator|=")
  {
    T combined = info | warn;
    REQUIRE(static_cast<uint8_t>(combined) == 0x3);

    combined |= error;
    REQUIRE(static_cast<uint8_t>(combined) == 0x7);
  }

  // SECTION("operator& and operator&=")
  {
    T combined = info | warn | error;  // 0x7
    REQUIRE(static_cast<uint8_t>(combined & warn) == 0x2);

    combined &= warn;
    REQUIRE(static_cast<uint8_t>(combined) == 0x2);
  }

  // SECTION("operator~")
  {
    constexpr T inverted = ~info;
    REQUIRE(static_cast<uint8_t>(inverted & info) == 0x0);
    REQUIRE(static_cast<uint8_t>(inverted & warn) == 0x2);
  }

  // SECTION("operator<< and operator<<=")
  {
    T shifted = info << 1;
    REQUIRE(static_cast<uint8_t>(shifted) == 0x2);

    shifted <<= 2;
    REQUIRE(static_cast<uint8_t>(shifted) == 0x8);
  }

  // SECTION("operator>> and operator>>=")
  {
    T shifted = error >> 2;
    REQUIRE(static_cast<uint8_t>(shifted) == 0x1);

    shifted >>= 1;
    REQUIRE(static_cast<uint8_t>(shifted) == 0x0);
  }

  // SECTION("isSet works correctly")
  {
    constexpr T mask = info | error;  // 0x5
    REQUIRE(isSet(mask, info));
    REQUIRE(!isSet(mask, warn));
    REQUIRE(isSet(mask, error));
    REQUIRE(!isSet(mask, critical));
  }

  // SECTION("all possible combinations")
  {
    constexpr std::array<T, 5> values = {{none, info, warn, error, critical}};
    for (const T lhs : values) {
      for (const T rhs : values) {
        const uint8_t lhsInt = static_cast<uint8_t>(lhs);
        const uint8_t rhsInt = static_cast<uint8_t>(rhs);

        REQUIRE(static_cast<uint8_t>(lhs | rhs) == (lhsInt | rhsInt));
        REQUIRE(static_cast<uint8_t>(lhs & rhs) == (lhsInt & rhsInt));
        REQUIRE(static_cast<uint8_t>(lhs << 1) == (lhsInt << 1));
        REQUIRE(static_cast<uint8_t>(lhs >> 1) == (lhsInt >> 1));
        REQUIRE(isSet(lhs | rhs, rhs) == static_cast<bool>((lhsInt | rhsInt) & rhsInt));
      }
    }
  }
}
