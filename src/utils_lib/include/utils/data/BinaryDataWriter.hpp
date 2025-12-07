/**
 * @file BinaryDataWriter.hpp
 * @brief A class to write binary data in a safe manner.
 *
 * @date 03.10.2025
 */

#pragma once

#include <bit>
#include <algorithm>
#include <bitset>
#include <new>
#include <cstdint>
#include <cstring>
#include <deque>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <utils/data/BinaryDataBuffer.hpp>
#include <utils/debug/logging.hpp>
#include <utils/string/utf8Conversion.hpp>

namespace serialize {

class BinaryDataWriter : public BinaryDataBuffer {
 public:
  /**
   * @brief Constructs a binary data writer with size constraints
   * @param minExpectedSize The initial size to resize the m_buffer to
   * @param maxExpectedSize The maximum allowed size the m_buffer can grow to
   */
  BinaryDataWriter(uint32_t minExpectedSize, uint32_t maxExpectedSize, std::endian endian) noexcept
      : BinaryDataBuffer(endian),
        maxExpectedSize_(maxExpectedSize) {
    minExpectedSize = std::min(minExpectedSize, maxExpectedSize);
    m_buffer.resize(minExpectedSize);
    m_cursor = 0;
    ready    = false;
  }

  /**
   * @brief Creates a writer from a reader's m_buffer (or rather its parent)
   * @param reader The reader to take the m_buffer from
   * @return A new writer containing the reader's m_buffer
   */
  template <class BinaryDataBufferChild>
  static BinaryDataWriter fromReader(BinaryDataBufferChild&& reader) {
    auto data       = std::move(reader.releaseBuffer());
    auto size       = data.size();
    auto writer     = BinaryDataWriter(size, size, reader.getEndian());
    writer.m_buffer = std::move(data);
    writer.ready    = true;
    return writer;
  }

  /**
   * @brief Indicates that all data has been written and shrinks m_buffer to fit
   * @param shring_to_cursor_position Reduces the size of the intrnal memory (vector) to the current cursor position which is the number of written bits, if not tempered with.
   * @return true if successful, false if m_buffer is empty or this function was called already or
   */
  bool setWritingFinished(bool shring_to_cursor_position) noexcept {
    if (m_buffer.empty() || ready) {
      return false;
    }
    if (shring_to_cursor_position) {
      m_buffer.resize(m_cursor);
    }
    ready = true;
    return true;
  }

  /**
   * @brief Writes a string at the current m_cursor position
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
      std::memcpy(m_buffer.data() + m_cursor, value.data(), value.size());
      m_cursor += value.size();
    }
    return true;
  }

  /**
   * @brief Writes a wstring at the current m_cursor position
   * @param value The wstring to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  bool writeNext(const std::wstring& value) noexcept {

    std::string utf8_string;
    if (util::wstringToUtf8(value, &utf8_string)) {
      return writeNext(utf8_string);
    }
    return false;
  }

  /**
   * @brief Writes a std::tuple<Ts...> by writing each element in order.
   * @tparam Ts Types of tuple elements.
   * @param value The tuple to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename... Ts>
  bool writeNext(const std::tuple<Ts...>& value) noexcept {
    return writeTupleImpl(value, std::index_sequence_for<Ts...>{});
  }

 private:
  template <typename Tuple, std::size_t... Is>
  bool writeTupleImpl(const Tuple& tup, std::index_sequence<Is...>) noexcept {
    return (... && writeNext(std::get<Is>(tup)));
  }

 public:
  /**
   * @brief Writes a std::bitset<N> as the smallest fixed integer that fits (up to 64 bits).
   * @tparam N number of bits (must be <= 64)
   */
  template <std::size_t N>
    requires(N > 0 && N <= 64)
  bool writeNext(const std::bitset<N>& bits) noexcept {
    // choose storage size
    if constexpr (N <= 8) {
      uint8_t v = static_cast<uint8_t>(bits.to_ulong());
      return writeNext(v);
    } else if constexpr (N <= 16) {
      uint16_t v = static_cast<uint16_t>(bits.to_ulong());
      return writeNext(v);
    } else if constexpr (N <= 32) {
      uint32_t v = static_cast<uint32_t>(bits.to_ullong());
      return writeNext(v);
    } else {
      uint64_t v = static_cast<uint64_t>(bits.to_ullong());
      return writeNext(v);
    }
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
      return false;
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
   * @brief Writes a std::array<T, N> by writing each element in order.
   * @tparam T Type of elements in the array
   * @tparam N Number of elements in the array
   * @param value Pointer to the array to write
   * @return true if successful, false if would exceed maxExpectedSize
   */
  template <typename T, std::size_t N>
  bool writeNext(const std::array<T, N>& value) noexcept {
    const size_t estimated_size = estimateContainerSize(value);
    if (!resizeIfNeeded(estimated_size)) {
      return false;
    }
    for (std::size_t i = 0; i < N; ++i) {
      if (!writeNext(value[i])) {
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

    using IntType =
      std::conditional_t<std::is_floating_point_v<T>,
                         std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>,
                         typename detail::MakeUnsignedIfIntegral<T>::type>;

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

    uint8_t* dest = m_buffer.data() + m_cursor;

    constexpr size_t SIZE_BYTE{8};
    constexpr size_t BYTE_MASK{0xFF};

    if constexpr (sizeof(T) == 1) {
      *dest = static_cast<uint8_t>(bits);
    } else if (getEndian() == std::endian::native) {
      std::memcpy(dest, &bits, sizeof(T));
    } else {
      if (getEndian() == std::endian::little) {
        for (size_t i = 0; i < sizeof(T); ++i) {
          dest[i] = static_cast<uint8_t>((bits >> (i * SIZE_BYTE)) & BYTE_MASK);
        }
      } else {
        for (size_t i = 0; i < sizeof(T); ++i) {
          dest[i] =
            static_cast<uint8_t>((bits >> ((sizeof(T) - 1 - i) * SIZE_BYTE)) & BYTE_MASK);
        }
      }
    }

    m_cursor += sizeof(T);

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
   * @brief Resizes the m_buffer if needed for additional data
   * @param additional_size Size of data to be added
   * @return true if resize was successful or not needed, false if would exceed maxExpectedSize
   */
  bool resizeIfNeeded(size_t additional_size) noexcept {
    if (additional_size == 0) {
      return true;
    }

    if (m_cursor + additional_size > maxExpectedSize_) {
      dbg::error(CURRENT_SOURCE_LOCATION,
                 "Writing would exceed maxExpectedSize.");
      return false;
    }

    if (m_cursor + additional_size > m_buffer.size()) {
      try {
        m_buffer.resize(m_cursor + additional_size);
      } catch (const std::bad_alloc&) {
        dbg::error(CURRENT_SOURCE_LOCATION,
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
    size_t total = sizeof(typename Container::size_type);

    if constexpr (std::is_trivially_copyable_v<typename Container::value_type>) {
      total += container.size() * sizeof(typename Container::value_type);
    } else {
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
    size_t total = sizeof(typename Map::size_type);

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
