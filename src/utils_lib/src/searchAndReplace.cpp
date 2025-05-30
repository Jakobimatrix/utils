#include <utils/string/searchAndReplace.hpp>

namespace util {
/**
 * @brief Replaces all occurrences of a substring with another substring in the given string, starting from a specified position.
 *
 * @param str Pointer to the string to modify.
 * @param toSearch The substring to search for.
 * @param toReplace The substring to replace with.
 * @param startPos The position to start searching from.
 */
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

/**
 * @brief Replaces all occurrences of a substring with another substring in the given string.
 *
 * @param str Pointer to the string to modify.
 * @param toSearch The substring to search for.
 * @param toReplace The substring to replace with.
 */
void replaceSubstring(std::string* str, const std::string& toSearch, const std::string& toReplace) {
  replaceSubstring(str, toSearch, toReplace, 0);
}
}  // namespace util
