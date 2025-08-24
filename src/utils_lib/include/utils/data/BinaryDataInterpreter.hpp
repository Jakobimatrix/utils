/**
 * @file BinaryDataInterpreter.hpp
 * @brief A class to interpret binary data in a save manner.
 *
 * @version 1.0
 * @date 2025.05.29
 */

#include <concepts>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <system_error>
#include <string>
#include <type_traits>
#include <vector>

namespace util {
class BinaryDataInterpreter {
 public:
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

    std::streamsize size = file.tellg();

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
  BinaryDataInterpreter(const std::filesystem::path& path) noexcept;

  /**
   * @brief Constructor reads a array of bytes.
   * @param data The pointer to the data
   * @param length The length of the data
   */
  BinaryDataInterpreter(const uint8_t* data, size_t length) noexcept;

  /**
   * @brief Returns the status of the data.
   * @return true if the data was read correctly, false otherwise.
   */
  [[nodiscard]] bool isReady() const noexcept { return ready; }

  /**
   * @brief Sets the cursor to the end of the data.
   */
  void setCursorToEnd() noexcept;

  /**
   * @brief Sets the cursor to the start of the data.
   */
  void setCursorToStart() noexcept;

  /**
   * @brief Checks if there is enough data left from the current cursor position.
   * @param requestedSize The number of bytes requested.
   * @return true if enough data is left, false otherwise.
   */
  [[nodiscard]] bool hasDataLeft(size_t requestedSize) const noexcept;

  /**
   * @brief Checks if the next bytes at the cursor match the given bytes.
   * @param bytes The bytes to compare.
   * @return true if the next bytes match, false otherwise.
   */
  [[nodiscard]] bool nextBytesEqual(const std::vector<uint8_t>& bytes) const noexcept;

  /**
   * @brief Advances the cursor by a given size.
   * @param size The number of bytes to advance.
   * @return true if the cursor was advanced, false if not enough data.
   */
  [[nodiscard]] bool advanceCursor(size_t size) noexcept;

  /**
   * @brief Advances the cursor if the next bytes match the given bytes.
   * @param bytes The bytes to compare.
   * @return true if the cursor was advanced, false otherwise.
   */
  [[nodiscard]] bool advanceCursorIfEqual(const std::vector<uint8_t>& bytes) noexcept;

  /**
   * @brief Finds the next occurrence of the given bytes and advances the cursor.
   * @param bytes The bytes to search for.
   * @param advanceBeyond If true, advances cursor beyond the found bytes.
   * @return true if the bytes were found and cursor advanced, false otherwise.
   */
  [[nodiscard]] bool findNextBytesAndAdvance(const std::vector<uint8_t>& bytes,
                                             bool advanceBeyond) noexcept;

  /**
   * @brief Gets the current cursor position.
   * @return The current cursor position.
   */
  [[nodiscard]] size_t getCursor() const noexcept;

  /**
   * @brief Gets the number of available bytes.
   * @return The size of the binary data.
   */
  [[nodiscard]] size_t size() const noexcept;

  /**
   * @brief Sets the cursor to a new position.
   * @param newCursor The new cursor position.
   * @return true if the cursor was set, false if out of bounds.
   */
  [[nodiscard]] bool setCursor(size_t newCursor) noexcept;

  template <typename T>
  [[nodiscard]] bool readNext(T* value)
    requires(std::is_trivially_copyable_v<T>)
  {
    if (!value || !ready || !hasDataLeft(sizeof(T))) {
      return false;
    }

    std::memcpy(value, std::to_address(getReadBegin()), sizeof(T));
    cursor += sizeof(T);
    return true;
  }

  /**
   * @brief Reads the next bytes as a string of given length.
   * @param value Pointer to the string to store the result.
   * @param length Number of bytes to read.
   * @return true if successful, false otherwise.
   */
  [[nodiscard]] bool readNext(std::string* value, size_t length) noexcept;

  /**
   * @brief Reads the next bytes as a string of given length.
   * @param value Pointer to the wstring to store the result.
   * @param length Number of bytes to read.
   * @return true if successful, false otherwise.
   */
  bool readNext(std::wstring* value, size_t length) noexcept;

  /**
   * @brief Returns an constant iterator to the curent cursor position of the data vector.
   * @return constant iterator to the data vector advanced to the current cursor position.
   */
  [[nodiscard]] std::vector<uint8_t>::const_iterator getReadBegin() const noexcept;

  /**
   * @brief Returns an pair: first is a constant iterator to the curent cursor position of the data vector, second is a constant iterator advanced about given length from first.
   * @param length Number of bytes to read.
   * @return pair of start and end iterator, positions in the data array at current cursor position and current cursor position + length.
   */
  [[nodiscard]] std::pair<std::vector<uint8_t>::const_iterator, std::vector<uint8_t>::const_iterator>
  getReadBeginAndEnd(size_t length) const noexcept;

 protected:
  std::vector<uint8_t> data;
  bool ready{false};

  size_t cursor{0};
};
}  // namespace util
