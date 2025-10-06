/**
 * @file test_utf8Conversion.cpp
 * @brief contains tests for the wstring to string utf8 converter
 *
 * @details
 * Tests round-trip identity for valid Unicode ranges and
 * rejection of invalid UTF-8 sequences (surrogates, overlong, truncated, etc.).
 *
 * Test against:
 * Encoding confusion:
 * Treating bytes as UTF‑8 vs UTF‑16 inconsistently can bypass filters or smuggle unsafe characters.
 * Overlong/forbidden sequences:
 * Non-canonical UTF‑8 can hide ASCII bytes (like  / or \0) to bypass checks.
 * Surrogate codepoints in UTF‑8:
 * U+D800..U+DFFF are invalid; accepting them causes corruption or errors.
 * Truncated/malformed sequences:
 * Missing or invalid continuation bytes can crash or mislead string processing.
 * Signed/unsigned miscasts:
 * High-bit bytes misinterpreted as negative can bypass validation.
 * Lone UTF‑16 surrogates:
 * Single high/low surrogates in wstring must be rejected to prevent invalid UTF‑8 output.
 *
 * @date 29.05.2025
 * @author Jakob Wandel
 **/


#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <utils/string/utf8Conversion.hpp>

#include <string>
#include <vector>
#include <iostream>

// NOLINTBEGIN (readability-magic-numbers) This test uses some random numbers, there is no value in giving them a name
static void test_range(uint32_t start, uint32_t end, bool expect_success) {
  std::wstring wstr;
  std::string utf8;

  for (uint32_t cp = start; cp <= end; ++cp) {
    if (cp >= util::utf::UTF16_HIGH_SURROGATE_MIN && cp <= util::utf::UTF16_LOW_SURROGATE_MAX)
      continue;  // skip surrogate area for valid cases

    wstr.clear();
    utf8.clear();

    // Simulate wchar_t representation depending on platform width
    if constexpr (std::numeric_limits<wchar_t>::max() == util::utf::UTF8_THREE_BYTE_MAX) {  // UTF-16
      if (cp <= util::utf::UTF8_THREE_BYTE_MAX) {
        wstr.push_back(static_cast<wchar_t>(cp));
      } else {
        const uint32_t offset = cp - util::utf::UNICODE_PLANE1_START;
        wstr.push_back(static_cast<wchar_t>(util::utf::UTF16_HIGH_SURROGATE_MIN +
                                            ((offset >> util::utf::UTF16_SURROGATE_SHIFT) &
                                             util::utf::UTF16_SURROGATE_MASK)));
        wstr.push_back(static_cast<wchar_t>(util::utf::UTF16_LOW_SURROGATE_MIN +
                                            (offset & util::utf::UTF16_SURROGATE_MASK)));
      }
    } else {  // UTF-32
      wstr.push_back(static_cast<wchar_t>(cp));
    }

    INFO("Codepoint: 0x" << std::hex << cp);

    bool enc_ok = util::wstring_to_utf8(wstr, utf8);
    if (!expect_success) {
      REQUIRE_FALSE(enc_ok);
      continue;
    }
    REQUIRE(enc_ok);

    std::wstring decoded;
    bool dec_ok = util::utf8_to_wstring(utf8, decoded);
    REQUIRE(dec_ok);
    REQUIRE(decoded == wstr);
  }
}


TEST_CASE("UTF-8 ↔ std::wstring identity — valid ranges") {
  std::cerr << "Testing valid codepoint ranges..." << std::endl;

  // ASCII
  test_range(0x00, 0x7F, true);
  // 2-byte UTF-8
  test_range(0x80, 0x7FF, true);
  // 3-byte UTF-8 excluding surrogates
  test_range(0x800, 0xD7FF, true);
  test_range(0xE000, 0xFFFF, true);
  // 4-byte UTF-8 (plane 1+)
  test_range(0x10000, 0x10FFFF, true);
}

TEST_CASE("Invalid UTF-8 sequences are rejected") {
  std::cerr << "Testing invalid sequences..." << std::endl;

  std::wstring decoded;

  // Lone continuation bytes (should all fail)
  for (unsigned char c = 0x80; c <= 0xBF; ++c) {
    std::string s(1, static_cast<char>(c));
    REQUIRE_FALSE(util::utf8_to_wstring(s, decoded));
  }

  // Overlong encodings (3× each type)
  const std::string overlong_examples[] = {
    "\xC0\xAF",          // overlong '/'
    "\xC1\xBF",          // overlong '?'
    "\xE0\x80\xAF",      // overlong '/'
    "\xF0\x80\x80\xAF",  // overlong '/'
    "\xF0\x82\x82\xAC",  // overlong €
    "\xC0\x81"           // overlong 'A'
  };
  for (auto& s : overlong_examples) {
    REQUIRE_FALSE(util::utf8_to_wstring(s, decoded));
  }

  // Truncated sequences (missing bytes)
  const std::string truncated_examples[] = {
    "\xC2",         // missing 2nd byte
    "\xE2\x82",     // missing 3rd byte
    "\xF0\x9F\x98"  // missing 4th byte
  };
  for (auto& s : truncated_examples) {
    REQUIRE_FALSE(util::utf8_to_wstring(s, decoded));
  }

  // Invalid continuation placement (continuation after ASCII)
  const std::vector<std::string> invalid_continuation = {
    std::string({'A', static_cast<char>(0x80), 'B'}),
    std::string({'A', static_cast<char>(0xBF), 'Z'}),
    std::string({static_cast<char>(0x7F), static_cast<char>(0x80)}),
    std::string({'A', static_cast<char>(0x80)})};
  for (auto& s : invalid_continuation) {
    REQUIRE_FALSE(util::utf8_to_wstring(s, decoded));
  }

  // Surrogate halves (illegal range U+D800–U+DFFF)
  const std::string surrogate_examples[] = {
    "\xED\xA0\x80",  // D800
    "\xED\xAF\xBF",  // DBFF
    "\xED\xB0\x80",  // DC00
    "\xED\xBF\xBF"   // DFFF
  };
  for (auto& s : surrogate_examples) {
    REQUIRE_FALSE(util::utf8_to_wstring(s, decoded));
  }

  // Codepoints beyond Unicode range (>= 0x110000)
  const std::string too_large_examples[] = {
    "\xF4\x90\x80\x80",  // 0x110000
    "\xF5\x80\x80\x80",  // 0x140000
    "\xF8\x88\x80\x80"   // 0x220000 (illegal 5-byte start)
  };
  for (auto& s : too_large_examples) {
    REQUIRE_FALSE(util::utf8_to_wstring(s, decoded));
  }

  // Invalid start bytes (0xF5–0xFF are never valid)
  for (uint32_t c = 0xF5; c <= 0xFF; ++c) {
    std::string s(1, static_cast<char>(c));
    REQUIRE_FALSE(util::utf8_to_wstring(s, decoded));
  }

  // Mixed valid + invalid (must fail)
  const std::string mixed_invalid[] = {
    "Hello\x80World", "Valid\xC2\xA2\xE2\x82Trunc", "🙂\xED\xA0\x80"};
  for (auto& s : mixed_invalid) {
    REQUIRE_FALSE(util::utf8_to_wstring(s, decoded));
  }
}

TEST_CASE(
  "Cross-platform UTF-16 / UTF-32 wstring simulation yields expected UTF-8") {
  using namespace util;

  // Example: codepoints (U+1F600) and (U+20AC)
  // Hardcoded expected UTF-8 output
  const std::string expected_utf8 = "\xF0\x9F\x98\x80\xE2\x82\xAC";

#if WCHAR_MAX == 0xFFFF
  // Simulate reading UTF-16 binary file (Windows)
  const std::array<uint16_t, 2> utf16_data = {
    0xD83D,  // high surrogate of
    0xDE00   // low surrogate of
  };
  const std::array<uint16_t, 1> utf16_euro = {0x20AC};  //

  std::wstring wstr;
  for (uint16_t cu : utf16_data) wstr.push_back(static_cast<wchar_t>(cu));
  for (uint16_t cu : utf16_euro) wstr.push_back(static_cast<wchar_t>(cu));

#else
  // Simulate reading UTF-32 binary file (Linux)
  const std::array<uint32_t, 2> utf32_data = {0x1F600, 0x20AC};
  std::wstring wstr;
  for (uint32_t cp : utf32_data) wstr.push_back(static_cast<wchar_t>(cp));
#endif

  std::string out_utf8;
  REQUIRE(wstring_to_utf8(wstr, out_utf8));

  INFO("UTF-8 output: " << out_utf8);
  REQUIRE(out_utf8 == expected_utf8);

  // Now simulate the reverse: read UTF-8 file and decode to wstring
  std::wstring decoded_wstr;
  REQUIRE(utf8_to_wstring(expected_utf8, decoded_wstr));

#if WCHAR_MAX == 0xFFFF
  // On Windows, re-encode to UTF-8 should yield same expected UTF-8
  std::string utf8_again;
  REQUIRE(wstring_to_utf8(decoded_wstr, utf8_again));
  REQUIRE(utf8_again == expected_utf8);
#else
  // On Linux, the wstring is UTF-32, still yields same UTF-8
  std::string utf8_again;
  REQUIRE(wstring_to_utf8(decoded_wstr, utf8_again));
  REQUIRE(utf8_again == expected_utf8);
#endif
}


TEST_CASE(
  "Comprehensive UTF-8 -> wstring conversion with full edge-case coverage") {
  using namespace util;

  // --------------------------
  // Universal UTF-8 table (all sequences, platform-independent)
  // --------------------------
  const std::vector<std::string> utf8_table = {
    // 1-byte ASCII
    "\x00",
    "\x01",
    "\x20",
    "\x7E",
    "\x7F",

    // 2-byte sequences (U+0080..U+07FF)
    "\xC2\x80",
    "\xC2\xA2",
    "\xDF\xBF",

    // 3-byte sequences excluding surrogates (U+0800..U+D7FF)
    "\xE0\xA0\x80",
    "\xE0\xB0\x80",
    "\xED\x9F\xBF",

    // 3-byte sequences after surrogates (U+E000..U+FFFF)
    "\xEE\x80\x80",
    "\xEF\xBF\xBF",

    // 4-byte sequences (plane 1 / emojis)
    "\xF0\x90\x80\x80",  // U+10000
    "\xF0\x9F\x98\x80",  // U+1F600
    "\xF0\x9F\x98\x82",  // U+1F602
    "\xF4\x8F\xBF\xBF"   // U+10FFFF
  };

  // --------------------------
  // Expected wstring table (platform-dependent)
  // --------------------------
#if WCHAR_MAX == 0xFFFF
  const std::vector<std::wstring> expected_wstr_table = {
    // 1-byte ASCII
    L"\x00",
    L"\x01",
    L"\x20",
    L"\x7E",
    L"\x7F",

    // 2-byte sequences
    L"\x0080",
    L"\x00A2",
    L"\x07FF",

    // 3-byte sequences excluding surrogates
    L"\x0800",
    L"\x0C00",
    L"\xD7FF",

    // 3-byte sequences after surrogates
    L"\xE000",
    L"\xFFFF",

    // 4-byte sequences (surrogate pairs)
    L"\xD800\xDC00",  // U+10000
    L"\xD83D\xDE00",  // U+1F600
    L"\xD83D\xDE02",  // U+1F602
    L"\xDBFF\xDFFF"   // U+10FFFF
  };
#else
  const std::vector<std::wstring> expected_wstr_table = {
    // 1-byte ASCII
    L"\x00",
    L"\x01",
    L"\x20",
    L"\x7E",
    L"\x7F",

    // 2-byte sequences
    L"\x0080",
    L"\x00A2",
    L"\x07FF",

    // 3-byte sequences excluding surrogates
    L"\x0800",
    L"\x0C00",
    L"\xD7FF",

    // 3-byte sequences after surrogates
    L"\xE000",
    L"\xFFFF",

    // 4-byte sequences (UTF-32)
    L"\x10000",
    L"\x1F600",
    L"\x1F602",
    L"\x10FFFF"  // max codepoint
  };
#endif

  REQUIRE(utf8_table.size() == expected_wstr_table.size());

  for (size_t i = 0; i < utf8_table.size(); ++i) {
    const auto& utf8_bytes    = utf8_table[i];
    const auto& expected_wstr = expected_wstr_table[i];

    std::wstring out_wstr;
    INFO("UTF-8 bytes: ");
    INFO("\nExpected wstring size: " << expected_wstr.size());
    REQUIRE(utf8_to_wstring(utf8_bytes, out_wstr));
    REQUIRE(out_wstr == expected_wstr);
  }
}


TEST_CASE("utf8_to_wstring: rejects overlong encodings and malformed bytes") {
  using namespace util;
  std::wstring out;

  // Overlong encodings (same codepoint encoded with too many bytes)
  const std::vector<std::string> overlong = {
    // Overlong NUL forms
    "\xC0\x80",         // 2-byte overlong for U+0000
    "\xE0\x80\x80",     // 3-byte overlong for U+0000
    "\xF0\x80\x80\x80"  // 4-byte overlong for U+0000
  };
  for (auto const& s : overlong) {
    // should not throw (functions are noexcept), must return false and leave out empty
    REQUIRE_FALSE(utf8_to_wstring(s, out));
    REQUIRE(out.empty());
  }

  // Surrogate codepoints encoded in UTF-8 (illegal)
  const std::vector<std::string> surrogate_utf8 = {
    "\xED\xA0\x80",  // U+D800 (high surrogate)
    "\xED\xBF\xBF"   // U+DFFF (low surrogate)
  };
  for (auto const& s : surrogate_utf8) {
    REQUIRE_FALSE(utf8_to_wstring(s, out));
    REQUIRE(out.empty());
  }

  // Unexpected continuation bytes at start / embedded
  const std::vector<std::string> lone_cont = {
    "\x80",                  // single continuation
    "\xBF",                  // single continuation (upper)
    {'A', char(0x80), 'B'},  // ASCII with continuation in middle
    "\x7F\x80"               // ASCII boundary then continuation
  };
  for (auto const& s : lone_cont) {
    REQUIRE_FALSE(utf8_to_wstring(s, out));
    REQUIRE(out.empty());
  }

  // Truncated multi-byte sequences
  const std::vector<std::string> truncated = {
    "\xC2",         // 2-byte start but missing continuation
    "\xE2\x82",     // 3-byte truncated
    "\xF0\x9F\x98"  // 4-byte truncated (emoji)
  };
  for (auto const& s : truncated) {
    REQUIRE_FALSE(utf8_to_wstring(s, out));
    REQUIRE(out.empty());
  }

  // Invalid start bytes (0xF5..0xFF) never valid in UTF-8
  for (uint32_t b = 0xF5; b <= 0xFF; ++b) {
    std::string s(1, static_cast<char>(b));
    REQUIRE_FALSE(utf8_to_wstring(s, out));
    REQUIRE(out.empty());
  }

  // Codepoints beyond Unicode max (>= U+110000)
  const std::vector<std::string> out_of_range = {
    "\xF4\x90\x80\x80",  // U+110000
    "\xF7\xBF\xBF\xBF"   // illegal 5-byte style
  };
  for (auto const& s : out_of_range) {
    REQUIRE_FALSE(utf8_to_wstring(s, out));
    REQUIRE(out.empty());
  }

  // Embedded NUL inside valid bytes: valid; function should succeed and preserve NUL in wstring
  // we still test it explicitly because attackers sometimes use embedded NULs to truncate strings downstream
  std::wstring out2;
  const std::string with_null = std::string({'A', '\0', 'B'});  // bytes: 0x41 0x00 0x42
  // This is valid UTF-8 (ASCII + NUL + ASCII)
  REQUIRE(utf8_to_wstring(with_null, out2));
  REQUIRE(out2.size() == 3);
  REQUIRE(out2[1] == L'\0');  // NUL preserved inside std::wstring
}

TEST_CASE(
  "wstring_to_utf8: rejects malformed surrogate pairs and invalid wchars") {
  using namespace util;
  std::string out;

  // On UTF-16 platforms, a lone high surrogate must be rejected
#if WCHAR_MAX == 0xFFFF
  {
    std::wstring bad;
    bad.push_back(static_cast<wchar_t>(0xD800));  // lone high surrogate
    REQUIRE_FALSE(wstring_to_utf8(bad, out));
    REQUIRE(out.empty());
  }
  // Lone low surrogate
  {
    std::wstring bad;
    bad.push_back(static_cast<wchar_t>(0xDC00));  // lone low surrogate
    REQUIRE_FALSE(wstring_to_utf8(bad, out));
    REQUIRE(out.empty());
  }
  // Reversed pair (low then high) should be rejected
  {
    std::wstring bad;
    bad.push_back(static_cast<wchar_t>(0xDC00));
    bad.push_back(static_cast<wchar_t>(0xD800));
    REQUIRE_FALSE(wstring_to_utf8(bad, out));
    REQUIRE(out.empty());
  }
#else
  // On UTF-32 platforms, these code unit values are actual codepoints in the surrogate range;
  // they must still be rejected (they are invalid Unicode scalar values).
  {
    std::wstring bad;
    bad.push_back(static_cast<wchar_t>(0xD800));
    REQUIRE_FALSE(wstring_to_utf8(bad, out));
    REQUIRE(out.empty());
  }
  {
    std::wstring bad;
    bad.push_back(static_cast<wchar_t>(0xDC00));
    REQUIRE_FALSE(wstring_to_utf8(bad, out));
    REQUIRE(out.empty());
  }
#endif

  // Valid surrogate pair should encode
#if WCHAR_MAX == 0xFFFF
  {
    std::wstring good;
    // U+1F600 => surrogate pair D83D DE00
    good.push_back(static_cast<wchar_t>(0xD83D));
    good.push_back(static_cast<wchar_t>(0xDE00));
    REQUIRE(wstring_to_utf8(good, out));
    REQUIRE(!out.empty());
    // quick sanity: must equal known bytes
    REQUIRE(out == std::string("\xF0\x9F\x98\x80", 4));
  }
#else
  {
    std::wstring good;
    good.push_back(static_cast<wchar_t>(0x1F600));
    REQUIRE(wstring_to_utf8(good, out));
    REQUIRE(!out.empty());
    REQUIRE(out == std::string("\xF0\x9F\x98\x80", 4));
  }
#endif

  // Large but valid sequences (moderate size); ensure no exceptions / rejections
  {
    std::wstring many;
#if WCHAR_MAX == 0xFFFF
    // Repeat a safe BMP char
    for (int i = 0; i < 1000; ++i) many.push_back(L'A');
#else
    // Repeat a 4-byte codepoint on UTF-32 build
    for (int i = 0; i < 1000; ++i)
      many.push_back(static_cast<wchar_t>(0x1F600));
#endif
    REQUIRE(wstring_to_utf8(many, out));
    REQUIRE(!out.empty());
  }
}



// NOLINTEND (readability-magic-numbers)
