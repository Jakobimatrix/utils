/**
 * @file demangle.cpp
 * @brief implementation for the demangle functions.
 * @date 29.05.2025
 * @author Jakob Wandel
 * @version 1.0
 **/
#include <string>
#include <cstdlib>
#include <utils/debug/demangle.hpp>
#include <utils/string/searchAndReplace.hpp>


#ifndef _WIN32
#include <cxxabi.h>
#endif

namespace util {
#ifndef _WIN32
std::string demangle(const char* name) {
  int status     = -1;
  char* realname = abi::__cxa_demangle(name, nullptr, nullptr, &status);
  std::string result((realname != nullptr) ? realname : name);
  free(realname);  // NOLINT abi::__cxa_demangle returns a char* allocated with malloc, which I must free... Anol shalom Anol sheh lay konnud de ne um Flavum nom de leesh
  const std::string stringDemangle(
    "std::__cxx11::basic_string<char, std::char_traits<char>, "
    "std::allocator<char> >");
  replaceSubstring(&result, stringDemangle, "std::string");
  return result;
}
#else
std::string demangle(const char* name) { return name; }
#endif

}  // namespace util
