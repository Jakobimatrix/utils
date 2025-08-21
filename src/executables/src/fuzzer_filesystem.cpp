/**
 * @file fuzzer_example.hpp
 * @brief fuzzer tries to break searchAndReplace.hpp
 * @date 29.05.2025
 * @author Jakob Wandel
 * @version 1.0
 **/

#include <utils/filesystem/filesystem.hpp>
#include <utils/data/BinaryDataInterpreter.hpp>


#include <iostream>


/**
 * @brief This function uses the given data to create strings for search and replace. Than calls the functions in searchAndReplace.hpp.
 *
 * @param data Pointer to data begin.
 * @param size Size of the Data.
 */
inline void callSearchAndReplace(util::BinaryDataInterpreter& data) {
  std::string string_data;
  std::wstring wide_string_data;

  if (!data.readNext(&string_data, data.size())) {
    std::cerr << "Failed to read binary to string." << std::endl;
    return;
  }

  size_t wstring_size = data.size() % 2 == 0 ? data.size() : data.size() - 1;
  if (data.size() == 0) {
    wstring_size = 0;
  }

  if (!data.readNext(&wide_string_data, wstring_size)) {
    std::cerr << "Failed to read binary to wstring." << std::endl;
    return;
  }

  std::filesystem::path pathFromString{string_data};
  std::filesystem::path pathFromWString{wide_string_data};
  [[maybe_unused]] const auto val1 = util::getLastPathComponent(pathFromString);
  [[maybe_unused]] const auto val2 = util::hasHiddenElement(pathFromString);
  [[maybe_unused]] const auto val3 = util::getLastPathComponent(pathFromWString);
  [[maybe_unused]] const auto val4 = util::hasHiddenElement(pathFromWString);
}


#if FUZZER_ACTIVE
// we compiled in release mode, The fuzzer can do its magic


extern "C" int LLVMFuzzerTestOneInput(const unsigned char* binary_data, unsigned long size) {

  util::BinaryDataInterpreter data{binary_data, static_cast<size_t>(size)};
  callSearchAndReplace(data);
  return 0;
}


#else
// We compiled in debug mode, you can call badFunction yourself with the data which crashed badFunction and see what went wrong

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <file_path>\n";
    return 1;
  }

  std::filesystem::path file_path(argv[1]);

  if (!std::filesystem::exists(file_path)) {
    std::cerr << "File does not exist: " << file_path << "\n";
    return 1;
  }
  util::BinaryDataInterpreter data(file_path);
  if (!data.isReady()) {
    std::cerr << "Failed to read file: " << file_path << "\n";
    return 1;
  }
  printf("\nFile found and read. Now attach debugger and press enter.\n");
  printf(
    "If you get an error from ptrace 'Could not attach to the process.' "
    "Use 'echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope' to relax "
    "restrictions temporarily.\n");
  getchar();
  callSearchAndReplace(data);

  return 0;
}


#endif
