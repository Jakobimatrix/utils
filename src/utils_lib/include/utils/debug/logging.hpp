/**
 * @file logging.hpp
 * @brief Modern C++20 logging with fmt-like syntax using std::format, console colors and styles
 *
 * @version 2.0
 * @author Jakob
 * @date 2025.10.03
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include <format>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace dbg {

struct SourceLocation {
  const char* file;
  const char* func;
  int line;
};

inline std::string to_string(const SourceLocation& loc) {
  return std::string(loc.file) + "::" + loc.func +
         "() Line: " + std::to_string(loc.line);
}

}  // namespace dbg

#if defined(__clang__) || defined(__GNUC__)
#define CURRENT_FUNC __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define CURRENT_FUNC __FUNCSIG__
#else
#define CURRENT_FUNC __func__
#endif

#define CURRENT_SOURCE_LOCATION \
  ::dbg::SourceLocation { __FILE__, CURRENT_FUNC, __LINE__ }

namespace console {

enum class Color : uint8_t {
  Default,
  Black,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White,
  Orange,
  BrightBlack,
  BrightRed,
  BrightGreen,
  BrightYellow,
  BrightBlue,
  BrightMagenta,
  BrightCyan,
  BrightWhite
};

enum class Style : uint8_t {
  Reset,
  Bold,
  Dim,
  Italic,
  Underline,
  Blink,
  Reverse,
  Hidden,
  Strikethrough
};

namespace console::detail {

inline const char* color_code(Color color) {
  switch (color) {
    case Color::Black:
      return "30";
    case Color::Red:
      return "31";
    case Color::Green:
      return "32";
    case Color::Yellow:
      return "33";
    case Color::Blue:
      return "34";
    case Color::Magenta:
      return "35";
    case Color::Cyan:
      return "36";
    case Color::White:
      return "37";
    case Color::Orange:
      return "38";
    case Color::BrightBlack:
      return "90";
    case Color::BrightRed:
      return "91";
    case Color::BrightGreen:
      return "92";
    case Color::BrightYellow:
      return "93";
    case Color::BrightBlue:
      return "94";
    case Color::BrightMagenta:
      return "95";
    case Color::BrightCyan:
      return "96";
    case Color::BrightWhite:
      return "97";
    default:
      return "39";
  }
}

inline const char* style_code(Style style) {
  switch (style) {
    case Style::Bold:
      return "1";
    case Style::Dim:
      return "2";
    case Style::Italic:
      return "3";
    case Style::Underline:
      return "4";
    case Style::Blink:
      return "5";
    case Style::Reverse:
      return "7";
    case Style::Hidden:
      return "8";
    case Style::Strikethrough:
      return "9";
    case Style::Reset:
      return "0";
  }
  return "0";
}

#if defined(_WIN32)
inline WORD win32_color(Color color, bool bold) {
  switch (color) {
    case Color::Black:
      return 0;
    case Color::Red:
      return FOREGROUND_RED | (bold ? FOREGROUND_INTENSITY : 0);
    case Color::Green:
      return FOREGROUND_GREEN | (bold ? FOREGROUND_INTENSITY : 0);
    case Color::Yellow:
      return FOREGROUND_RED | FOREGROUND_GREEN | (bold ? FOREGROUND_INTENSITY : 0);
    case Color::Blue:
      return FOREGROUND_BLUE | (bold ? FOREGROUND_INTENSITY : 0);
    case Color::Magenta:
      return FOREGROUND_RED | FOREGROUND_BLUE | (bold ? FOREGROUND_INTENSITY : 0);
    case Color::Cyan:
      return FOREGROUND_GREEN | FOREGROUND_BLUE | (bold ? FOREGROUND_INTENSITY : 0);
    case Color::White:
      return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
             (bold ? FOREGROUND_INTENSITY : 0);
    case Color::Orange:
      return FOREGROUND_RED | (bold ? FOREGROUND_INTENSITY : 0);
    case Color::BrightBlack:
      return FOREGROUND_INTENSITY;
    case Color::BrightRed:
      return FOREGROUND_RED | FOREGROUND_INTENSITY;
    case Color::BrightGreen:
      return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case Color::BrightYellow:
      return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case Color::BrightBlue:
      return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    case Color::BrightMagenta:
      return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    case Color::BrightCyan:
      return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    case Color::BrightWhite:
      return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    default:
      return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  }
}
#endif

}  // namespace console::detail

template <Color C = Color::Default, Style... S>
struct PrettyConsole {
  explicit PrettyConsole(std::string text)
      : _text(std::move(text)) {}

  std::string str() const {
#if defined(_WIN32)
    HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hConsole, &info);
    WORD original = info.wAttributes;
    bool bold     = ((false || ... || (S == Style::Bold)));
    SetConsoleTextAttribute(hConsole, detail::win32_color(C, bold));
    std::string result = _text;
    SetConsoleTextAttribute(hConsole, original);
    return result;
#else
    std::ostringstream oss;
    oss << "\033[";
    oss << ::console::console::detail::color_code(C);
    ((oss << ";" << ::console::console::detail::style_code(S)), ...);
    oss << "m" << _text << "\033[0m";
    return oss.str();
#endif
  }

  const char* c_str() const {
    _cached = str();
    return _cached.c_str();
  }

 private:
  std::string _text;
  mutable std::string _cached;
};

}  // namespace console

namespace dbg {

// Base log function
inline void log_message(const char* level, const SourceLocation& loc, const std::string& msg) {
  std::cerr << level << "\t " << to_string(loc) << ": " << msg << "\n";
}

// fmt-style logging using std::vformat (format string can be runtime or literal)
#include <string_view>
template <typename... Args>
inline void logf(const char* level, const SourceLocation& loc, std::string_view fmt, Args&&... args) {
  log_message(level, loc, std::vformat(fmt, std::make_format_args(args...)));
}

// Debug
inline void debug(const SourceLocation& loc, const std::string& msg) {
  log_message("[DEBUG]", loc, msg);
}
template <typename... Args>
inline void debugf(const SourceLocation& loc, std::string_view fmt, Args&&... args) {
  logf("[DEBUG]", loc, fmt, std::forward<Args>(args)...);
}

// Warning
inline void warning(const SourceLocation& loc, const std::string& msg) {
  log_message(console::PrettyConsole<console::Color::Orange, console::Style::Bold>("[WARN]")
                .c_str(),
              loc,
              msg);
}
template <typename... Args>
inline void warningf(const SourceLocation& loc, std::string_view fmt, Args&&... args) {
  logf(console::PrettyConsole<console::Color::Orange, console::Style::Bold>(
         "[WARN]")
         .c_str(),
       loc,
       fmt,
       std::forward<Args>(args)...);
}

// Error
inline void error(const SourceLocation& loc, const std::string& msg) {
  log_message(
    console::PrettyConsole<console::Color::Red, console::Style::Bold>("[ERROR]").c_str(), loc, msg);
}
template <typename... Args>
inline void errorf(const SourceLocation& loc, std::string_view fmt, Args&&... args) {
  logf(
    console::PrettyConsole<console::Color::Red, console::Style::Bold>("[ERROR]").c_str(),
    loc,
    fmt,
    std::forward<Args>(args)...);
}

// Assert
inline void assert_that(const SourceLocation& loc, bool expr, const std::string& msg) {
  if (!expr) {
    log_message(
      console::PrettyConsole<console::Color::Red, console::Style::Bold, console::Style::Blink>("[ASSERT]")
        .c_str(),
      loc,
      msg);
    std::cerr << '\n';
    assert(expr && "Debug assert failed. See error message above!");
  }
}

template <typename... Args>
inline void assertf(const SourceLocation& loc, bool expr, std::string_view fmt, Args&&... args) {
  if (!expr) {
    assert_that(loc, false, std::vformat(fmt, std::make_format_args(args...)));
  }
}

}  // namespace dbg
