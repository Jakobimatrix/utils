/**
 * @file test_range.cpp
 * @brief contains tests for the Range class
 * @date 30.08.2025
 * @author Jakob Wandel
 **/

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <utils/types/range.hpp>


using util::Range;

// NOLINTBEGIN readability-magic-numbers Ya, very magic, and random...

TEST_CASE("Test initialization", "[range]") {
  SECTION("Default constructed is empty") {  // NOLINT misc-const-correctness
    const Range<int> range;
    REQUIRE(range.isEmpty());
  }

  SECTION("Construct with min < max") {  // NOLINT misc-const-correctness
    const Range<int> range(1, 5);
    REQUIRE(range.getMin() == 1);
    REQUIRE(range.getMax() == 5);
    REQUIRE_FALSE(range.isEmpty());
  }

  SECTION("Construct with min > max swaps automatically") {  // NOLINT misc-const-correctness
    const Range<int> range(10, 3);
    REQUIRE(range.getMin() == 3);
    REQUIRE(range.getMax() == 10);
  }

  SECTION("Degenerate range") {  // NOLINT misc-const-correctness
    const Range<int> range(5, 5);
    REQUIRE(range.isDegenerate(0));
    REQUIRE(range.getDistance() == 0);
  }
}

TEST_CASE("Test scale", "[range]") {
  const Range<double> range(0.0, 10.0);

  SECTION("scale01 maps correctly") {  // NOLINT misc-const-correctness
    REQUIRE(range.scale01(0.0) == Catch::Approx(0.0));
    REQUIRE(range.scale01(5.0) == Catch::Approx(0.5));
    REQUIRE(range.scale01(10.0) == Catch::Approx(1.0));
  }

  SECTION("scaleBack maps correctly") {  // NOLINT misc-const-correctness
    REQUIRE(range.scaleBack(0.0) == Catch::Approx(0.0));
    REQUIRE(range.scaleBack(0.5) == Catch::Approx(5.0));
    REQUIRE(range.scaleBack(1.0) == Catch::Approx(10.0));
  }

  SECTION("scale01 and scaleBack are inverses") {  // NOLINT misc-const-correctness
    const double v = 7.2;
    REQUIRE(range.scaleBack(range.scale01(v)) == Catch::Approx(v));
  }
}

TEST_CASE("Test operators", "[range]") {
  const Range<int> range(0, 10);

  SECTION("operator() excludes boundaries") {  // NOLINT misc-const-correctness
    REQUIRE(range(5));
    REQUIRE_FALSE(range(0));
    REQUIRE_FALSE(range(10));
  }

  SECTION("operator[] includes boundaries") {  // NOLINT misc-const-correctness
    REQUIRE(range[0]);
    REQUIRE(range[10]);
    REQUIRE(range[5]);
  }

  SECTION("Equality and inequality") {  // NOLINT misc-const-correctness
    const Range<int> range2(0, 10);
    const Range<int> range3(1, 9);
    REQUIRE(range == range2);
    REQUIRE(range != range3);
  }

  SECTION("Clamp inside and outside") {  // NOLINT misc-const-correctness
    REQUIRE(range.clamp(5) == 5);
    REQUIRE(range.clamp(-1) == 0);
    REQUIRE(range.clamp(15) == 10);
  }
}

TEST_CASE("Test range x range", "[range]") {  // NOLINT misc-const-correctness
  const Range<int> range1(0, 10);
  const Range<int> range2(5, 15);
  const Range<int> range3(11, 20);

  SECTION("Overlaps") {  // NOLINT misc-const-correctness
    REQUIRE(range1.overlaps(range2));
    REQUIRE_FALSE(range1.overlaps(range3));
  }

  SECTION("Common range") {  // NOLINT misc-const-correctness
    const auto common = range1.commonRange(range2);
    REQUIRE(common.getMin() == 5);
    REQUIRE(common.getMax() == 10);
  }

  SECTION("Empty common range when no overlap") {  // NOLINT misc-const-correctness
    const auto common = range1.commonRange(range3);
    REQUIRE(common.isEmpty());
  }

  SECTION("isInsideOf") {  // NOLINT misc-const-correctness
    const Range<int> outer(0, 20);
    const Range<int> inner(5, 10);
    REQUIRE(inner.isInsideOf(outer));
    REQUIRE_FALSE(outer.isInsideOf(inner));
  }
}

// NOLINTEND readability-magic-numbers
