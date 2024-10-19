#pragma once

#include <string>

void replaceSubstring(std::string* str, const std::string& toSearch, const std::string& toReplace, size_t startPos = 0) {
  size_t pos = str->find(toSearch, startPos);

  if (pos != std::string::npos) {
    str->replace(pos, toSearch.length(), toReplace);
    replaceSubstring(str, toSearch, toReplace, pos + toReplace.length());
  }
}
