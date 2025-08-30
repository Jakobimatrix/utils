/**
 * @file fuzzer_searchAndReplace.cpp
 * @brief fuzzer tries to break searchAndReplace.hpp
 * @date 29.05.2025
 * @author Jakob Wandel
 * @version 1.0
 **/

#include <iostream>
#include <utils/data/BinaryDataInterpreter.hpp>
#include <utils/string/searchAndReplace.hpp>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <utility>
#include <system_error>
#include <vector>

namespace {
/**
 * @brief This function uses the given data to create strings for search and replace. Than calls the functions in searchAndReplace.hpp.
 *
 * @param data Pointer to data begin.
 * @param size Size of the Data.
 */
constexpr void callSearchAndReplace(util::BinaryDataInterpreter& data) {
  const std::vector<uint8_t> start_marker = {255, 0, 255, 0};
  const std::vector<uint8_t> end_marker   = {0, 0, 0, 0};

  std::vector<std::string> extracted_strings;

  // Try to extract three strings
  for (int i = 0; i < 3; ++i) {
    if (!data.findNextBytesAndAdvance(start_marker, true)) {
      break;
    }

    const size_t str_start = data.getCursor();
    if (!data.findNextBytesAndAdvance(end_marker, false)) {
      break;
    }
    const size_t str_end = data.getCursor();
    if (!data.setCursor(str_start)) {
      __builtin_trap();  // This should not happen! str_start was read succesfully, we can always advance behind it
    }
    std::string stringValue;
    if (!data.readNext(&stringValue, str_end - str_start)) {
      std::cerr << "Failed to read next bytes after start marker.\n";
      break;
    }
    extracted_strings.push_back(std::move(stringValue));
  }

  if (extracted_strings.size() == 3) {
    std::string base             = extracted_strings[0];
    const std::string& toSearch  = extracted_strings[1];
    const std::string& toReplace = extracted_strings[2];
    util::replaceSubstring(&base, toSearch, toReplace);
  }
}
}  // namespace

#if FUZZER_ACTIVE
// we compiled in release mode, The fuzzer can do its magic


extern "C" int LLVMFuzzerTestOneInput(const unsigned char* binary_data, unsigned long size) {

  util::BinaryDataInterpreter data{binary_data, static_cast<size_t>(size)};
  callSearchAndReplace(data);
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
  util::BinaryDataInterpreter data(file_path);
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
  callSearchAndReplace(data);

  return 0;
}


#endif
