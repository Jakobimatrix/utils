#include <utils/debug/demangle.hpp>
#include <utils/string/searchAndReplace.hpp>


#include <typeinfo>
#ifndef _WIN32
#include <cxxabi.h>
#endif

namespace util {
#ifndef _WIN32
std::string demangle(const char* name) {
  int status     = -1;
  char* realname = abi::__cxa_demangle(name, 0, 0, &status);
  std::string result(realname ? realname : name);
  free(realname);
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