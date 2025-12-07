/**
 * @file filesystem.hpp
 * @brief Adds some utility functions for filesystem operations.
 *
 * @date 2023
 */

#pragma once

#include <filesystem>
#include <string>

namespace util {

/**
 * @brief Extracts the rightmost entry name from a given filesystem path.
 *
 * This function returns the last component from the provided path.
 * If the path is a file, it directly returns the filename.
 * If the path is a directory with a trailing slash, it returns the directory name.
 *
 * If permission denied, non-existent path, invalid path, an empty string is returned.
 *
 * @param path The filesystem path from which to extract the filename or directory name.
 * @return std::wstring The name of the file or the directory at the end of the path.
 */
std::wstring getLastPathComponent(const std::filesystem::path& path) noexcept;

/**
 * @brief Extracts the rightmost entry name from a directory entry.
 *
 * This function calls `getLastPathComponent` on the directory entry’s path to retrieve
 * the last component of the path.
 *
 * @param entry The filesystem directory entry from which to extract the filename or directory name.
 * @return std::wstring The name of the file or the directory at the end of the path in the entry.
 */
std::wstring getLastPathComponent(const std::filesystem::directory_entry& entry) noexcept;

/**
 * @brief Checks if any element in the given path is hidden (starts with a '.').
 *
 * @param path The filesystem path to check.
 * @return true If at least one element in the path is hidden.
 * @return false Otherwise.
 */
bool hasHiddenElement(const std::filesystem::path& path) noexcept;

/**
 * @brief Check if a given path exists and is writable.
 *
 * @param source Path to check.
 * @return true if path exists and is writable, false otherwise.
 */
bool isWritable(const std::filesystem::path& source);


}  // namespace util
