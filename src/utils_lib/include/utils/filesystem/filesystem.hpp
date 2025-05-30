/**
 * @file filesystem.hpp
 * @brief Adds some utility functions for filesystem operations.
 * 
 * @version 1.0
 * @date 2023
 */

#pragma once

#include <filesystem>

namespace util {

/**
 * @brief Extracts the rightmost entry name from a given filesystem path.
 *
 * This function returns the last component from the provided path.
 * If the path is a file, it directly returns the filename.
 * If the path is a directory with a trailing slash, it returns the directory name.
 *
 * @param path The filesystem path from which to extract the filename or directory name.
 * @return std::wstring The name of the file or the directory at the end of the path.
 */
std::wstring getLastPathComponent(const std::filesystem::path& path);

/**
 * @brief Extracts the rightmost entry name from a directory entry.
 *
 * This function calls `getLastPathComponent` on the directory entry’s path to retrieve
 * the last  component of the path.
 *
 * @param entry The filesystem directory entry from which to extract the filename or directory name.
 * @return std::wstring The name of the file or the directory at the end of the path in the entry.
 */
std::wstring inline getLastPathComponent(const std::filesystem::directory_entry& entry);

/**
 * @brief Checks if any element in the given path is hidden (starts with a '.').
 *
 * @param path The filesystem path to check.
 * @return true If at least one element in the path is hidden.
 * @return false Otherwise.
 */
bool inline hasHiddenElement(const std::filesystem::path& path);


}  // namespace util
