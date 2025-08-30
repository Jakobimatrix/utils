/**
 * @file BinaryDataInterpreter.cpp
 * @brief implementation for the BinaryDataInterpreter class.
 * @date 29.05.2025
 * @author Jakob Wandel
 * @version 1.0
 **/

#include <utils/data/BinaryDataInterpreter.hpp>

#include <algorithm>
#include <filesystem>
#include <cstdint>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <utility>
#include <string>
#include <system_error>
#include <vector>



namespace util {

BinaryDataInterpreter::BinaryDataInterpreter(const std::filesystem::path& path) noexcept {
  std::error_code error_code;
  data = BinaryDataInterpreter::readFileBinary<uint8_t>(path, error_code);
  if (error_code) {
    std::cerr << "Error reading file: " << path << ": " << error_code.message() << "\n";
  } else {
    ready = true;
  }
}

BinaryDataInterpreter::BinaryDataInterpreter(const uint8_t* binary_data, size_t length) noexcept {
  if (binary_data == nullptr) {
    return;
  }
  data.reserve(length);
  if (length != 0) {
    const auto* const begin = binary_data;
    const auto* const end = std::next(begin, static_cast<std::ptrdiff_t>(length));
    data.assign(begin, end);
  }
  ready = true;
}

void BinaryDataInterpreter::setCursorToEnd() noexcept { cursor = data.size(); }

void BinaryDataInterpreter::setCursorToStart() noexcept { cursor = 0; }

bool BinaryDataInterpreter::hasDataLeft(size_t requestedSize) const noexcept {
  return ready && (cursor + requestedSize <= data.size());
}

bool BinaryDataInterpreter::nextBytesEqual(const std::vector<uint8_t>& bytes) const noexcept {
  if (!hasDataLeft(bytes.size())) {
    return false;
  }
  // Cast cursor to difference_type to avoid sign-conversion warning
  auto offset = static_cast<std::vector<uint8_t>::difference_type>(cursor);
  return std::equal(bytes.begin(), bytes.end(), data.begin() + offset);
}

bool BinaryDataInterpreter::advanceCursor(size_t size) noexcept {
  if (!hasDataLeft(size)) {
    return false;
  }
  cursor += size;
  return true;
}

bool BinaryDataInterpreter::advanceCursorIfEqual(const std::vector<uint8_t>& bytes) noexcept {
  if (nextBytesEqual(bytes)) {
    cursor += bytes.size();
    return true;
  }
  return false;
}

bool BinaryDataInterpreter::findNextBytesAndAdvance(const std::vector<uint8_t>& bytes,
                                                    bool advanceBeyond) noexcept {
  if (!ready || bytes.empty() || cursor >= data.size()) {
    return false;
  }
  // Cast cursor to difference_type to avoid sign-conversion warning
  auto offset = static_cast<std::vector<uint8_t>::difference_type>(cursor);
  auto it = std::search(data.begin() + offset, data.end(), bytes.begin(), bytes.end());
  if (it == data.end()) {
    return false;
  }
  cursor = static_cast<size_t>(std::distance(data.begin(), it));
  if (advanceBeyond) {
    cursor += bytes.size();
  }
  return true;
}

size_t BinaryDataInterpreter::getCursor() const noexcept { return cursor; }

size_t BinaryDataInterpreter::size() const noexcept { return data.size(); }

bool BinaryDataInterpreter::setCursor(size_t newCursor) noexcept {
  if (newCursor > data.size()) {
    return false;
  }
  cursor = newCursor;
  return true;
}

bool BinaryDataInterpreter::readNext(std::string* value, size_t length) noexcept {
  if ((value == nullptr) || !ready || !hasDataLeft(length) || value == nullptr) {
    return false;
  }
  const auto readIterators = getReadBeginAndEnd(length);
  value->assign(readIterators.first, readIterators.second);
  cursor += length;
  return true;
}

bool BinaryDataInterpreter::readNext(std::wstring* value, size_t length) noexcept {
  if ((value == nullptr) || !ready || !hasDataLeft(length) || value == nullptr) {
    return false;
  }
  if (length % 2 == 1) {
    return false;
  }
  const auto readIterators = getReadBeginAndEnd(length);
  value->assign(readIterators.first, readIterators.second);
  cursor += length;
  return true;
}


std::vector<uint8_t>::const_iterator BinaryDataInterpreter::getReadBegin() const noexcept {
  return std::next(data.cbegin(), static_cast<std::ptrdiff_t>(cursor));
}

std::pair<std::vector<uint8_t>::const_iterator, std::vector<uint8_t>::const_iterator>
BinaryDataInterpreter::getReadBeginAndEnd(size_t length) const noexcept {
  const auto readBegin = getReadBegin();
  const auto readEnd = std::next(readBegin, static_cast<std::ptrdiff_t>(length));
  return {readBegin, readEnd};
}

}  // namespace util
