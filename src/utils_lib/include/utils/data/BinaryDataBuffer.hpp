/**
 * @file BinaryDataBuffer.hpp
 * @brief A parent class for both reader and writer from and to binary.
 *
 * @date 03.10.2025
 */

#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <vector>

namespace serialize {


class BinaryDataBuffer {

 protected:
  // NOLINTBEGIN (misc-non-private-member-variables-in-classes) managed by children
  mutable size_t m_cursor = 0;
  bool ready{false};
  std::vector<uint8_t> m_buffer;
  std::endian m_endian;
  bool m_enable_crossplatform_checks{true};
  // NOLINTEND (misc-non-private-member-variables-in-classes)

  explicit BinaryDataBuffer(std::endian endian) noexcept;
  explicit BinaryDataBuffer(std::vector<uint8_t>&& buffer, std::endian endian) noexcept;

 public:
  using SizeType = uint64_t;

  // Fixed-width / platform-independent aliases
  // The purpose of these aliases is to provide canonical types that are
  // identical across compilers and platforms and are large enough to hold
  // the corresponding native type's information on any supported platform.
  // Use these when serializing/deserializing to ensure consistent on-disk
  // format regardless of target architecture or ABI.

  // A signed counterpart that can hold any value of ptrdiff_t / signed sizes
  // on common platforms (LP64, LLP64, ILP32). 64 bits is chosen to be safe.
  using SignedSizeType = int64_t;

  // Fixed storage for character data when raw bytes are intended. Use this
  // alias for serializing `char` / `signed char` / `unsigned char` values.
  using ByteType       = uint8_t;
  using SignedByteType = int8_t;

  // wchar_t differs between platforms: 16 bits on Windows, 32 bits on many
  // Unix systems. Use a 32-bit unsigned type to be able to hold any wchar_t
  // value and to carry Unicode code points when converting to/from UTF-8.
  using WideCharStorageType = uint32_t;

  // Native pointer-sized unsigned and signed integer types. These aliases
  // describe values that represent pointer-sized integers but promote them
  // to 64-bit to be portable across 32/64-bit platforms when serialized.
  using PointerUnsignedType = uint64_t;
  using PointerSignedType   = int64_t;

  // Canonical integer types to represent platform 'long' and related types
  // in a fixed-width way when serializing. Using 64-bit signed/unsigned
  // types ensures we can represent 'long' (32 or 64 bit) from any ABI.
  using CanonicalLong         = int64_t;
  using CanonicalUnsignedLong = uint64_t;

  /**
   * @brief Returns the rad only buffer.
   * @return The buffer
   */
  [[nodiscard]] const std::vector<uint8_t>& getBuffer() const {
    return m_buffer;
  }

  /**
   * @brief Returns a span of the rad only buffer.
   * @param start The start bit.
   * @param length The size to read.
   * @return The buffer span. If invalid start/length given, an empty span is returned.
   */
  [[nodiscard]] std::span<const uint8_t> getBuffer(size_t start, size_t length) const {
    const size_t end = start + length;
    if (end >= m_buffer.size() || end < start) {
      return {};
    }
    return {&m_buffer[start], &m_buffer[start + length]};
  }

  [[nodiscard]] std::vector<uint8_t>&& releaseBuffer() noexcept;

  /**
   * @brief Returns the status of the data.
   * @return true if the data was read correctly, false otherwise.
   */
  [[nodiscard]] bool isReady() const noexcept { return ready; }

  /**
   * @brief Returns the m_endian the data.
   * @return ho to interpret the data..
   */
  [[nodiscard]] std::endian getEndian() const noexcept { return m_endian; }

  /**
   * @brief Sets the cursor to the end of the data.
   */
  void setCursorToEnd() noexcept;

  /**
   * @brief Sets the cursor to the start of the data.
   */
  void setCursorToStart() noexcept;

  /**
   * @brief Gets the current m_cursor position.
   * @return The current m_cursor position.
   */
  [[nodiscard]] size_t getCursor() const noexcept { return m_cursor; }

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

namespace detail {
// Helper trait to avoid instantiating std::make_unsigned_t for non-integral types
template <typename U, bool = std::is_integral_v<U>>
struct MakeUnsignedIfIntegral;
template <typename U>
struct MakeUnsignedIfIntegral<U, true> {
  using type = std::make_unsigned_t<U>;
};
template <typename U>
struct MakeUnsignedIfIntegral<U, false> {
  using type = U;
};
}  // namespace detail

}  // namespace serialize
