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
std::wstring inline getLastPathComponent(const std::filesystem::path& path) {
  // Use filename() directly, as it will handle cases with and without trailing slashes
  if (path.has_filename() || std::filesystem::is_regular_file(path)) {
    return path.filename().wstring();
  }
  return path.parent_path().filename().wstring();  // Directory path with a trailing slash
}

/**
 * @brief Extracts the rightmost entry name from a directory entry.
 *
 * This function calls `getLastPathComponent` on the directory entry’s path to retrieve
 * the last  component of the path.
 *
 * @param entry The filesystem directory entry from which to extract the filename or directory name.
 * @return std::wstring The name of the file or the directory at the end of the path in the entry.
 */
std::wstring inline getLastPathComponent(const std::filesystem::directory_entry& entry) {
  return getLastPathComponent(entry.path());
}

/**
 * @brief Checks if any element in the given path is hidden (starts with a '.').
 *
 * @param path The filesystem path to check.
 * @return true If at least one element in the path is hidden.
 * @return false Otherwise.
 */
bool inline hasHiddenElement(const std::filesystem::path& path) {
  for (const auto& part : path) {
    if (!part.empty() && part.filename().string().starts_with('.')) {
      return true;
    }
  }
  return false;
}


}  // namespace util
