/**
 * @file fuzzer_example.hpp
 * @brief fuzzer tries to break searchAndReplace.hpp
 * @date 29.05.2025
 * @author Jakob Wandel
 * @version 1.0
 **/

#include <utils/string/searchAndReplace.hpp>
#include <utils/data/BinaryDataInterpreter.hpp>


#include <iostream>


/**
 * @brief This function uses the given data to create strings for search and replace. Than calls the functions in searchAndReplace.hpp.
 *
 * @param data Pointer to data begin.
 * @param size Size of the Data.
 */
inline void callSearchAndReplace(util::BinaryDataInterpreter& data) {
  const std::vector<uint8_t> start_marker = {255, 0, 255, 0};
  const std::vector<uint8_t> end_marker   = {0, 0, 0, 0};

  std::vector<std::string> extracted_strings;

  // Try to extract two strings
  for (int i = 0; i < 2; ++i) {
    if (!data.findNextBytesAndAdvance(start_marker, true))
      break;

    size_t str_start = data.getCursor();
    if (!data.findNextBytesAndAdvance(end_marker, false))
      break;
    size_t str_end = data.getCursor();
    data.setCursor(str_start);
    std::string s;
    if (!data.readNext(&s, str_end - str_start)) {
      std::cerr << "Failed to read next bytes after start marker." << std::endl;
      break;
    }
    extracted_strings.push_back(std::move(s));
  }

  if (extracted_strings.size() == 2) {
    std::string base             = extracted_strings[0];
    const std::string& toSearch  = extracted_strings[0];
    const std::string& toReplace = extracted_strings[1];
    util::replaceSubstring(&base, toSearch, toReplace);
  }
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
  if (!data.ready()) {
    std::cerr << "Failed to read file: " << file_path << "\n";
    return 1;
  }
  printf("\nFile found and read. Now attach debugger and press enter.\n");
  getchar();
  callSearchAndReplace(data);

  return 0;
}


#endif
