/**
 * @file searchAndReplace.hpp
 * @brief Contains templates to perform search and replace action on std::string
 *
 * @date 2023
 */

#pragma once

#include <cstddef>
#include <string>

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
                      size_t startPos);

/**
 * @brief Replaces all occurrences of a substring with another substring in the given string.
 *
 * @param str Pointer to the string to modify.
 * @param toSearch The substring to search for.
 * @param toReplace The substring to replace with.
 */
void replaceSubstring(std::string* str, const std::string& toSearch, const std::string& toReplace);
}  // namespace util
