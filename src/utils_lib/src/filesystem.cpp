#include <algorithm>
#include <iostream>
#include <string>
#include <system_error>
#include <utils/filesystem/filesystem.hpp>

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


}  // namespace util
