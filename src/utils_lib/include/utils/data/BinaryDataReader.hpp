/**
 * @file BinaryDataReader.hpp
 * @brief A class to interpret binary data in a save manner.Dont use this class inside a multithreaded environment! The cursor is mutable.
 *
 * @date 03.10.2025
 */

#pragma once

#include <algorithm>
#include <bit>
#include <bitset>
#include <iterator>
#include <cstddef>
#include <limits>
#include <new>
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

#include <utils/data/BinaryDataBuffer.hpp>
#include <utils/debug/logging.hpp>
#include <utils/string/utf8Conversion.hpp>


namespace serialize {

class BinaryDataReader : public BinaryDataBuffer {
 public:
  /**
   * @brief Creates a reader from a writers's buffer (or rather its parent)
   * @param writer The writer to take the buffer from
   * @return A new reader containing the writers's buffer
   */
  template <class BinaryDataBufferChild>
  static BinaryDataReader fromReader(BinaryDataBufferChild&& writer) {
    auto data = std::move(writer.releaseBuffer());
    return BinaryDataReader(std::move(data), writer.getEndian());
  }

  /**
   * @brief Reads a binary file into a vector of bytes.
   *
   * @tparam ByteType Must be either char, unsigned char, or std::uint8_t.
   * @param path Path to the file.
   * @param error_code Error code output (set if an error occurs).
   * @return std::vector<uint8_t> Contents of the file.
   */
  template <typename ByteType>
    requires(!std::is_const_v<ByteType> &&
             (std::same_as<ByteType, char> || std::same_as<ByteType, unsigned char> ||
              std::same_as<ByteType, signed char> || std::same_as<ByteType, std::uint8_t>))
  static std::vector<ByteType> readFileBinary(const std::filesystem::path& path,
                                              std::error_code& error_code) noexcept {
    error_code.clear();
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
      error_code = std::make_error_code(std::errc::no_such_file_or_directory);
      return {};
    }

    std::streamsize const size = file.tellg();

    if (size < 0) {
      error_code = std::make_error_code(std::errc::io_error);
      return {};
    }

    file.seekg(0, std::ios::beg);

    std::vector<ByteType> buffer(static_cast<size_t>(size));
    if constexpr (std::is_same<char, ByteType>()) {
      if (!file.read(buffer.data(), size)) {
        error_code = std::make_error_code(std::errc::io_error);
        return {};
      }
    } else {
      if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        // reinterpret_cast is required: std::ifstream::read takes char*,
        // and buffer.data() may be uint8_t* / unsigned char*, etc. This cast is safe and idiomatic.
        error_code = std::make_error_code(std::errc::io_error);
        return {};
      }
    }

    return buffer;
  }

  /**
   * @brief Constructor reads a binary file. Check with ready() if the file was read correctly.
   * @param path The path to the file to read
   */
  BinaryDataReader(const std::filesystem::path& path, std::endian endian) noexcept
      : BinaryDataBuffer(endian) {
    std::error_code error_code;
    m_buffer = BinaryDataReader::readFileBinary<uint8_t>(path, error_code);
    if (error_code) {
      dbg::errorf(CURRENT_SOURCE_LOCATION,
                  "Error reading file: {}: {}",
                  path.string(),
                  error_code.message());
    } else {
      ready = true;
    }
  }

  /**
   * @brief Constructor reads an array of bytes.
   * @param binary_data The pointer to the data
   * @param length The length of the data
   * @param all_data Set to true if binary_data contained all data, set to false, if more data will come later.
   */
  BinaryDataReader(const uint8_t* binary_data, size_t length, bool all_data, std::endian endian) noexcept
      : BinaryDataBuffer(endian) {
    [[maybe_unused]] bool const result = addData(binary_data, length, all_data);
  }

  /**
   * @brief Constructor reads an array of bytes.
   * @param binary_data The pointer to the data
   * @param length The length of the data
   * @param all_data Set to true if binary_data contained all data, set to false, if more data will come later.
   * @param max_expected_size If you know the maximal size you expect to fill later, give it now to avoid the risc of a bad allocation error later in case of fragmented data or other operating system dependend resize problems. (data size will be reserved accordingly)
   */
  BinaryDataReader(const uint8_t* binary_data,
                   size_t length,
                   bool all_data,
                   size_t max_expected_size,
                   std::endian endian) noexcept
      : BinaryDataBuffer(endian) {
    m_buffer.reserve(max_expected_size);
    [[maybe_unused]] bool const result = addData(binary_data, length, all_data);
  }

  /**
   * @brief Constructor reads a vector of bytes.
   * @param binary_data The vector containing all data. You may want to use std::move to avoid a copy.
   */
  BinaryDataReader(std::vector<uint8_t> binary_data, std::endian endian) noexcept
      : BinaryDataBuffer(std::move(binary_data), endian) {}


  void setEndian(std::endian endian) { m_endian = endian; }

  /**
   * @brief In case not all data was available at construction you can add more data here.
   * @param binary_data The pointer to the data
   * @param length The length of the data
   * @param all_data Set to true if binary_data contained all data, set to false, if more data will come later.
   */
  [[nodiscard]] bool addData(const uint8_t* binary_data, size_t length, bool all_data) noexcept {
    // Null pointer is not acceptable; do not mark the reader as ready when
    // constructing from a null pointer. This makes behaviour explicit and
    // matches tests that expect a nullptr constructor to yield not-ready.
    if (binary_data == nullptr) {
      dbg::error(CURRENT_SOURCE_LOCATION, "Nullpointer given.");
      return false;
    }
    if (length == 0 && all_data) {
      ready = all_data;
      m_buffer.shrink_to_fit();
      return true;
    }
    if (ready) {
      dbg::error(
        CURRENT_SOURCE_LOCATION,
        "You can not add more data. Did you use the wrong constructor?");
      return false;
    }
    if (length != 0) {
      try {
        m_buffer.reserve(m_buffer.size() + length);
        const auto* const begin = binary_data;
        const auto* const end = std::next(begin, static_cast<std::ptrdiff_t>(length));
        m_buffer.insert(m_buffer.end(), begin, end);
      } catch (const std::bad_alloc&) {
        dbg::error(CURRENT_SOURCE_LOCATION,
                   "Memory allocation failed while adding data.");
        return false;
      }
    }
    ready = all_data;
    if (ready) {
      m_buffer.shrink_to_fit();
    }
    return true;
  }

  /**
   * @brief Checks if there is enough data left from the current m_cursor position.
   * @param requestedSize The number of bytes requested.
   * @return true if enough data is left, false otherwise.
   */
  [[nodiscard]] bool hasDataLeft(size_t requestedSize) const noexcept {
    return ready && (m_cursor + requestedSize <= m_buffer.size());
  }

  /**
   * @brief Calculates the number of bytes left.
   * @return The number of unread bytes.
   */
  [[nodiscard]] size_t getNumUnreadBytes() const noexcept {
    return m_buffer.size() - m_cursor;
  }

  /**
   * @brief Checks if the next bytes at the cursor match the given bytes.
   * @param bytes The bytes to compare.
   * @return true if the next bytes match, false otherwise.
   */
  [[nodiscard]] bool nextBytesEqual(const std::vector<uint8_t>& bytes) const noexcept {
    if (!hasDataLeft(bytes.size())) {
      return false;
    }
    // Cast m_cursor to difference_type to avoid sign-conversion warning
    auto offset = static_cast<std::vector<uint8_t>::difference_type>(m_cursor);
    return std::equal(bytes.begin(), bytes.end(), m_buffer.begin() + offset);
  }

  /**
   * @brief Advances the m_cursor by a given size.
   * @param size The number of bytes to advance.
   * @return true if the m_cursor was advanced, false if not enough data.
   */
  [[nodiscard]] bool advanceCursor(size_t size) const noexcept {
    if (!hasDataLeft(size)) {
      return false;
    }
    m_cursor += size;
    return true;
  }

  /**
   * @brief Advances the m_cursor if the next bytes match the given bytes.
   * @param bytes The bytes to compare.
   * @return true if the m_cursor was advanced, false otherwise.
   */
  [[nodiscard]] bool advanceCursorIfEqual(const std::vector<uint8_t>& bytes) const noexcept {
    if (nextBytesEqual(bytes)) {
      m_cursor += bytes.size();
      return true;
    }
    return false;
  }

  /**
   * @brief Finds the next occurrence of the given bytes and advances the m_cursor.
   * @param bytes The bytes to search for.
   * @param advanceBeyond If true, advances m_cursor beyond the found bytes.
   * @return true if the bytes were found and m_cursor advanced, false otherwise.
   */
  [[nodiscard]] bool findNextBytesAndAdvance(const std::vector<uint8_t>& bytes,
                                             bool advanceBeyond) const noexcept {
    if (!ready || bytes.empty() || m_cursor >= m_buffer.size()) {
      return false;
    }
    // Cast m_cursor to difference_type to avoid sign-conversion warning
    auto offset = static_cast<std::vector<uint8_t>::difference_type>(m_cursor);
    auto it     = std::search(
      m_buffer.begin() + offset, m_buffer.end(), bytes.begin(), bytes.end());
    if (it == m_buffer.end()) {
      return false;
    }
    m_cursor = static_cast<size_t>(std::distance(m_buffer.begin(), it));
    if (advanceBeyond) {
      m_cursor += bytes.size();
    }
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::optional<T>.
   * First reads a bool flag (true if value exists), then the value if present.
   *
   * @tparam T Contained type.
   * @param value Pointer to optional where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename T>
  [[nodiscard]] bool readNext(std::optional<T>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    bool hasValue{};
    if (!readNext(&hasValue)) {
      return false;
    }

    if (!hasValue) {
      *value = std::nullopt;
      return true;
    }

    T elem{};
    if (!readNext(&elem)) {
      return false;
    }
    *value = std::move(elem);
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::variant<Ts...>.
   * First reads an index (size_t) identifying the active type, then the value.
   *
   * @tparam Ts Types of possible alternatives.
   * @param value Pointer to variant where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename... Ts>
  [[nodiscard]] bool readNext(std::variant<Ts...>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    size_t index{};
    if (!readNext(&index)) {
      return false;
    }

    if (index >= sizeof...(Ts)) {
      return false;  // invalid variant index
    }

    return readVariantImpl<std::variant<Ts...>>(*this, index, *value);
  }

  /**
   * @brief Reads the next bytes as a std::pair<A,B>.
   * Calls readNext() for first and second elements.
   *
   * @tparam A Type of first element.
   * @tparam B Type of second element.
   * @param value Pointer to pair where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename A, typename B>
  [[nodiscard]] bool readNext(std::pair<A, B>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    A first{};
    B second{};
    if (!readNext(&first)) {
      return false;
    }
    if (!readNext(&second)) {
      return false;
    }

    *value = std::make_pair(std::move(first), std::move(second));
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::tuple<Ts...>.
   * Calls readNext() for each element recursively.
   *
   * @tparam Ts Types of tuple elements.
   * @param value Pointer to tuple where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename... Ts>
  [[nodiscard]] bool readNext(std::tuple<Ts...>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }
    return readTupleImpl(*this, *value, std::index_sequence_for<Ts...>{});
  }

  /**
   * @brief Reads the next bytes as a std::vector<T>. First reads the number of
   * elements (size_t), then each element with readNext().
   *
   * @tparam T Element type. Must be supported by readNext().
   * @param value Pointer to vector where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename T>
  [[nodiscard]] bool readNext(std::vector<T>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    size_t count{};
    if (!readNext(&count)) {
      return false;
    }
    value->clear();
    value->reserve(count);

    for (size_t i = 0; i < count; ++i) {
      T elem{};
      if (!readNext(&elem)) {
        return false;
      }
      value->push_back(std::move(elem));
    }
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::list<T>. First reads the number of
   * elements (size_t), then each element with readNext().
   *
   * @tparam T Element type. Must be supported by readNext().
   * @param value Pointer to list where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename T>
  [[nodiscard]] bool readNext(std::list<T>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    size_t count{};
    if (!readNext(&count)) {
      return false;
    }
    value->clear();

    for (size_t i = 0; i < count; ++i) {
      T elem{};
      if (!readNext(&elem)) {
        return false;
      }
      value->push_back(std::move(elem));
    }
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::deque<T>. First reads the number of
   * elements (size_t), then each element with readNext().
   *
   * @tparam T Element type. Must be supported by readNext().
   * @param value Pointer to list where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename T>
  [[nodiscard]] bool readNext(std::deque<T>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    size_t count{};
    if (!readNext(&count)) {
      return false;
    }
    value->clear();
    value->resize(count);

    for (size_t i = 0; i < count; ++i) {
      if (!readNext(&(*value)[i])) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::array<T,N>. Reads N elements with readNext().
   *
   * @tparam T Element type. Must be supported by readNext().
   * @param value Pointer to list where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename T, std::size_t N>
  [[nodiscard]] bool readNext(std::array<T, N>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    for (std::size_t i = 0; i < N; ++i) {
      if (!readNext(&(*value)[i])) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::map<K,V>. First reads the number of
   * elements (size_t), then key-value pairs with readNext().
   *
   * @tparam K Key type.
   * @tparam V Value type.
   * @param value Pointer to map where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename K, typename V>
  [[nodiscard]] bool readNext(std::map<K, V>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    size_t count{};
    if (!readNext(&count)) {
      return false;
    }
    value->clear();

    for (size_t i = 0; i < count; ++i) {
      K key{};
      V val{};
      if (!readNext(&key)) {
        return false;
      }
      if (!readNext(&val)) {
        return false;
      }
      value->emplace(std::move(key), std::move(val));
    }
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::unordered_map<K,V>.
   *
   * @tparam K Key type.
   * @tparam V Value type.
   * @param value Pointer to map where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename K, typename V>
  [[nodiscard]] bool readNext(std::unordered_map<K, V>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    size_t count{};
    if (!readNext(&count)) {
      return false;
    }
    value->clear();

    for (size_t i = 0; i < count; ++i) {
      K key{};
      V val{};
      if (!readNext(&key)) {
        return false;
      }
      if (!readNext(&val)) {
        return false;
      }
      value->emplace(std::move(key), std::move(val));
    }
    return true;
  }

  /**
   * @brief Reads the next bytes as a std::set<T>. First reads the number of
   * elements (size_t), then inserts them with readNext().
   *
   * @tparam T Element type.
   * @param value Pointer to set where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename T>
  [[nodiscard]] bool readNext(std::set<T>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    size_t count{};
    if (!readNext(&count)) {
      return false;
    }
    value->clear();

    for (size_t i = 0; i < count; ++i) {
      T elem{};
      if (!readNext(&elem)) {
        return false;
      }
      value->insert(std::move(elem));
    }
    return true;
  }


  /**
   * @brief Reads the next bytes as a std::unordered_set<T>.
   *
   * @tparam T Element type.
   * @param value Pointer to set where data will be stored.
   * @return true if successful, false otherwise.
   */
  template <typename T>
  [[nodiscard]] bool readNext(std::unordered_set<T>* value) const noexcept {
    if (!value || !ready) {
      return false;
    }

    size_t count{};
    if (!readNext(&count)) {
      return false;
    }
    value->clear();

    for (size_t i = 0; i < count; ++i) {
      T elem{};
      if (!readNext(&elem)) {
        return false;
      }
      value->insert(std::move(elem));
    }
    return true;
  }

  /**
   * @brief Read a std::bitset<N> stored as the smallest integer that fits (up to 64 bits).
   * @tparam N number of bits (must be <= 64)
   */
  template <std::size_t N>
    requires(N > 0 && N <= 64)
  bool readNext(std::bitset<N>* value) const noexcept {
    // TODO optimize: use M times uint8_t instead of hardcoded if else
    // M is ceil(N/8)
    if constexpr (N <= 8) {
      uint8_t tmp{};
      if (!readNext(&tmp)) {
        return false;
      }
      *value = std::bitset<N>(tmp);
      return true;
    } else if constexpr (N <= 16) {
      uint16_t tmp{};
      if (!readNext(&tmp)) {
        return false;
      }
      *value = std::bitset<N>(tmp);
      return true;
    } else if constexpr (N <= 32) {
      uint32_t tmp{};
      if (!readNext(&tmp)) {
        return false;
      }
      *value = std::bitset<N>(tmp);
      return true;
    } else {
      uint64_t tmp{};
      if (!readNext(&tmp)) {
        return false;
      }
      *value = std::bitset<N>(tmp);
      return true;
    }
  }

  /**
   * @brief Returns an constant iterator to the curent m_cursor position of the data vector.
   * @return constant iterator to the data vector advanced to the current m_cursor position.
   */
  [[nodiscard]] std::vector<uint8_t>::const_iterator getReadBegin() const noexcept {
    return std::next(m_buffer.cbegin(), static_cast<std::ptrdiff_t>(m_cursor));
  }

  /**
   * @brief Returns an pair: first is a constant iterator to the curent cursor position of the data vector, second is a constant iterator advanced about given length from first.
   * @param length Number of bytes to read.
   * @return pair of start and end iterator, positions in the data array at current cursor position and current cursor position + length.
   */
  [[nodiscard]] std::pair<std::vector<uint8_t>::const_iterator, std::vector<uint8_t>::const_iterator>
  getReadBeginAndEnd(size_t length) const noexcept {
    const auto readBegin = getReadBegin();
    const auto readEnd = std::next(readBegin, static_cast<std::ptrdiff_t>(length));
    return {readBegin, readEnd};
  }

  /**
   * @brief Reads the next bytes as a string of given length.
   * @param value Pointer to the string to store the result.
   * @return true if successful, false otherwise.
   */
  [[nodiscard]] bool readNext(std::string* value) const noexcept {

    size_t length = 0;
    if (!readNext(&length)) {
      return false;
    }

    if ((value == nullptr) || !ready || !hasDataLeft(length)) {
      return false;
    }
    const auto readIterators = getReadBeginAndEnd(length);
    value->assign(readIterators.first, readIterators.second);
    m_cursor += length;
    return true;
  }

  /**
   * @brief Reads the next bytes as a wstring of given length.
   * @param value Pointer to the wstring to store the result.
   * @return true if successful, false otherwise.
   */
  [[nodiscard]] bool readNext(std::wstring* value) const noexcept {
    std::string tmp;
    if (readNext(&tmp)) {
      return util::utf8ToWstring(tmp, value);
    }
    return false;
  }

  /**
   * @brief Reads info about std::size_t always from a 64 bit type.
   * @return true if the size could be read and did not overflow.
   */
  template <typename T>
  bool readNext(T* size) noexcept
    requires(std::is_same_v<T, std::size_t> && sizeof(std::size_t) != sizeof(SizeType))
  {
    SizeType temp{};
    if (!readNext(&temp)) {
      return false;
    }
    if (temp > static_cast<SizeType>(std::numeric_limits<T>::max())) {
      dbg::errorf(CURRENT_SOURCE_LOCATION,
                  "Your data origenates from a 64 bit system! You tried to "
                  "read a size (64 bit) with value {} but your size type in "
                  "only 32 bit and overflows!",
                  temp);
      return false;
    }
    *size = static_cast<std::size_t>(temp);
    return true;
  }

  /**
   * @brief Read a trivially scalar value from the buffer.
   * @tparam enum type.
   * @param value Pointer to the value to store the result.
   * @return true if successful, false otherwise.
   */
  template <typename T>
    requires(std::is_enum_v<T>)
  bool readNext(T* value) const {
    return readNext(static_cast<std::underlying_type_t<T*>>(value));
  }

  /**
   * @brief Read a trivially scalar value from the buffer.
   * @tparam T Integral, floating-point.
   * @param value Pointer to the value to store the result.
   * @return true if successful, false otherwise.
   */
  template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
  bool readNext(T* value) const {

    const uint8_t* src = m_buffer.data() + m_cursor;

    if (!hasDataLeft(sizeof(T))) {
      return false;
    }

    m_cursor += sizeof(T);

    using IntType =
      std::conditional_t<std::is_floating_point_v<T>,
                         std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>,
                         typename detail::MakeUnsignedIfIntegral<T>::type>;

    IntType bits;

    constexpr size_t SIZE_BYTE{8};

    if constexpr (sizeof(T) == 1) {
      bits = *src;
    } else if (getEndian() == std::endian::native) {
      std::memcpy(&bits, src, sizeof(T));
    } else {
      bits = 0;
      if (getEndian() == std::endian::little) {
        for (size_t i = 0; i < sizeof(T); ++i) {
          bits |= static_cast<IntType>(src[i]) << (i * SIZE_BYTE);
        }
      } else {
        for (size_t i = 0; i < sizeof(T); ++i) {
          bits |= static_cast<IntType>(src[i]) << ((sizeof(T) - 1 - i) * SIZE_BYTE);
        }
      }
    }

    if constexpr (std::is_floating_point_v<T>) {
      *value = std::bit_cast<T>(bits);
    } else {
      *value = static_cast<T>(bits);
    }
    return true;
  }

  // Special-case bool to avoid instantiating make_unsigned<bool>.
  bool readNext(bool* value) const noexcept {
    if (!hasDataLeft(1)) {
      return false;
    }
    const uint8_t* src  = m_buffer.data() + m_cursor;
    m_cursor           += 1;
    *value              = (*src != 0);
    return true;
  }

  /**
   * @brief Read a serializable object from the buffer.
   * @tparam SerializableObject A object that implements bool deserialize(const BinaryDataReader& reader);
   * @param value Pointer to the value to store the result.
   * @return true if successful, false otherwise.
   */
  template <typename SerializableObject>
  bool readNext(SerializableObject* value) const noexcept {
    return value->deserialize(*this);
  }

  /**
   * @brief Returns a constant reference to the binaries.
   * @return The binaries.
   */
  [[nodiscard]] const std::vector<uint8_t>& getData() const noexcept {
    return m_buffer;
  }

 private:
  template <std::size_t I, typename... Ts>
  bool readTupleElement(const BinaryDataReader& reader, std::tuple<Ts...>& tup) const noexcept {
    using Elem = std::tuple_element_t<I, std::tuple<Ts...>>;
    Elem elem{};
    if (!reader.readNext(&elem)) {
      return false;
    }
    std::get<I>(tup) = std::move(elem);
    return true;
  }

  template <std::size_t... Is, typename... Ts>
  bool readTupleImpl(const BinaryDataReader& reader,
                     std::tuple<Ts...>& tup,
                     std::index_sequence<Is...> /*unused*/) const noexcept {
    return (... && readTupleElement<Is>(reader, tup));
  }

  template <typename Variant, std::size_t I = 0>
  bool readVariantImpl(const BinaryDataReader& reader, size_t index, Variant& var) const noexcept {
    if constexpr (I >= std::variant_size_v<Variant>) {
      return false;  // invalid index
    } else {
      if (I == index) {
        using Alt = std::variant_alternative_t<I, Variant>;
        Alt elem{};
        if (!reader.readNext(&elem)) {
          return false;
        }
        var = std::move(elem);
        return true;
      }
      return readVariantImpl<Variant, I + 1>(reader, index, var);
    }
  }
};


}  // namespace serialize
