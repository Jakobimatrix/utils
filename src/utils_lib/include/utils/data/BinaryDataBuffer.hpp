/**
 * @file BinaryDataBuffer.hpp
 * @brief A parent class for both reader and writer from and to binary.
 *
 * @version 1.0
 * @date 03.10.2025
 */

#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace serialize {


class BinaryDataBuffer {
  std::endian endian;

 protected:
  // NOLINTBEGIN (misc-non-private-member-variables-in-classes) managed by children
  mutable size_t cursor = 0;
  bool ready{false};
  std::vector<uint8_t> buffer;
  // NOLINTEND (misc-non-private-member-variables-in-classes)

  explicit BinaryDataBuffer(std::endian endian) noexcept;
  explicit BinaryDataBuffer(std::vector<uint8_t>&& buffer, std::endian endian) noexcept;

 public:
  using SizeType = uint64_t;

  [[nodiscard]] const std::vector<uint8_t>& getBuffer() const { return buffer; }
  [[nodiscard]] std::vector<uint8_t>&& releaseBuffer() noexcept;

  /**
   * @brief Returns the status of the data.
   * @return true if the data was read correctly, false otherwise.
   */
  [[nodiscard]] bool isReady() const noexcept { return ready; }

  /**
   * @brief Returns the endian the data.
   * @return ho to interpret the data..
   */
  [[nodiscard]] std::endian getEndian() const noexcept { return endian; }

  /**
   * @brief Sets the cursor to the end of the data.
   */
  void setCursorToEnd() noexcept;

  /**
   * @brief Sets the cursor to the start of the data.
   */
  void setCursorToStart() noexcept;

  /**
   * @brief Gets the current cursor position.
   * @return The current cursor position.
   */
  [[nodiscard]] size_t getCursor() const noexcept { return cursor; }

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
};

}  // namespace serialize
