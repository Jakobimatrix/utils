/**
 * @file BitwiseEnums.hpp
 * @brief Contains template to treat Enum class like integer. You can use now bitwise operations.
 *
 * { // dont be in a global namespace
 * using namespace enum_operations; // enable sketchy operations
 *
 * enum class DBG_LVL{
 *   NONE = 0x0,
 *   INFO = 0x1,
 *   WARN = 0x2,
 *   ERROR = 0x4,
 *   CRITICAL = 0x8
 * };
 *
 *
 * template<DBG_LVL lvl>
 * void printError(DBG_LVL type, const std::string_view& msg){
 *    if constexpr(isSet(lvl, type)){
 *      printf("=%.*s=\n", static_cast<int>(msg.length()), msg.data());
 *    }
 * }
 *
 * constexpr DBG_LVL DEBUG_LEVEL = DBG_LVL::ERROR | DBG_LVL::CRITICAL;
 * ...
 *
 * printError<DEBUG_LEVEL>(DBG_LVL:WARNING, "this is just a warning")
 *
 * } // leave namespace of sketchy enum operations
 *
 * @version 2.0
 * @date 2025.08.24
 */

#pragma once

#include <type_traits>

namespace enum_operations {

template <bool B, class T = void>
using enable_if_t = std::enable_if_t<B, T>;

template <typename T>
constexpr T operator~(const T& value)
  requires std::is_enum_v<T>
{
  return static_cast<T>(~static_cast<std::underlying_type_t<T>>(value));
}

template <typename T>
constexpr T operator|(const T& left, const T& right)
  requires std::is_enum_v<T>
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(left) |
                        static_cast<std::underlying_type_t<T>>(right));
}

template <typename T>
constexpr T& operator|=(T& left, const T& right)
  requires std::is_enum_v<T>
{
  return left = left | right;
}

template <typename T>
constexpr T operator&(const T& left, const T& right)
  requires std::is_enum_v<T>
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(left) &
                        static_cast<std::underlying_type_t<T>>(right));
}

template <typename T>
constexpr T& operator&=(T& left, const T& right)
  requires std::is_enum_v<T>
{
  return left = left & right;
}

template <typename T>
constexpr T operator>>(const T& value, const typename std::underlying_type<T>::type& shift)
  requires std::is_enum_v<T>
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(value) >> shift);
}

template <typename T>
constexpr T& operator>>=(T& value, const typename std::underlying_type<T>::type& shift)
  requires std::is_enum_v<T>
{
  return value = value >> shift;
}

template <typename T>
constexpr T operator<<(const T& value, const typename std::underlying_type<T>::type& shift)
  requires std::is_enum_v<T>
{
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(value) << shift);
}

template <typename T>
constexpr T& operator<<=(T& value, const typename std::underlying_type<T>::type& shift)
  requires std::is_enum_v<T>
{
  return value = value << shift;
}

template <typename T>
constexpr bool isSet(const T& mask, const T& probe)
  requires std::is_enum_v<T>
{
  return static_cast<bool>(mask & probe);
}
}  // namespace enum_operations
