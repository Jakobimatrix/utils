#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <filesystem>
#include <string>
#include <utils/data/BinaryDataInterpreter.hpp>
#include <vector>
// NOLINTBEGIN (readability-magic-numbers) This test uses some random numbers, there is no value in giving them a name
// Construction from valid data array
// Ensures that the class can be constructed from a valid byte array and is ready.
TEST_CASE("BinaryDataInterpreter: Construct from valid array") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4, 5};
  util::BinaryDataInterpreter const bdi(arr.data(), 5);
  REQUIRE(bdi.isReady());
  REQUIRE(bdi.hasDataLeft(5));
}

// Construction from nullptr and zero length
// Should not crash, should not be ready.
TEST_CASE("BinaryDataInterpreter: Construct from nullptr") {
  util::BinaryDataInterpreter const bdi(nullptr, 0);
  REQUIRE_FALSE(bdi.isReady());
  REQUIRE_FALSE(bdi.hasDataLeft(1));
}

// setCursor out of bounds
// Setting cursor beyond data size should fail and not crash.
TEST_CASE("BinaryDataInterpreter: setCursor out of bounds") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  REQUIRE_FALSE(bdi.setCursor(10));
  REQUIRE(bdi.getCursor() == 0);
}

// advanceCursor beyond data
// Advancing cursor past end should fail and not crash.
TEST_CASE("BinaryDataInterpreter: advanceCursor out of bounds") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  REQUIRE_FALSE(bdi.advanceCursor(10));
  REQUIRE(bdi.getCursor() == 0);
}

// nextBytesEqual with insufficient data
// Should return false, not crash.
TEST_CASE("BinaryDataInterpreter: nextBytesEqual insufficient data") {
  const std::array<uint8_t, 5> arr = {1, 2};
  util::BinaryDataInterpreter const bdi(arr.data(), 2);
  std::vector<uint8_t> const cmp = {1, 2, 3};
  REQUIRE_FALSE(bdi.nextBytesEqual(cmp));
}

// findNextBytesAndAdvance with empty search
// Should return false, not crash.
TEST_CASE("BinaryDataInterpreter: findNextBytesAndAdvance empty search") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  std::vector<uint8_t> const empty;
  REQUIRE_FALSE(bdi.findNextBytesAndAdvance(empty, true));
}

// readNext string with nullptr
// Should return false, not crash.
TEST_CASE("BinaryDataInterpreter: readNext string nullptr") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  REQUIRE_FALSE(bdi.readNext<int>(nullptr));
  REQUIRE_FALSE(bdi.readNext<double>(nullptr));
  std::string* nullptr_string = nullptr;
  REQUIRE_FALSE(bdi.readNext(nullptr_string, 2));
  std::wstring* nullptr_wstring = nullptr;
  REQUIRE_FALSE(bdi.readNext(nullptr_wstring, 2));
}

// readNext string with length exceeding data
// Should return false, not crash.
TEST_CASE("BinaryDataInterpreter: readNext string too long") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  std::string out;
  REQUIRE_FALSE(bdi.readNext(&out, 10));
}

// readNext trivially copyable type with insufficient data
// Should return false, not crash.
TEST_CASE(
  "BinaryDataInterpreter: readNext trivially copyable insufficient data") {
  const std::array<uint8_t, 5> arr = {1, 2};
  util::BinaryDataInterpreter bdi(arr.data(), 2);
  uint32_t val = 0;
  REQUIRE_FALSE(bdi.readNext(&val));
}

// Construction from non-existent file
// Should not throw, but should not be ready.
TEST_CASE("BinaryDataInterpreter: Construct from non-existent file") {
  util::BinaryDataInterpreter const bdi(
    std::filesystem::path("this_file_does_not_exist.bin"));
  REQUIRE_FALSE(bdi.isReady());
}

// Positive test for setCursorToEnd and setCursorToStart
TEST_CASE("BinaryDataInterpreter: setCursorToEnd and setCursorToStart") {
  const std::array<uint8_t, 5> arr = {10, 20, 30, 40};
  util::BinaryDataInterpreter bdi(arr.data(), 4);
  bdi.setCursorToEnd();
  REQUIRE(bdi.getCursor() == 4);
  bdi.setCursorToStart();
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for hasDataLeft
TEST_CASE("BinaryDataInterpreter: hasDataLeft positive") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4};
  util::BinaryDataInterpreter const bdi(arr.data(), 4);
  REQUIRE(bdi.hasDataLeft(2));
}

// Negative test for hasDataLeft
TEST_CASE("BinaryDataInterpreter: hasDataLeft negative") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  bdi.setCursorToEnd();
  REQUIRE_FALSE(bdi.hasDataLeft(1));
}

// Positive test for nextBytesEqual
TEST_CASE("BinaryDataInterpreter: nextBytesEqual positive") {
  const std::array<uint8_t, 5> arr = {5, 6, 7, 8};
  util::BinaryDataInterpreter const bdi(arr.data(), 4);
  std::vector<uint8_t> const cmp = {5, 6};
  REQUIRE(bdi.nextBytesEqual(cmp));
}

// Negative test for nextBytesEqual
TEST_CASE("BinaryDataInterpreter: nextBytesEqual negative") {
  const std::array<uint8_t, 5> arr = {5, 6, 7, 8};
  util::BinaryDataInterpreter const bdi(arr.data(), 4);
  std::vector<uint8_t> const cmp = {6, 7};
  REQUIRE_FALSE(bdi.nextBytesEqual(cmp));
}

// Positive test for advanceCursor
TEST_CASE("BinaryDataInterpreter: advanceCursor positive") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4};
  util::BinaryDataInterpreter bdi(arr.data(), 4);
  REQUIRE(bdi.advanceCursor(2));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for advanceCursor
TEST_CASE("BinaryDataInterpreter: advanceCursor negative") {
  const std::array<uint8_t, 5> arr = {1, 2};
  util::BinaryDataInterpreter bdi(arr.data(), 2);
  REQUIRE_FALSE(bdi.advanceCursor(3));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for advanceCursorIfEqual
TEST_CASE("BinaryDataInterpreter: advanceCursorIfEqual positive") {
  const std::array<uint8_t, 5> arr = {9, 8, 7};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  std::vector<uint8_t> const cmp = {9, 8};
  REQUIRE(bdi.advanceCursorIfEqual(cmp));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for advanceCursorIfEqual
TEST_CASE("BinaryDataInterpreter: advanceCursorIfEqual negative") {
  const std::array<uint8_t, 5> arr = {9, 8, 7};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  std::vector<uint8_t> const cmp = {8, 7};
  REQUIRE_FALSE(bdi.advanceCursorIfEqual(cmp));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for findNextBytesAndAdvance
TEST_CASE("BinaryDataInterpreter: findNextBytesAndAdvance positive") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 6);
  std::vector<uint8_t> const cmp = {2, 3};
  REQUIRE(bdi.findNextBytesAndAdvance(cmp, false));
  REQUIRE(bdi.getCursor() == 1);
}

// Positive test for findNextBytesAndAdvance beyond
TEST_CASE("BinaryDataInterpreter: findNextBytesAndAdvance beyond positive") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 6);
  std::vector<uint8_t> const cmp = {2, 3};
  REQUIRE(bdi.findNextBytesAndAdvance(cmp, true));
  REQUIRE(bdi.getCursor() == 3);
}

// Negative test for findNextBytesAndAdvance
TEST_CASE("BinaryDataInterpreter: findNextBytesAndAdvance negative") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4};
  util::BinaryDataInterpreter bdi(arr.data(), 4);
  std::vector<uint8_t> const cmp = {5, 6};
  REQUIRE_FALSE(bdi.findNextBytesAndAdvance(cmp, true));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for setCursor
TEST_CASE("BinaryDataInterpreter: setCursor positive") {
  const std::array<uint8_t, 5> arr = {1, 2, 3, 4};
  util::BinaryDataInterpreter bdi(arr.data(), 4);
  REQUIRE(bdi.setCursor(2));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for setCursor
TEST_CASE("BinaryDataInterpreter: setCursor negative") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  REQUIRE_FALSE(bdi.setCursor(5));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for readNext (trivially copyable)
TEST_CASE("BinaryDataInterpreter: readNext trivially copyable positive") {
  const std::array<uint8_t, 5> arr = {0x12, 0x34, 0x56, 0x78};
  util::BinaryDataInterpreter bdi(arr.data(), 4);
  uint32_t val = 0;
  REQUIRE(bdi.readNext(&val));
  // Platform endianness may affect value, but at least cursor should advance
  REQUIRE(bdi.getCursor() == 4);
}

// Positive test for readNext (string)
TEST_CASE("BinaryDataInterpreter: readNext string positive") {
  const char* text = "abcd";
  util::BinaryDataInterpreter bdi(reinterpret_cast<const uint8_t*>(text), 4);
  std::string out;
  REQUIRE(bdi.readNext(&out, 4));
  REQUIRE(out == "abcd");
  REQUIRE(bdi.getCursor() == 4);
}

// Negative test for readNext (string)
TEST_CASE("BinaryDataInterpreter: readNext string negative") {
  const std::array<uint8_t, 5> arr = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr.data(), 3);
  std::string out;
  REQUIRE_FALSE(bdi.readNext(&out, 5));
  REQUIRE(bdi.getCursor() == 0);
}
// NOLINTEND (readability-magic-numbers)
