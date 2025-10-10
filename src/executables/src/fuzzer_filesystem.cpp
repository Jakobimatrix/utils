/**
 * @file fuzzer_filesystem.cpp
 * @brief fuzzer tries to break searchAndReplace.hpp
 * @date 29.05.2025
 * @author Jakob Wandel
 * @version 1.0
 **/

// NOLINTBEGIN (misc-include-cleaner) // depends on the cmake configuration
#include <utils/filesystem/filesystem.hpp>
#include <utils/data/BinaryDataReader.hpp>

#include <bit>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
// NOLINTEND (misc-include-cleane


namespace {

/**
 * @brief This function uses the given data to create strings than calls the functions to fuzz.
 *
 * @param data Pointer to data begin.
 * @param size Size of the Data.
 */
inline void callFilesystemFunctions(serialize::BinaryDataReader& data) noexcept {
  try {
    std::string string_data;
    std::wstring wide_string_data;

    if (!data.readNext(&string_data)) {
      std::cerr << "Failed to read binary to string.\n";
      return;
    }

    if (!data.readNext(&wide_string_data)) {
      std::cerr << "Failed to read binary to wstring.\n";
      return;
    }

    const std::filesystem::path pathFromString{string_data};
    const std::filesystem::path pathFromWString{wide_string_data};
    [[maybe_unused]] const auto val1 = util::getLastPathComponent(pathFromString);
    [[maybe_unused]] const auto val2 = util::hasHiddenElement(pathFromString);
    [[maybe_unused]] const auto val3 = util::getLastPathComponent(pathFromWString);
    [[maybe_unused]] const auto val4 = util::hasHiddenElement(pathFromWString);
  } catch (...) {
    __builtin_trap();
  }
}
}  // namespace

#if FUZZER_ACTIVE
// we compiled in release mode, The fuzzer can do its magic


extern "C" int LLVMFuzzerTestOneInput(const unsigned char* binary_data, unsigned long size) {

  util::BinaryDataInterpreter data{binary_data, static_cast<size_t>(size)};
  callFilesystemFunctions(data);
  return 0;
}


#else
// We compiled in debug mode, you can call badFunction yourself with the data which crashed badFunction and see what went wrong

/**
 * @brief Entry point of the filesystem fuzzer utility.
 *
 * This program reads a file, checks if it exists, and prepares it for
 * debugging with search-and-replace functionality.
 *
 * @param argc Number of command-line arguments.
 * @param argv Argument vector.
 * @return int Exit code (0 on success, 1 on failure).
 */
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0]  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) Thats how its done unfortunately
              << " <file_path>\n";
    return 1;
  }

  const std::filesystem::path file_path(argv[1]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) Thats how its done unfortunately

  std::error_code error_code;
  if (!std::filesystem::exists(file_path, error_code) || error_code) {
    std::cerr << "File does not exist or error checking file: " << error_code.message()
              << " [for " << file_path << "]\n";
    return 1;
  }
  serialize::BinaryDataReader data(file_path, std::endian::little);
  if (!data.isReady()) {
    std::cerr << "Failed to read file: " << file_path << "\n";
    return 1;
  }
  std::cout << "\nFile found and read. Now attach debugger and press enter.\n";
  std::cout
    << "If you get an error from ptrace 'Could not attach to the process.' "
       "Use 'echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope' to relax "
       "restrictions temporarily.\n"
    << std::flush;
  getchar();
  callFilesystemFunctions(data);

  return 0;
}


#endif
