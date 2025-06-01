#include <algorithm>  // Added for std::search and std::equal
#include <utils/data/BinaryDataInterpreter.hpp>


namespace util {

BinaryDataInterpreter::BinaryDataInterpreter(const std::filesystem::path& path) {
  try {
    data  = BinaryDataInterpreter::readFileBinary<uint8_t>(path);
    ready = true;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
}

BinaryDataInterpreter::BinaryDataInterpreter(const uint8_t* binary_data, size_t length) {
  if (binary_data == nullptr) {
    return;
  }
  data.reserve(length);
  if (length != 0) {
    this->data.assign(binary_data, binary_data + length);
  }
  ready = true;
}

void BinaryDataInterpreter::setCursorToEnd() { cursor = data.size(); }

void BinaryDataInterpreter::setCursorToStart() { cursor = 0; }

bool BinaryDataInterpreter::hasDataLeft(size_t requestedSize) const {
  return ready && (cursor + requestedSize <= data.size());
}

bool BinaryDataInterpreter::nextBytesEqual(const std::vector<uint8_t>& bytes) const {
  if (!hasDataLeft(bytes.size())) {
    return false;
  }
  // Cast cursor to difference_type to avoid sign-conversion warning
  auto offset = static_cast<std::vector<uint8_t>::difference_type>(cursor);
  return std::equal(bytes.begin(), bytes.end(), data.begin() + offset);
}

bool BinaryDataInterpreter::advanceCursor(size_t size) {
  if (!hasDataLeft(size)) {
    return false;
  }
  cursor += size;
  return true;
}

bool BinaryDataInterpreter::advanceCursorIfEqual(const std::vector<uint8_t>& bytes) {
  if (nextBytesEqual(bytes)) {
    cursor += bytes.size();
    return true;
  }
  return false;
}

bool BinaryDataInterpreter::findNextBytesAndAdvance(const std::vector<uint8_t>& bytes,
                                                    bool advanceBeyond) {
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

size_t BinaryDataInterpreter::getCursor() const { return cursor; }

size_t BinaryDataInterpreter::size() const { return data.size(); }

bool BinaryDataInterpreter::setCursor(size_t newCursor) {
  if (newCursor > data.size()) {
    return false;
  }
  cursor = newCursor;
  return true;
}

bool BinaryDataInterpreter::readNext(std::string* value, size_t length) {
  if (!value || !ready || !hasDataLeft(length) || value == nullptr) {
    return false;
  }
  value->assign(reinterpret_cast<const char*>(data.data() + cursor), length);
  cursor += length;
  return true;
}

bool BinaryDataInterpreter::readNext(std::wstring* value, size_t length) {
  if (!value || !ready || !hasDataLeft(length) || value == nullptr) {
    return false;
  }
  if (length % 2 == 1) {
    return false;
  }
  value->assign(reinterpret_cast<const wchar_t*>(data.data() + cursor), length / 2);
  cursor += length;
  return true;
}

}  // namespace util
