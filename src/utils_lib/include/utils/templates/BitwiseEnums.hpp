/**
 * @file BitwiseEnums.hpp
 * @brief Contains template to treat Enum class like integer. You can use now bitwise operations.
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
 * @version 1.0
 * @date 2021
 */

#pragma once

#include <type_traits>

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator~(const T &value) {
  return static_cast<T>(~static_cast<typename std::underlying_type<T>::type>(value));
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator|(const T &left, const T &right) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(left) |
                        static_cast<typename std::underlying_type<T>::type>(right));
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T &operator|=(T &left, const T &right) {
  return left = left | right;
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator&(const T &left, const T &right) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(left) &
                        static_cast<typename std::underlying_type<T>::type>(right));
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T &operator&=(T &left, const T &right) {
  return left = left & right;
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator>>(const T &value,
                              const typename std::underlying_type<T>::type &shift) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(value) >> shift);
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T &operator>>=(T &value, const typename std::underlying_type<T>::type &shift) {
  return value = value >> shift;
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T operator<<(const T &value,
                              const typename std::underlying_type<T>::type &shift) {
  return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(value) << shift);
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr T &operator<<=(T &value, const typename std::underlying_type<T>::type &shift) {
  return value = value << shift;
}

template <typename T, typename = enable_if_t<std::is_enum<T>::value>>
inline constexpr bool isSet(const T &mask, const T &probe) {
  return static_cast<bool>(mask & probe);
}
