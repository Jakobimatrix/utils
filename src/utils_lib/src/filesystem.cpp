/**
 * @file filesystem.cpp
 * @brief implementation for the filesystem functions.
 * @date 29.05.2025
 * @author Jakob Wandel
 **/

#include <algorithm>
#include <iostream>
#include <string>
#include <system_error>
#include <utils/filesystem/filesystem.hpp>
#include <fstream>
#include <filesystem>

namespace util {


std::wstring getLastPathComponent(const std::filesystem::path& path) noexcept {
  // Use filename() directly, as it will handle cases with and without trailing slashes
  try {
    if (path.has_filename()) {
      return path.filename().wstring();
    }
    std::error_code error_code;
    if (std::filesystem::is_regular_file(path, error_code)) {
      if (error_code) {
        return L"";
      }
      return path.filename().wstring();
    }
    return path.parent_path().filename().wstring();  // Directory path with a trailing slash
  } catch (...) {
    std::cerr << "Something terrible went wrong with your path: " << path << "\n";
    return L"";
  }
}


std::wstring getLastPathComponent(const std::filesystem::directory_entry& entry) noexcept {
  return getLastPathComponent(entry.path());
}

bool hasHiddenElement(const std::filesystem::path& path) noexcept {
  return std::ranges::any_of(path, [](const auto& part) {
    if (part.empty()) {
      return false;
    }
    auto fn   = part.filename();
    auto iter = fn.begin();
    return iter != fn.end() && (*iter).native()[0] == '.';
  });
}



bool isWritable(const std::filesystem::path& source) {
  namespace fs = std::filesystem;

  if (!fs::exists(source)) {
    return false;  // does not exist
  }

  if (fs::is_directory(source)) {
    // Try creating a temp file inside the directory
    auto testFile = source / ".writetest.tmp";
    std::ofstream ofs(testFile.string(), std::ios::out | std::ios::trunc);
    if (ofs) {
      ofs.close();
      fs::remove(testFile);  // cleanup
      return true;
    }
    return false;
  }
  // It's a file: check if we can open in append/write mode
  const std::ofstream ofs(source, std::ios::app);
  return ofs.good();
}

}  // namespace util
