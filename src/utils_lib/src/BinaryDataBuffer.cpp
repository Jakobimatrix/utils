/**
 * @file BinaryDataBuffer.cpp
 * @brief Implementation of the parent class for both reader and writer from and to binary.
 *
 * @version 1.0
 * @date 03.10.2025
 */

#include <utils/data/BinaryDataBuffer.hpp>
#include <utils/debug/logging.hpp>

namespace serialize {

BinaryDataBuffer::BinaryDataBuffer(std::endian endian) noexcept
    : endian(endian) {}

BinaryDataBuffer::BinaryDataBuffer(std::vector<uint8_t>&& buf, std::endian endian) noexcept
    : endian(endian),
      ready(true),
      buffer(std::move(buf)) {}

std::vector<uint8_t>&& BinaryDataBuffer::releaseBuffer() noexcept {
  cursor = 0;
  ready  = false;
  return std::move(buffer);
}

void BinaryDataBuffer::setCursorToEnd() noexcept { cursor = buffer.size(); }

void BinaryDataBuffer::setCursorToStart() noexcept { cursor = 0; }

size_t BinaryDataBuffer::size() const noexcept { return buffer.size(); }

bool BinaryDataBuffer::setCursor(size_t newCursor) noexcept {
  if (newCursor > buffer.size()) {
    return false;
  }
  cursor = newCursor;
  return true;
}

}  // namespace serialize
