/**
 * @file BinaryDataBuffer.cpp
 * @brief Implementation of the parent class for both reader and writer from and to binary.
 *
 * @version 1.0
 * @date 03.10.2025
 */

#include <bit>
#include <cstdint>
#include <utility>
#include <cstddef>
#include <utils/data/BinaryDataBuffer.hpp>
#include <vector>

namespace serialize {

BinaryDataBuffer::BinaryDataBuffer(std::endian endian) noexcept
    : m_endian(endian) {}

BinaryDataBuffer::BinaryDataBuffer(std::vector<uint8_t>&& buf, std::endian endian) noexcept
    : ready(true),
      m_buffer(std::move(buf)),
      m_endian(endian) {}

std::vector<uint8_t>&& BinaryDataBuffer::releaseBuffer() noexcept {
  m_cursor = 0;
  ready    = false;
  return std::move(m_buffer);
}

void BinaryDataBuffer::setCursorToEnd() noexcept { m_cursor = m_buffer.size(); }

void BinaryDataBuffer::setCursorToStart() noexcept { m_cursor = 0; }

size_t BinaryDataBuffer::size() const noexcept { return m_buffer.size(); }

bool BinaryDataBuffer::setCursor(size_t newCursor) noexcept {
  if (newCursor > m_buffer.size()) {
    return false;
  }
  m_cursor = newCursor;
  return true;
}

}  // namespace serialize
