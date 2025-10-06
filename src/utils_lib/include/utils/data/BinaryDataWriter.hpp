/**
 * @file BinaryDataWriter.hpp
 * @brief A class to write binary data in a safe manner.
 *
 * @version 1.0
 * @date 03.10.2025
 */

#pragma once

#include <utils/data/BinaryDataBuffer.hpp>

#include <array>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <system_error>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include "utils/debug/logging.hpp"

namespace serialize {

class BinaryDataWriter : public BinaryDataBuffer {
 public:
  /**
   * @brief Constructs a binary data writer with size constraints
   * @param minExpectedSize The initial size to resize the buffer to
   * @param maxExpectedSize The maximum allowed size the buffer can grow to
   */
  BinaryDataWriter(uint32_t minExpectedSize, uint32_t maxExpectedSize, std::endian endian) noexcept
      : BinaryDataBuffer(endian),
        maxExpectedSize_(maxExpectedSize) {
    if (minExpectedSize > maxExpectedSize) {
      minExpectedSize = maxExpectedSize;
    }
    buffer.resize(minExpectedSize);
    cursor = 0;
    ready  = false;
  }

  /**
   * @brief Creates a writer from a reader's buffer (or rather its parent)
   * @param reader The reader to take the buffer from
   * @return A new writer containing the reader's buffer
   */
  template <class BinaryDataBufferChild>
  static BinaryDataWriter fromReader(BinaryDataBufferChild&& reader) {
    auto data     = std::move(reader.releaseBuffer());
    auto size     = data.size();
    auto writer   = BinaryDataWriter(size, size, reader.getEndian());
    writer.buffer = std::move(data);
    return writer;
  }

  /**
   * @brief Indicates that all data has been written and shrinks buffer to fit
   * @return true if successful, false if buffer is empty
   */
  bool setWritingFinished() noexcept {
    if (buffer.empty()) {
      return false;
    }
    buffer.shrink_to_fit();
    ready = true;
    return true;
  }

  /**
   * @brief Writes a string at the current cursor position
   * @param value The string to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  bool writeNext(const std::string& value) noexcept {

    if (!resizeIfNeeded(value.size() + 8)) {
      return false;
    }

    if (!writeNext(value.size())) {
      return false;
    }

    if (!value.empty()) {
      std::memcpy(buffer.data() + cursor, value.data(), value.size());
      cursor += value.size();
    }
    return true;
  }

  /**
   * @brief Writes an optional value
   * @tparam T The type contained in the optional
   * @param value The optional value to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename T>
  bool writeNext(const std::optional<T>& value) noexcept {
    if (!writeNext(value.has_value())) {
      return false;  // TODO write bool false!!!
    }
    if (value) {
      return writeNext(*value);
    }
    return true;
  }

  /**
   * @brief Writes a variant
   * @tparam Ts The types the variant can hold
   * @param value The variant to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename... Ts>
  bool writeNext(const std::variant<Ts...>& value) noexcept {
    const size_t index = value.index();
    if (!writeNext(index)) {
      return false;
    }
    return std::visit([this](const auto& v) { return this->writeNext(v); }, value);
  }

  /**
   * @brief Writes a pair of values
   * @tparam A Type of the first element
   * @tparam B Type of the second element
   * @param value The pair to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename A, typename B>
  bool writeNext(const std::pair<A, B>& value) noexcept {
    return writeNext(value.first) && writeNext(value.second);
  }

  /**
   * @brief Writes a vector of values
   * @tparam T Type of elements in the vector
   * @param value The vector to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename T>
  bool writeNext(const std::vector<T>& value) noexcept {
    const size_t estimated_size = estimateContainerSize(value);
    if (!resizeIfNeeded(estimated_size)) {
      return false;
    }
    if (!writeNext(value.size())) {
      return false;
    }
    for (const auto& element : value) {
      if (!writeNext(element)) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Writes a list of values
   * @tparam T Type of elements in the list
   * @param value The list to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename T>
  bool writeNext(const std::list<T>& value) noexcept {
    const size_t estimated_size = estimateContainerSize(value);
    if (!resizeIfNeeded(estimated_size)) {
      return false;
    }
    if (!writeNext(value.size())) {
      return false;
    }
    for (const auto& element : value) {
      if (!writeNext(element)) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Writes a deque of values
   * @tparam T Type of elements in the deque
   * @param value The deque to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename T>
  bool writeNext(const std::deque<T>& value) noexcept {
    const size_t estimated_size = estimateContainerSize(value);
    if (!resizeIfNeeded(estimated_size)) {
      return false;
    }
    if (!writeNext(value.size())) {
      return false;
    }
    for (const auto& element : value) {
      if (!writeNext(element)) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Writes a set of values
   * @tparam T Type of elements in the set
   * @param value The set to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename T>
  bool writeNext(const std::set<T>& value) noexcept {
    const size_t estimated_size = estimateContainerSize(value);
    if (!resizeIfNeeded(estimated_size)) {
      return false;
    }
    if (!writeNext(value.size())) {
      return false;
    }
    for (const auto& element : value) {
      if (!writeNext(element)) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Writes an unordered set of values
   * @tparam T Type of elements in the unordered_set
   * @param value The unordered_set to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename T>
  bool writeNext(const std::unordered_set<T>& value) noexcept {
    const size_t estimated_size = estimateContainerSize(value);
    if (!resizeIfNeeded(estimated_size)) {
      return false;
    }
    if (!writeNext(value.size())) {
      return false;
    }
    for (const auto& element : value) {
      if (!writeNext(element)) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Writes a map of key-value pairs
   * @tparam K Type of keys
   * @tparam V Type of values
   * @param value The map to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename K, typename V>
  bool writeNext(const std::map<K, V>& value) noexcept {
    const size_t estimated_size = estimateMapSize(value);
    if (!resizeIfNeeded(estimated_size)) {
      return false;
    }
    if (!writeNext(value.size())) {
      return false;
    }
    for (const auto& [key, val] : value) {
      if (!writeNext(key) || !writeNext(val)) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Writes an unordered map of key-value pairs
   * @tparam K Type of keys
   * @tparam V Type of values
   * @param value The unordered_map to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename K, typename V>
  bool writeNext(const std::unordered_map<K, V>& value) noexcept {
    const size_t estimated_size = estimateMapSize(value);
    if (!resizeIfNeeded(estimated_size)) {
      return false;
    }
    if (!writeNext(value.size())) {
      return false;
    }
    for (const auto& [key, val] : value) {
      if (!writeNext(key) || !writeNext(val)) {
        return false;
      }
    }
    return true;
  }


  /**
   * @brief Writes info about std::size_t always into a 64 bit type.
   * @return true if resize was successful or not needed, false if would exceed maxExpectedSize
   */
  template <typename T>
  bool writeNext(T size) noexcept
    requires(std::is_same_v<T, std::size_t> && sizeof(std::size_t) != sizeof(SizeType))
  {
    return writeNext(static_cast<SizeType>(size));
  }

  /**
   * @brief Write a trivially copyable enum value into the buffer in ENDIAN order.
   * @tparam enum type.
   * @param value The value to write.
   */
  template <typename T>
    requires(std::is_enum_v<T>)
  bool writeNext(T value) {
    return writeNext(static_cast<std::underlying_type_t<T>>(value));
  }

  /**
   * @brief Write a trivially copyable scalar value into the buffer in ENDIAN order.
   * @tparam T Integral or floating-point type.
   * @param value The value to write.
   */
  template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
  bool writeNext(T value) {
    using UnsignedT = std::make_unsigned_t<T>;
    using IntType =
      std::conditional_t<std::is_floating_point_v<T>, std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>, UnsignedT>;

    if (!resizeIfNeeded(sizeof(T))) {
      return false;
    }

    const IntType bits = [&]() {
      if constexpr (std::is_floating_point_v<T>) {
        return std::bit_cast<IntType>(value);
      } else {
        return static_cast<IntType>(value);
      }
    }();

    uint8_t* dest = buffer.data() + cursor;

    if constexpr (sizeof(T) == 1) {
      *dest = static_cast<uint8_t>(bits);
    } else if (getEndian() == std::endian::native) {
      std::memcpy(dest, &bits, sizeof(T));
    } else {
      if (getEndian() == std::endian::little) {
        for (size_t i = 0; i < sizeof(T); ++i) {
          dest[i] = static_cast<uint8_t>((bits >> (i * 8)) & 0xFF);
        }
      } else {
        for (size_t i = 0; i < sizeof(T); ++i) {
          dest[i] = static_cast<uint8_t>((bits >> ((sizeof(T) - 1 - i) * 8)) & 0xFF);
        }
      }
    }

    cursor += sizeof(T);

    return true;
  }

  // Special-case bool to avoid instantiating make_unsigned<bool> which is
  // ill-formed. We encode bool as a single byte (0 or 1).
  bool writeNext(bool value) {
    return writeNext<uint8_t>(static_cast<uint8_t>(value));
  }

  /**
   * @brief Write a serializable object into the buffer.
   * @tparam SerializableObject A object that implements bool serialize(BinaryDataWriter& writer);
   * @param value The value to write.
   */
  template <typename SerializableObject>
  bool writeNext(SerializableObject value) {
    return value.serialize(*this);
  }

 private:
  /**
   * @brief Resizes the buffer if needed for additional data
   * @param additional_size Size of data to be added
   * @return true if resize was successful or not needed, false if would exceed maxExpectedSize
   */
  bool resizeIfNeeded(size_t additional_size) noexcept {
    if (additional_size == 0) {
      return true;
    }

    if (cursor + additional_size > maxExpectedSize_) {
      log::error(CURRENT_SOURCE_LOCATION,
                 "Writing would exceed maxExpectedSize.");
      return false;
    }

    if (cursor + additional_size > buffer.size()) {
      try {
        buffer.resize(cursor + additional_size);
      } catch (const std::bad_alloc&) {
        log::error(CURRENT_SOURCE_LOCATION,
                   "Memory allocation failed while resizing buffer.");
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Estimates size needed for a trivially copyable type
   * @tparam T Type to estimate size for
   * @return Size in bytes needed to store T
   */
  template <typename T>
    requires std::is_trivially_copyable_v<T>
  static constexpr size_t estimateSize() noexcept {
    return sizeof(T);
  }

  /**
   * @brief Estimates size needed for a string
   * @param str String to estimate size for
   * @return Size in bytes needed to store the string
   */
  static size_t estimateSize(const std::string& str) noexcept {
    return str.size();
  }

  /**
   * @brief Estimates size needed for a wide string
   * @param str Wide string to estimate size for
   * @return Size in bytes needed to store the wide string
   */
  static size_t estimateSize(const std::wstring& str) noexcept {
    return str.size() * sizeof(wchar_t);
  }

  /**
   * @brief Estimates size needed for a container and its elements
   * @tparam Container Container type (vector, list, set, etc.)
   * @param container The container to estimate size for
   * @return Size in bytes needed to store the container
   */
  template <typename Container>
  size_t estimateContainerSize(const Container& container) noexcept {
    using SizeType = typename Container::size_type;
    size_t total   = sizeof(SizeType);  // Size of container's size

    if constexpr (std::is_trivially_copyable_v<typename Container::value_type>) {
      total += container.size() * sizeof(typename Container::value_type);
    } else {
      // For non-trivial types, we need to estimate each element
      for (const auto& element : container) {
        total += estimateElementSize(element);
      }
    }
    return total;
  }

  /**
   * @brief Estimates size needed for a map and its elements
   * @tparam Map Map type (map, unordered_map)
   * @param map The map to estimate size for
   * @return Size in bytes needed to store the map
   */
  template <typename Map>
  size_t estimateMapSize(const Map& map) noexcept {
    using SizeType = typename Map::size_type;
    size_t total   = sizeof(SizeType);  // Size of map's size

    for (const auto& [key, value] : map) {
      total += estimateElementSize(key);
      total += estimateElementSize(value);
    }
    return total;
  }

  /**
   * @brief Estimates size needed for various element types
   * @tparam T Type of element to estimate size for
   * @param element The element to estimate size for
   * @return Size in bytes needed to store the element
   */
  template <typename T>
  size_t estimateElementSize(const T& element) noexcept {
    if constexpr (std::is_trivially_copyable_v<T>) {
      return sizeof(T);
    } else if constexpr (std::is_same_v<T, std::string>) {
      return estimateSize(element);
    } else if constexpr (std::is_same_v<T, std::wstring>) {
      return estimateSize(element);
    } else if constexpr (requires {
                           typename T::key_type;
                           typename T::mapped_type;
                         }) {
      // Map-like containers
      return estimateMapSize(element);
    } else if constexpr (requires { typename T::value_type; }) {
      // Other containers
      return estimateContainerSize(element);
    } else {
      // For other types, make a conservative estimate
      return sizeof(T);
    }
  }

  uint32_t maxExpectedSize_;
};

}  // namespace serialize
