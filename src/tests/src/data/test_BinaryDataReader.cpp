/**
 * @file test_BinaryDataReader.cpp
 * @brief contains tests for the BinaryDataReader class
 *
 * @date 2025
 * @author Jakob Wandel
 **/


#include <catch2/catch_test_macros.hpp>
#include <utils/data/BinaryDataReader.hpp>
#include <utils/data/BinaryDataWriter.hpp>

#include <array>
#include <vector>
#include <map>
#include <cstdint>
#include <filesystem>
#include <string>

// NOLINTBEGIN (readability-magic-numbers) This test uses some random numbers, there is no value in giving them a name
// Construction from valid data array
// Ensures that the class can be constructed from a valid byte array and is ready.
TEST_CASE("BinaryDataReader: Construct from valid array") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4, 5};
  serialize::BinaryDataReader const bdi(arr.data(), 5, true, std::endian::little);
  REQUIRE(bdi.isReady());
  REQUIRE(bdi.hasDataLeft(5));
}

// Construction from nullptr and zero length
// Should not crash, should not be ready.
TEST_CASE("BinaryDataReader: Construct from nullptr") {
  serialize::BinaryDataReader const bdi(nullptr, 0, true, std::endian::little);
  REQUIRE_FALSE(bdi.isReady());
  REQUIRE_FALSE(bdi.hasDataLeft(1));
}

// setCursor out of bounds
// Setting cursor beyond data size should fail and not crash.
TEST_CASE("BinaryDataReader: setCursor out of bounds") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  REQUIRE_FALSE(bdi.setCursor(10));
  REQUIRE(bdi.getCursor() == 0);
}

// advanceCursor beyond data
// Advancing cursor past end should fail and not crash.
TEST_CASE("BinaryDataReader: advanceCursor out of bounds") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  REQUIRE_FALSE(bdi.advanceCursor(10));
  REQUIRE(bdi.getCursor() == 0);
}

// nextBytesEqual with insufficient data
// Should return false, not crash.
TEST_CASE("BinaryDataReader: nextBytesEqual insufficient data") {
  const std::array<uint8_t, 5> arr = {1, 2};
  serialize::BinaryDataReader const bdi(arr.data(), 2, true, std::endian::little);
  std::vector<uint8_t> const cmp = {1, 2, 3};
  REQUIRE_FALSE(bdi.nextBytesEqual(cmp));
}

// findNextBytesAndAdvance with empty search
// Should return false, not crash.
TEST_CASE("BinaryDataReader: findNextBytesAndAdvance empty search") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  std::vector<uint8_t> const empty;
  REQUIRE_FALSE(bdi.findNextBytesAndAdvance(empty, true));
}

// readNext string with nullptr
// Should return false, not crash.
TEST_CASE("BinaryDataReader: readNext string nullptr") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  REQUIRE_FALSE(bdi.readNext(static_cast<std::string*>(nullptr)));
  std::string* nullptr_string = nullptr;
  REQUIRE_FALSE(bdi.readNext(nullptr_string));
}

// readNext string with length exceeding data
// Should return false, not crash.
TEST_CASE("BinaryDataReader: readNext string too long") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  std::string out;
  REQUIRE_FALSE(bdi.readNext(&out));
}

// readNext trivially copyable type with insufficient data
// Should return false, not crash.
TEST_CASE("BinaryDataReader: readNext trivially copyable insufficient data") {
  const std::array<uint8_t, 5> arr = {1, 2};
  serialize::BinaryDataReader bdi(arr.data(), 2, true, std::endian::little);
  uint32_t val = 0;
  REQUIRE_FALSE(bdi.readNext(&val));
}

// Construction from non-existent file
// Should not throw, but should not be ready.
TEST_CASE("BinaryDataReader: Construct from non-existent file") {
  serialize::BinaryDataReader const bdi(
    std::filesystem::path("this_file_does_not_exist.bin"), std::endian::little);
  REQUIRE_FALSE(bdi.isReady());
}

// Positive test for setCursorToEnd and setCursorToStart
TEST_CASE("BinaryDataReader: setCursorToEnd and setCursorToStart") {
  const std::array<uint8_t, 5> arr = {10, 20, 30, 40};
  serialize::BinaryDataReader bdi(arr.data(), 4, true, std::endian::little);
  bdi.setCursorToEnd();
  REQUIRE(bdi.getCursor() == 4);
  bdi.setCursorToStart();
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for hasDataLeft
TEST_CASE("BinaryDataReader: hasDataLeft positive") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4};
  serialize::BinaryDataReader const bdi(arr.data(), 4, true, std::endian::little);
  REQUIRE(bdi.hasDataLeft(2));
}

// Negative test for hasDataLeft
TEST_CASE("BinaryDataReader: hasDataLeft negative") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  bdi.setCursorToEnd();
  REQUIRE_FALSE(bdi.hasDataLeft(1));
}

// Positive test for nextBytesEqual
TEST_CASE("BinaryDataReader: nextBytesEqual positive") {
  const std::array<uint8_t, 5> arr = {5, 6, 7, 8};
  serialize::BinaryDataReader const bdi(arr.data(), 4, true, std::endian::little);
  std::vector<uint8_t> const cmp = {5, 6};
  REQUIRE(bdi.nextBytesEqual(cmp));
}

// Negative test for nextBytesEqual
TEST_CASE("BinaryDataReader: nextBytesEqual negative") {
  const std::array<uint8_t, 5> arr = {5, 6, 7, 8};
  serialize::BinaryDataReader const bdi(arr.data(), 4, true, std::endian::little);
  std::vector<uint8_t> const cmp = {6, 7};
  REQUIRE_FALSE(bdi.nextBytesEqual(cmp));
}

// Positive test for advanceCursor
TEST_CASE("BinaryDataReader: advanceCursor positive") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4};
  serialize::BinaryDataReader bdi(arr.data(), 4, true, std::endian::little);
  REQUIRE(bdi.advanceCursor(2));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for advanceCursor
TEST_CASE("BinaryDataReader: advanceCursor negative") {
  const std::array<uint8_t, 5> arr = {1, 2};
  serialize::BinaryDataReader bdi(arr.data(), 2, true, std::endian::little);
  REQUIRE_FALSE(bdi.advanceCursor(3));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for advanceCursorIfEqual
TEST_CASE("BinaryDataReader: advanceCursorIfEqual positive") {
  const std::array<uint8_t, 5> arr = {9, 8, 7};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  std::vector<uint8_t> const cmp = {9, 8};
  REQUIRE(bdi.advanceCursorIfEqual(cmp));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for advanceCursorIfEqual
TEST_CASE("BinaryDataReader: advanceCursorIfEqual negative") {
  const std::array<uint8_t, 5> arr = {9, 8, 7};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  std::vector<uint8_t> const cmp = {8, 7};
  REQUIRE_FALSE(bdi.advanceCursorIfEqual(cmp));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for findNextBytesAndAdvance
TEST_CASE("BinaryDataReader: findNextBytesAndAdvance positive") {
  const std::array<uint8_t, 6> arr = {1, 2, 3, 4, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 6, true, std::endian::little);
  std::vector<uint8_t> const cmp = {2, 3};
  REQUIRE(bdi.findNextBytesAndAdvance(cmp, false));
  REQUIRE(bdi.getCursor() == 1);
}

// Positive test for findNextBytesAndAdvance beyond
TEST_CASE("BinaryDataReader: findNextBytesAndAdvance beyond positive") {
  const std::array<uint8_t, 6> arr = {1, 2, 3, 4, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 6, true, std::endian::little);
  std::vector<uint8_t> const cmp = {2, 3};
  REQUIRE(bdi.findNextBytesAndAdvance(cmp, true));
  REQUIRE(bdi.getCursor() == 3);
}

// Negative test for findNextBytesAndAdvance
TEST_CASE("BinaryDataReader: findNextBytesAndAdvance negative") {
  const std::array<uint8_t, 4> arr = {1, 2, 3, 4};
  serialize::BinaryDataReader bdi(arr.data(), 4, true, std::endian::little);
  std::vector<uint8_t> const cmp = {5, 6};
  REQUIRE_FALSE(bdi.findNextBytesAndAdvance(cmp, true));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for setCursor
TEST_CASE("BinaryDataReader: setCursor positive") {
  const std::array<uint8_t, 4> arr = {1, 2, 3, 4};
  serialize::BinaryDataReader bdi(arr.data(), 4, true, std::endian::little);
  REQUIRE(bdi.setCursor(2));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for setCursor
TEST_CASE("BinaryDataReader: setCursor negative") {
  const std::array<uint8_t, 3> arr = {1, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  REQUIRE_FALSE(bdi.setCursor(5));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for readNext (trivially copyable)
TEST_CASE("BinaryDataReader: readNext trivially copyable positive") {
  const std::array<uint8_t, 4> arr = {0x12, 0x34, 0x56, 0x78};
  serialize::BinaryDataReader bdi(arr.data(), 4, true, std::endian::little);
  uint32_t val = 0;
  REQUIRE(bdi.readNext(&val));
  // Platform endianness may affect value, but at least cursor should advance
  REQUIRE(bdi.getCursor() == 4);
}

// Positive test for readNext (string)
TEST_CASE("BinaryDataReader: readNext string positive") {
  // Data layout: 8-byte little-endian length (uint64_t = 4) then 4 bytes of string
  const std::array<uint8_t, 12> data = {
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'a', 'b', 'c', 'd'};
  serialize::BinaryDataReader bdi(data.data(), data.size(), true, std::endian::little);
  std::string out;
  REQUIRE(bdi.readNext(&out));
  REQUIRE(out == "abcd");
  REQUIRE(bdi.getCursor() == data.size());
}

// Negative test for readNext (string)
TEST_CASE("BinaryDataReader: readNext string negative") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  serialize::BinaryDataReader bdi(arr.data(), 3, true, std::endian::little);
  std::string out;
  REQUIRE_FALSE(bdi.readNext(&out));
  REQUIRE(bdi.getCursor() == 0);
}



TEST_CASE("BinaryDataReader: getReadBegin and getData and releaseBuffer") {
  const std::array<uint8_t, 4> arr = {1, 2, 3, 4};
  serialize::BinaryDataReader bdi(arr.data(), arr.size(), true, std::endian::little);
  REQUIRE(bdi.getData().size() == 4);
  auto it = bdi.getReadBegin();
  REQUIRE(*it == 1);
  auto buf = bdi.releaseBuffer();
  REQUIRE(buf.size() == 4);
}

TEST_CASE("BinaryDataReader: setCursorToStartEnd and setCursor bounds") {
  const std::array<uint8_t, 4> arr = {10, 20, 30, 40};
  serialize::BinaryDataReader bdi(arr.data(), arr.size(), true, std::endian::little);
  bdi.setCursorToEnd();
  REQUIRE(bdi.getCursor() == 4);
  bdi.setCursorToStart();
  REQUIRE(bdi.getCursor() == 0);
  REQUIRE_FALSE(bdi.setCursor(10));
  REQUIRE(bdi.getCursor() == 0);
}

TEST_CASE("BinaryDataReader: readNext scalar success and failure") {
  // provide a uint32 (4 bytes)
  const std::array<uint8_t, 4> arr = {0x01, 0x00, 0x00, 0x00};
  serialize::BinaryDataReader bdi(arr.data(), arr.size(), true, std::endian::little);
  uint32_t v = 0;
  REQUIRE(bdi.readNext(&v));
  REQUIRE(v == 1);
  // insufficient data: new reader with only 2 bytes for a uint32
  const std::array<uint8_t, 2> arr2 = {0x01, 0x00};
  serialize::BinaryDataReader bdi2(arr2.data(), arr2.size(), true, std::endian::little);
  uint32_t w = 0;
  REQUIRE_FALSE(bdi2.readNext(&w));
}

TEST_CASE(
  "BinaryDataReader: readNext std::string with size prefix success and "
  "failure") {
  // encode size (uint64 little-endian) = 3, then "abc"
  const std::array<uint8_t, 11> data = {0x03, 0, 0, 0, 0, 0, 0, 0, 'a', 'b', 'c'};
  serialize::BinaryDataReader bdi(data.data(), data.size(), true, std::endian::little);
  std::string out;
  REQUIRE(bdi.readNext(&out));
  REQUIRE(out == "abc");
  // truncated: claim length 5 but only 2 bytes follow
  const std::array<uint8_t, 10> data2 = {0x05, 0, 0, 0, 0, 0, 0, 0, 'x', 'y'};
  serialize::BinaryDataReader bdi3(data2.data(), data2.size(), true, std::endian::little);
  std::string out2;
  REQUIRE_FALSE(bdi3.readNext(&out2));
}

TEST_CASE(
  "BinaryDataReader: readNext optional/variant/pair/tuple/vector/map/set "
  "success and failure") {
  // We'll construct writers to produce correct encoded data then read them
  serialize::BinaryDataWriter writer(32, 1024, std::endian::little);
  // write optional<int> with value
  std::optional<int> oi = 42;
  REQUIRE(writer.writeNext(oi));
  // write optional<int> without value
  std::optional<int> none = std::nullopt;
  REQUIRE(writer.writeNext(none));
  // write variant<int,char>
  std::variant<int, char> var = 7;
  REQUIRE(writer.writeNext(var));
  // write pair<int,int>
  std::pair<int, int> pr{1, 2};
  REQUIRE(writer.writeNext(pr));
  // write tuple<int,char>
  // Avoid writing raw std::tuple (no serialize), use pair instead
  std::pair<int, char> tup{3, 'z'};
  REQUIRE(writer.writeNext(tup));
  // write vector<int>
  std::vector<int> vec{1, 2};
  REQUIRE(writer.writeNext(vec));
  // write map<int,int>
  std::map<int, int> mp;
  mp.emplace(1, 10);
  REQUIRE(writer.writeNext(mp));
  REQUIRE(writer.setWritingFinished(true));

  auto buffer = std::move(writer.releaseBuffer());
  serialize::BinaryDataReader reader(std::move(buffer), std::endian::little);

  std::optional<int> r_oi;
  REQUIRE(reader.readNext(&r_oi));
  REQUIRE(r_oi.has_value());
  REQUIRE(*r_oi == 42);
  std::optional<int> r_none;
  REQUIRE(reader.readNext(&r_none));
  REQUIRE(!r_none.has_value());
  std::variant<int, char> rvar;
  REQUIRE(reader.readNext(&rvar));
  REQUIRE(std::get<int>(rvar) == 7);
  std::pair<int, int> rpr;
  REQUIRE(reader.readNext(&rpr));
  REQUIRE(rpr.first == 1);
  REQUIRE(rpr.second == 2);
  std::pair<int, char> rtup;
  REQUIRE(reader.readNext(&rtup));
  REQUIRE(rtup.first == 3);
  REQUIRE(rtup.second == 'z');
  std::vector<int> rvec;
  REQUIRE(reader.readNext(&rvec));
  REQUIRE(rvec.size() == 2);
  REQUIRE(rvec[0] == 1);
  std::map<int, int> rmap;
  REQUIRE(reader.readNext(&rmap));
  REQUIRE(rmap.size() == 1);
  REQUIRE(rmap.begin()->second == 10);

  // Now failure paths: try to read with missing data (create small buffer)
  const std::array<uint8_t, 0> bad = {};
  serialize::BinaryDataReader br(bad.data(), bad.size(), true, std::endian::little);
  std::optional<int> tmp;
  REQUIRE_FALSE(br.readNext(&tmp));
}
// NOLINTEND (readability-magic-numbers)
