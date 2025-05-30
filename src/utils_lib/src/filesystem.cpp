#include <utils/filesystem/filesystem.hpp>

#include <filesystem>

namespace util {


std::wstring getLastPathComponent(const std::filesystem::path& path) {
  // Use filename() directly, as it will handle cases with and without trailing slashes
  if (path.has_filename() || std::filesystem::is_regular_file(path)) {
    return path.filename().wstring();
  }
  return path.parent_path().filename().wstring();  // Directory path with a trailing slash
}


std::wstring getLastPathComponent(const std::filesystem::directory_entry& entry) {
  return getLastPathComponent(entry.path());
}

bool hasHiddenElement(const std::filesystem::path& path) {
  for (const auto& part : path) {
    if (!part.empty() && part.filename().string().starts_with('.')) {
      return true;
    }
  }
  return false;
}


}  // namespace util
