#pragma once

#include <typeinfo>
#include <cxxabi.h>

#include <utils/searchAndReplace.hpp>

std::string demangle(const char* name) {
  int status = -1;
  char* realname = abi::__cxa_demangle(name, 0, 0, &status);
  std::string result(realname ? realname : name);
  free(realname);
  const std::string stringDemangle("std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >");
  replaceSubstring(&result, stringDemangle, "std::string");
  return result;
}

// Visitor that prints the type of each element
template<typename T>
void printType(const T&) {
  std::cout << demangle(typeid(T).name()) << std::endl;
}

// Visitor for map types to print in a more readable way
template <typename T1, typename T2>
void printType(const std::map<T1, T2>&) {
  std::cout << "std::map<" << demangle(typeid(T1).name()) << ", " << demangle(typeid(T2).name()) << ">" << std::endl;
}

template<typename Variant, std::size_t Index = 0>
void printVariantTypes() {
  if constexpr (Index < std::variant_size_v<Variant>) {
    using CurrentType = std::variant_alternative_t<Index, Variant>;
    printType(CurrentType{});
    printVariantTypes<Variant, Index + 1>();
  }
}
