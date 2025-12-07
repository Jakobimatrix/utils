/**
 * @file range.hpp
 * @brief Contains a Container/class that represents a range [min, max]. It is mostly to combat clang-tidy warning bugprone-easily-swappable-parameters but it also hase some usefull functions.
 *
 * @author Jakob Wandel
 * @date 30.08.2025
 */

#pragma once

#include <cassert>
#include <limits>
#include <utility>
#include <algorithm>

namespace util {

/**
 * @brief Class to hold two values one bigger than the other.
 * @tparam The underlying type.
 */
template <typename T>
class Range {
  T min{std::numeric_limits<T>::max()};
  T max{std::numeric_limits<T>::lowest()};

 public:
  T getMin() const { return min; }

  T getMax() const { return max; }

  void setMin(T newMin) {
    min = newMin;
    if (min > max) {
      std::swap(min, max);
    }
  }

  void setMax(T newMax) {
    max = newMax;
    if (min > max) {
      std::swap(min, max);
    }
  }

  /**
   * A default constructed Range is considered to be empty.
   */
  constexpr Range() = default;

  /**
   * Constructor for a Range. If you want an empty Range, use the default
   * constructor. Inputs will be swapped if lowerBoder > higherBorder.
   * @param lowerBoder The smaller value of the Range.
   * @param higherBorder The bigger value of the Range.
   */
  /// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) The class takes care of this
  constexpr Range(T lowerBoder, T higherBorder)
      : min(lowerBoder),
        max(higherBorder) {
    if (min > max) {
      std::swap(min, max);
    }
  }

  /**
   * Moves min and max about delta equaly in opposite directions. A Positive
   * delta widens the Range, a negative delta reduces the Range.
   * @param delta The delta to move min and max.
   */
  void addDelta(const T delta) {
    if (delta < 0 && -delta > getDistance()) {
      // empty Range
      max = getDistance() / static_cast<T>(2.) + min;
      min = max;
      return;
    }
    max += delta;
    min -= delta;
  }

  /**
   * Moves min and max about delta equaly in the same direction. A Positive
   * delta moves the Range to the right, a negative to the left.
   * @param delta The delta to move min and max.
   */
  void moveDelta(const T delta) {
    max += delta;
    min += delta;
  }

  /**
   * If the Range is default constructed or contains a NaN value, it is
   * considered to be empty.
   * @return True if the Range was default constructed.
   */
  [[nodiscard]] constexpr bool isEmpty() const { return min > max; }

  /**
   * If the Ranges boarders are so close to each other that it becomes a single point, it is called degenerate.
   * @param epsilon The epsilon (for example raster delta) to check if smaller than.
   * @return True if the Range is degenerate.
   */
  [[nodiscard]] constexpr bool isDegenerate(const T epsilon) const {
    return epsilon >= getDistance();
  }

  /**
   * Calculates the difference between min and max
   * @return The difference between min and max.
   */
  [[nodiscard]] constexpr T getDistance() const { return max - min; }

  /**
   * Calculates the center of the Range.
   * @return The center of the Range.
   */
  [[nodiscard]] constexpr T getCenter() const {
    return (max + min) / static_cast<T>(2.);
  }

  /**
   * clamp a value between min and max
   * @param value The value to be clamped.
   * @return The value between min and max or min or max. If Range is empty,
   * return NaN
   */
  [[nodiscard]] constexpr T clamp(T value) const {
    if (isEmpty()) {
      assert(false && "range is empty");
      if constexpr (std::is_floating_point_v<T>) {
        return std::numeric_limits<T>::quiet_NaN();
      } else {
        return std::numeric_limits<T>::max();  // I dont know...
      }
    }
    if (value < min) {
      value = min;
    } else if (value > max) {
      value = max;
    }
    return value;
  }

  /**
   * Calculates a scaled Value between min and max such that min corresponds to
   * 0 and max corresponds to 1.
   * You can scale back by using @see bom::Range::scaleBack()
   * @param value The value to be scaled between min and max.
   * @return The scaled version of the input.
   */
  [[nodiscard]] constexpr T scale01(T value) const {
    return (value - min) / getDistance();
  }

  /**
   * Calculates a the Value between min and max given scaledValue wich was scaled between 0 and 1.
   * @see bom::Range::scale01()
   * @param scaledValue The value to be scaled between min and max.
   * @return The unscaled version of the input.
   */
  [[nodiscard]] constexpr T scaleBack(T scaledValue) const {
    return (scaledValue * getDistance()) + min;
  }

  /**
   * Check if a value is between min and max
   * @param value the value to be checked
   * @return true if min < value < max
   */
  [[nodiscard]] constexpr bool operator()(const T value) const {
    assert(min <= max);
    return min < value && value < max;
  }

  /**
   * Check if a value is between or equal to min and max.
   * @param value the value to be checked.
   * @return True if min <= value <= max.
   */
  [[nodiscard]] constexpr bool operator[](const T value) const {
    assert(min <= max);
    return min <= value && value <= max;
  }

  [[nodiscard]] constexpr bool operator!=(const Range<T>& value) const {
    return min != value.min || value.max != max;
  }

  [[nodiscard]] constexpr bool operator==(const Range<T>& value) const {
    return !(*this != value);
  }

  /**
   * Check if a other Range overlaps with this.
   * @param other The other Range
   * @return True if both Ranges overlap
   */
  [[nodiscard]] constexpr bool overlaps(const Range<T>& other) const {
    return !static_cast<bool>(max < other.min || other.max < min);
  }

  /**
   * Check Calculate the common Range.
   * @param other The other Range
   * @return Return the common Range. If both Ranges don't overlap an
   * empty Range is returned.
   */
  [[nodiscard]] constexpr Range<T> commonRange(const Range<T>& other) const {
    if (!overlaps(other)) {
      return Range<T>();
    }

    T commonMin = std::max(min, other.min);
    T commonMax = std::min(max, other.max);

    return Range<T>(commonMin, commonMax);
  }

  /**
   * Check if this Range is inside of another range.
   * @param other The other Range
   * @return True if this Range is inside the other.
   */
  [[nodiscard]] constexpr bool isInsideOf(const Range<T>& other) const {
    return min > other.min && max < other.max;
  }
};

}  // namespace util
