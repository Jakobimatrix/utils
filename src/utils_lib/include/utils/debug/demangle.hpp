/**
 * @file demangle.hpp
 * @brief For debuging template types
 *
 * @version 1.0
 * @date 2023
 */

#pragma once

#include <cstddef>
#include <string>
#include <format>
#include <map>
#include <typeinfo>
#include <iostream>
#include <variant>

namespace util {

/**
 * @brief Demangles a mangled C++ type name.
 *
 * Converts a mangled type name (as returned by typeid().name()) into a human-readable string.
 *
 * @param name The mangled type name.
 * @return std::string The demangled, human-readable type name.
 */
std::string demangle(const char* name);


/**
 * @brief Prints the demangled type of the given object to std::cout.
 *
 * Uses demangle() and typeid to print the type of the provided object.
 *
 * @tparam T Type of the object.
 * @param obj The object whose type will be printed.
 */
template <typename T>
void printType(const T& /*unused*/) {
  std::cout << demangle(typeid(T).name()) << '\n';
}

/**
 * @brief Prints the demangled type of a std::map in a readable format.
 *
 * Specialization for std::map to print its key and value types in a more readable way.
 *
 * @tparam T1 Key type of the map.
 * @tparam T2 Value type of the map.
 * @param map The map whose type will be printed.
 */
template <typename T1, typename T2>
void printType(const std::map<T1, T2>& /*unused*/) {
  std::cout << std::format("std::map<{}, {}>",
                           demangle(typeid(T1).name()),
                           demangle(typeid(T2).name()))
            << '\n';
}

/**
 * @brief Prints the demangled types of all alternatives in a std::variant.
 *
 * Recursively prints the type of each alternative in the given std::variant type.
 *
 * @tparam Variant The std::variant type.
 * @tparam Index The current index (used internally, defaults to 0).
 */
template <typename Variant, std::size_t Index = 0>
void printVariantTypes() {
  if constexpr (Index < std::variant_size_v<Variant>) {
    using CurrentType = std::variant_alternative_t<Index, Variant>;
    printType(CurrentType{});
    printVariantTypes<Variant, Index + 1>();
  }
}

}  // namespace util
