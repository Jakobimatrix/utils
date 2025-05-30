#include <utils/string/searchAndReplace.hpp>

namespace util {

void replaceSubstring(std::string* str,
                      const std::string& toSearch,
                      const std::string& toReplace,
                      size_t startPos) {
  if (toSearch.empty()) {
    return;
  }
  size_t pos = str->find(toSearch, startPos);

  if (pos != std::string::npos) {
    str->replace(pos, toSearch.length(), toReplace);
    replaceSubstring(str, toSearch, toReplace, pos + toReplace.length());
  }
}


void replaceSubstring(std::string* str, const std::string& toSearch, const std::string& toReplace) {
  replaceSubstring(str, toSearch, toReplace, 0);
}
}  // namespace util
