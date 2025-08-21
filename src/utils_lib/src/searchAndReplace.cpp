#include <string>
#include <cstddef>
#include <utils/string/searchAndReplace.hpp>

namespace util {

void replaceSubstring(std::string* str, const std::string& toSearch, const std::string& toReplace) {
  replaceSubstring(str, toSearch, toReplace, 0);
}

void replaceSubstring(std::string* str,
                      const std::string& toSearch,
                      const std::string& toReplace,
                      size_t startPos) {
  if (str == nullptr || toSearch.empty()) {
    return;
  }

  size_t pos = startPos;
  while ((pos = str->find(toSearch, pos)) != std::string::npos) {
    str->replace(pos, toSearch.length(), toReplace);
    pos += toReplace.length();  // move past the replaced part
  }
}
}  // namespace util
