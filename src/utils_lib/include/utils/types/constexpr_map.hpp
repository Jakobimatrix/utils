/**
 * @file constexpr_map.hpp
 * @brief Contains a std::map like structure which can be used in a constexpr context.
 * Source: Jason Turner, C++ Weekly Ep 223 on Youtube: https://www.youtube.com/watch?v=INn3xa4pMfg&list=WL&index=8&t=0s
 *
 * @version 1.0
 * @date 2021
 */

#pragma once

#include <array>
#include <cstddef>
#include <utility>
#include <stdexcept>
#include <algorithm>

namespace util {

template <typename Key, typename Value, std::size_t Size>
struct Map {
  std::array<std::pair<Key, Value>, Size> data;

  [[nodiscard]] constexpr Value at(const Key& key) const {
    const auto itr = std::find_if(begin(data), end(data), [&key](const auto& value) {
      return value.first == key;
    });
    if (itr != end(data)) {
      return itr->second;
    }
    throw std::range_error("Not Found");
  }
};

}  // namespace util
