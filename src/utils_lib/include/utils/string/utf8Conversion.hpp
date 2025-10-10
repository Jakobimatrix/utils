/**
 * @file utf8Conversion.hpp
 * @brief Exception-free, constexpr-safe UTF-8 ↔ UTF-16/32 conversion utilities (namespace util).
 *
 * @details
 * - Functions are `constexpr` and `noexcept` (C++20).
 * - Portable for platforms where `wchar_t` is UTF-16 (Windows) or UTF-32 (many Unix).
 * - Returns boolean for success; no error codes or exceptions.
 *
 * Author: ChatGPT(Dont juge me, This is tested with unit and fuzzy, but I have to admit, that I dont have any Idea whow the conversion from and to utf8 binary wise works.)
 * @date 2025-10-06
 */

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <limits>
#include <cstdint>
#include <utility>

namespace util {

namespace utf {

// Unicode codepoint limits
constexpr uint32_t UNICODE_MAX_CODEPOINT = 0x10FFFF;
constexpr uint32_t UNICODE_PLANE1_START  = 0x10000;

// UTF-8 minimum values for each byte-length (for overlong check)
constexpr uint32_t UTF8_MIN_1BYTE = 0x00;
constexpr uint32_t UTF8_MIN_2BYTE = 0x80;
constexpr uint32_t UTF8_MIN_3BYTE = 0x800;
constexpr uint32_t UTF8_MIN_4BYTE = UNICODE_PLANE1_START;

// UTF-16 surrogate ranges
constexpr uint32_t UTF16_HIGH_SURROGATE_MIN = 0xD800;
constexpr uint32_t UTF16_HIGH_SURROGATE_MAX = 0xDBFF;
constexpr uint32_t UTF16_LOW_SURROGATE_MIN  = 0xDC00;
constexpr uint32_t UTF16_LOW_SURROGATE_MAX  = 0xDFFF;

// UTF-8 bitmasks and prefixes
constexpr uint8_t UTF8_CONT_MASK         = 0x3F;
constexpr uint8_t UTF8_CONT_PREFIX       = 0x80;
constexpr uint8_t UTF8_TWO_BYTE_PREFIX   = 0xC0;
constexpr uint8_t UTF8_THREE_BYTE_PREFIX = 0xE0;
constexpr uint8_t UTF8_FOUR_BYTE_PREFIX  = 0xF0;

// UTF-8 masks for decoding
constexpr uint8_t UTF8_2BYTE_MASK        = 0x1F;
constexpr uint8_t UTF8_3BYTE_MASK        = 0x0F;
constexpr uint8_t UTF8_4BYTE_MASK        = 0x07;
constexpr uint8_t UTF8_4BYTE_PREFIX_MASK = 0xF8;

// Surrogate pair helpers
constexpr uint32_t UTF16_SURROGATE_SHIFT = 10;
constexpr uint32_t UTF16_SURROGATE_MASK  = 0x3FF;

// UTF-8 shifts
constexpr uint8_t UTF8_SHIFT_6  = 6;
constexpr uint8_t UTF8_SHIFT_12 = 12;
constexpr uint8_t UTF8_SHIFT_18 = 18;

// UTF-8 encoding thresholds
constexpr uint32_t UTF8_ONE_BYTE_MAX   = 0x7F;
constexpr uint32_t UTF8_TWO_BYTE_MAX   = 0x7FF;
constexpr uint32_t UTF8_THREE_BYTE_MAX = 0xFFFF;

}  // namespace utf

/**
 * @brief Convert a UTF-8 input to std::wstring (UTF-16 or UTF-32 depending on platform).
 *
 * @param input UTF-8 encoded input (string_view).
 * @param out   Output parameter; appended with the decoded wide string on success.
 * @return True on success.
 */
constexpr bool utf8ToWstring(std::string_view input, std::wstring* out)  // NOLINT (readability-function-cognitive-complexity)
  noexcept {

  if (!out) {
    return false;
  }
  std::wstring temp_out;
  constexpr bool is_wchar_16 =
    (std::numeric_limits<wchar_t>::max() == utf::UTF8_THREE_BYTE_MAX);

  const size_t input_size = input.size();

  auto decodeLeadingByte = [](uint8_t first, uint32_t& codePoint, size_t& len)  // NOLINT (bugprone-easily-swappable-parameters)
    constexpr noexcept {
      if (first <= utf::UTF8_ONE_BYTE_MAX) {
        codePoint = first;
        len       = 1;
        return true;
      }
      if ((first & utf::UTF8_THREE_BYTE_PREFIX) == utf::UTF8_TWO_BYTE_PREFIX) {
        codePoint = first & utf::UTF8_2BYTE_MASK;
        len       = 2;
        return true;
      }
      if ((first & utf::UTF8_FOUR_BYTE_PREFIX) == utf::UTF8_THREE_BYTE_PREFIX) {
        codePoint = first & utf::UTF8_3BYTE_MASK;
        len       = 3;
        return true;
      }
      if ((first & utf::UTF8_4BYTE_PREFIX_MASK) == utf::UTF8_FOUR_BYTE_PREFIX) {
        codePoint = first & utf::UTF8_4BYTE_MASK;
        len       = 4;
        return true;
      }
      return false;
    };

  auto appendContBytes =
    [](std::string_view str, size_t start, size_t len, uint32_t& codePoint)  // NOLINT (bugprone-easily-swappable-parameters)
    constexpr noexcept {
      for (size_t j = 1; j < len; ++j) {
        const uint8_t uchar =
          static_cast<uint8_t>(static_cast<unsigned char>(str[start + j]));  // NOLINT (useless-cast)
        if ((uchar & utf::UTF8_TWO_BYTE_PREFIX) != utf::UTF8_CONT_PREFIX) {
          return false;
        }
        codePoint = (codePoint << utf::UTF8_SHIFT_6) | (uchar & utf::UTF8_CONT_MASK);
      }
      return true;
    };

  auto isOverlong = [](uint32_t codePoint, size_t len) constexpr noexcept {
    return (len == 2 && codePoint < utf::UTF8_MIN_2BYTE) ||
           (len == 3 && codePoint < utf::UTF8_MIN_3BYTE) ||
           (len == 4 && codePoint < utf::UTF8_MIN_4BYTE);
  };

  auto isSurrogate = [](uint32_t codePoint) constexpr noexcept {
    return codePoint >= utf::UTF16_HIGH_SURROGATE_MIN && codePoint <= utf::UTF16_LOW_SURROGATE_MAX;
  };

  size_t i = 0;
  while (i < input_size) {
    const uint8_t first = static_cast<uint8_t>(static_cast<unsigned char>(input[i]));
    uint32_t codePoint = 0;
    size_t len         = 0;

    if (!decodeLeadingByte(first, codePoint, len)) {
      return false;
    }
    if (i + len > input_size) {
      return false;
    }
    if (!appendContBytes(input, i, len, codePoint)) {
      return false;
    }
    if (isOverlong(codePoint, len) || isSurrogate(codePoint) ||
        codePoint > utf::UNICODE_MAX_CODEPOINT) {
      return false;
    }

    if constexpr (is_wchar_16) {
      if (codePoint <= utf::UTF8_THREE_BYTE_MAX) {
        temp_out.push_back(static_cast<wchar_t>(codePoint));
      } else {
        const uint32_t cpOffset = codePoint - utf::UNICODE_PLANE1_START;
        temp_out.push_back(static_cast<wchar_t>(
          utf::UTF16_HIGH_SURROGATE_MIN +
          ((cpOffset >> utf::UTF16_SURROGATE_SHIFT) & utf::UTF16_SURROGATE_MASK)));
        temp_out.push_back(static_cast<wchar_t>(
          utf::UTF16_LOW_SURROGATE_MIN + (cpOffset & utf::UTF16_SURROGATE_MASK)));
      }
    } else {
      temp_out.push_back(static_cast<wchar_t>(codePoint));
    }

    i += len;
  }

  // Success: commit the decoded data to the caller-provided output.
  *out = std::move(temp_out);
  return true;
}

/**
 * @brief Convert a std::wstring (UTF-16 or UTF-32) to UTF-8.
 *
 * @param wstr Input wide-string view.
 * @param out  Output parameter; appended with UTF-8 bytes on success.
 * @return True on success.
 */
constexpr bool wstringToUtf8(std::wstring_view wstr, std::string* out)  // NOLINT (readability-function-cognitive-complexity)
  noexcept {
  if (!out) {
    return false;
  }
  std::string temp_out;
  constexpr bool is_wchar_16 =
    (std::numeric_limits<wchar_t>::max() == utf::UTF8_THREE_BYTE_MAX);

  const size_t input_size = wstr.size();

  auto pushUtf8Bytes = [](std::string& outStr, uint32_t codePoint) constexpr noexcept {
    if (codePoint <= utf::UTF8_ONE_BYTE_MAX) {
      outStr.push_back(static_cast<char>(codePoint));
    } else if (codePoint <= utf::UTF8_TWO_BYTE_MAX) {
      outStr.push_back(static_cast<char>(utf::UTF8_TWO_BYTE_PREFIX |
                                         (codePoint >> utf::UTF8_SHIFT_6)));
      outStr.push_back(static_cast<char>(utf::UTF8_CONT_PREFIX |
                                         (codePoint & utf::UTF8_CONT_MASK)));
    } else if (codePoint <= utf::UTF8_THREE_BYTE_MAX) {
      outStr.push_back(static_cast<char>(utf::UTF8_THREE_BYTE_PREFIX |
                                         (codePoint >> utf::UTF8_SHIFT_12)));
      outStr.push_back(static_cast<char>(
        utf::UTF8_CONT_PREFIX | ((codePoint >> utf::UTF8_SHIFT_6) & utf::UTF8_CONT_MASK)));
      outStr.push_back(static_cast<char>(utf::UTF8_CONT_PREFIX |
                                         (codePoint & utf::UTF8_CONT_MASK)));
    } else if (codePoint <= utf::UNICODE_MAX_CODEPOINT) {
      outStr.push_back(static_cast<char>(utf::UTF8_FOUR_BYTE_PREFIX |
                                         (codePoint >> utf::UTF8_SHIFT_18)));
      outStr.push_back(static_cast<char>(
        utf::UTF8_CONT_PREFIX | ((codePoint >> utf::UTF8_SHIFT_12) & utf::UTF8_CONT_MASK)));
      outStr.push_back(static_cast<char>(
        utf::UTF8_CONT_PREFIX | ((codePoint >> utf::UTF8_SHIFT_6) & utf::UTF8_CONT_MASK)));
      outStr.push_back(static_cast<char>(utf::UTF8_CONT_PREFIX |
                                         (codePoint & utf::UTF8_CONT_MASK)));
    } else {
      // Should never happen; caller checks codepoint
    }
  };

  size_t i = 0;
  while (i < input_size) {

    uint32_t codePoint = static_cast<uint32_t>(wstr[i]);

    if constexpr (is_wchar_16) {
      if (codePoint >= utf::UTF16_HIGH_SURROGATE_MIN && codePoint <= utf::UTF16_HIGH_SURROGATE_MAX) {
        if (i + 1 >= input_size) {
          return false;
        }
        uint32_t const low = static_cast<uint32_t>(wstr[i + 1]);
        if (low < utf::UTF16_LOW_SURROGATE_MIN || low > utf::UTF16_LOW_SURROGATE_MAX) {
          return false;
        }
        codePoint = utf::UNICODE_PLANE1_START +
                    (((codePoint - utf::UTF16_HIGH_SURROGATE_MIN) << utf::UTF16_SURROGATE_SHIFT) |
                     (low - utf::UTF16_LOW_SURROGATE_MIN));
        ++i;
      } else if (codePoint >= utf::UTF16_LOW_SURROGATE_MIN &&
                 codePoint <= utf::UTF16_LOW_SURROGATE_MAX) {
        return false;
      }
    } else {
      // On UTF-32 platforms a wchar_t is a full codepoint. However, values in
      // the UTF-16 surrogate ranges (U+D800..U+DFFF) are not valid Unicode
      // scalar values and must be rejected per the tests and Unicode rules.
      if (codePoint >= utf::UTF16_HIGH_SURROGATE_MIN && codePoint <= utf::UTF16_LOW_SURROGATE_MAX) {
        return false;
      }
    }

    pushUtf8Bytes(temp_out, codePoint);
    ++i;
  }

  *out = std::move(temp_out);
  return true;
}

}  // namespace util
