/**
 * @file BinaryDataInterpreter.hpp
 * @brief A class to interpret binary data in a save manner.
 *
 * @version 1.0
 * @date 2025.05.29
 */

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace util {
class BinaryDataInterpreter {
 public:
  /**
   * @brief Reads a binary file into a vector of bytes.
   *
   * @tparam ByteType Must be either char, unsigned char, or std::uint8_t.
   * @param path Path to the file.
   * @return std::vector<uint8_t> Contents of the file.
   * @throws std::runtime_error if the file can't be opened.
   */
  template <typename ByteType>
    requires(!std::is_const_v<ByteType> &&
             (std::same_as<ByteType, char> || std::same_as<ByteType, unsigned char> ||
              std::same_as<ByteType, signed char> || std::same_as<ByteType, std::uint8_t>))
  static std::vector<ByteType> readFileBinary(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
      throw std::runtime_error("Failed to open file: " + path.string());
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<ByteType> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
      throw std::runtime_error("Failed to read file: " + path.string());
    }

    return buffer;
  }

  /**
   * @brief Constructor reads a binary file. Check with ready() if the file was read correctly.
   * @param path The path to the file to read
   */
  BinaryDataInterpreter(const std::filesystem::path& path);

  /**
   * @brief Constructor reads a array of bytes.
   * @param data The pointer to the data
   * @param length The length of the data
   */
  BinaryDataInterpreter(const uint8_t* data, size_t length);

  /**
   * @brief Returns the status of the data.
   * @return true if the data was read correctly, false otherwise.
   */
  bool isReady() const { return ready; }

  /**
   * @brief Sets the cursor to the end of the data.
   */
  void setCursorToEnd();

  /**
   * @brief Sets the cursor to the start of the data.
   */
  void setCursorToStart();

  /**
   * @brief Checks if there is enough data left from the current cursor position.
   * @param requestedSize The number of bytes requested.
   * @return true if enough data is left, false otherwise.
   */
  bool hasDataLeft(size_t requestedSize) const;

  /**
   * @brief Checks if the next bytes at the cursor match the given bytes.
   * @param bytes The bytes to compare.
   * @return true if the next bytes match, false otherwise.
   */
  bool nextBytesEqual(const std::vector<uint8_t>& bytes) const;

  /**
   * @brief Advances the cursor by a given size.
   * @param size The number of bytes to advance.
   * @return true if the cursor was advanced, false if not enough data.
   */
  bool advanceCursor(size_t size);

  /**
   * @brief Advances the cursor if the next bytes match the given bytes.
   * @param bytes The bytes to compare.
   * @return true if the cursor was advanced, false otherwise.
   */
  bool advanceCursorIfEqual(const std::vector<uint8_t>& bytes);

  /**
   * @brief Finds the next occurrence of the given bytes and advances the cursor.
   * @param bytes The bytes to search for.
   * @param advanceBeyond If true, advances cursor beyond the found bytes.
   * @return true if the bytes were found and cursor advanced, false otherwise.
   */
  bool findNextBytesAndAdvance(const std::vector<uint8_t>& bytes, bool advanceBeyond);

  /**
   * @brief Gets the current cursor position.
   * @return The current cursor position.
   */
  size_t getCursor() const;

  /**
   * @brief Gets the number of available bytes.
   * @return The size of the binary data.
   */
  size_t size() const;

  /**
   * @brief Sets the cursor to a new position.
   * @param newCursor The new cursor position.
   * @return true if the cursor was set, false if out of bounds.
   */
  bool setCursor(size_t newCursor);

  template <typename T>
  bool readNext(T* value)
    requires(std::is_trivially_copyable_v<T>)
  {
    if (!value || !ready || !hasDataLeft(sizeof(T))) {
      return false;
    }
    std::memcpy(value, data.data() + cursor, sizeof(T));
    cursor += sizeof(T);
    return true;
  }

  /**
   * @brief Reads the next bytes as a string of given length.
   * @param value Pointer to the string to store the result.
   * @param length Number of bytes to read.
   * @return true if successful, false otherwise.
   */
  bool readNext(std::string* value, size_t length);

  /**
   * @brief Reads the next bytes as a string of given length.
   * @param value Pointer to the wstring to store the result.
   * @param length Number of bytes to read.
   * @return true if successful, false otherwise.
   */
  bool readNext(std::wstring* value, size_t length);

 protected:
  std::vector<uint8_t> data{};
  bool ready{false};

  size_t cursor{0};
};
}  // namespace util
