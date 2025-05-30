#include <catch2/catch_test_macros.hpp>
#include <utils/data/BinaryDataInterpreter.hpp>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

// Construction from valid data array
// Ensures that the class can be constructed from a valid byte array and is ready.
TEST_CASE("BinaryDataInterpreter: Construct from valid array") {
  uint8_t arr[] = {1, 2, 3, 4, 5};
  util::BinaryDataInterpreter bdi(arr, 5);
  REQUIRE(bdi.isReady());
  REQUIRE(bdi.hasDataLeft(5));
}

// Construction from nullptr and zero length
// Should not crash, should not be ready.
TEST_CASE("BinaryDataInterpreter: Construct from nullptr") {
  util::BinaryDataInterpreter bdi(nullptr, 0);
  REQUIRE_FALSE(bdi.isReady());
  REQUIRE_FALSE(bdi.hasDataLeft(1));
}

// setCursor out of bounds
// Setting cursor beyond data size should fail and not crash.
TEST_CASE("BinaryDataInterpreter: setCursor out of bounds") {
  uint8_t arr[] = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 3);
  REQUIRE_FALSE(bdi.setCursor(10));
  REQUIRE(bdi.getCursor() == 0);
}

// advanceCursor beyond data
// Advancing cursor past end should fail and not crash.
TEST_CASE("BinaryDataInterpreter: advanceCursor out of bounds") {
  uint8_t arr[] = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 3);
  REQUIRE_FALSE(bdi.advanceCursor(10));
  REQUIRE(bdi.getCursor() == 0);
}

// nextBytesEqual with insufficient data
// Should return false, not crash.
TEST_CASE("BinaryDataInterpreter: nextBytesEqual insufficient data") {
  uint8_t arr[] = {1, 2};
  util::BinaryDataInterpreter bdi(arr, 2);
  std::vector<uint8_t> cmp = {1, 2, 3};
  REQUIRE_FALSE(bdi.nextBytesEqual(cmp));
}

// findNextBytesAndAdvance with empty search
// Should return false, not crash.
TEST_CASE("BinaryDataInterpreter: findNextBytesAndAdvance empty search") {
  uint8_t arr[] = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 3);
  std::vector<uint8_t> empty;
  REQUIRE_FALSE(bdi.findNextBytesAndAdvance(empty, true));
}

// readNext string with nullptr
// Should return false, not crash.
TEST_CASE("BinaryDataInterpreter: readNext string nullptr") {
  uint8_t arr[] = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 3);
  REQUIRE_FALSE(bdi.readNext(nullptr, 2));
}

// readNext string with length exceeding data
// Should return false, not crash.
TEST_CASE("BinaryDataInterpreter: readNext string too long") {
  uint8_t arr[] = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 3);
  std::string out;
  REQUIRE_FALSE(bdi.readNext(&out, 10));
}

// readNext trivially copyable type with insufficient data
// Should return false, not crash.
TEST_CASE(
  "BinaryDataInterpreter: readNext trivially copyable insufficient data") {
  uint8_t arr[] = {1, 2};
  util::BinaryDataInterpreter bdi(arr, 2);
  uint32_t val = 0;
  REQUIRE_FALSE(bdi.readNext(&val));
}

// Construction from non-existent file
// Should not throw, but should not be ready.
TEST_CASE("BinaryDataInterpreter: Construct from non-existent file") {
  util::BinaryDataInterpreter bdi(
    std::filesystem::path("this_file_does_not_exist.bin"));
  REQUIRE_FALSE(bdi.isReady());
}

// Positive test for setCursorToEnd and setCursorToStart
TEST_CASE("BinaryDataInterpreter: setCursorToEnd and setCursorToStart") {
  uint8_t arr[] = {10, 20, 30, 40};
  util::BinaryDataInterpreter bdi(arr, 4);
  bdi.setCursorToEnd();
  REQUIRE(bdi.getCursor() == 4);
  bdi.setCursorToStart();
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for hasDataLeft
TEST_CASE("BinaryDataInterpreter: hasDataLeft positive") {
  uint8_t arr[] = {1, 2, 3, 4};
  util::BinaryDataInterpreter bdi(arr, 4);
  REQUIRE(bdi.hasDataLeft(2));
}

// Negative test for hasDataLeft
TEST_CASE("BinaryDataInterpreter: hasDataLeft negative") {
  uint8_t arr[] = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 3);
  bdi.setCursorToEnd();
  REQUIRE_FALSE(bdi.hasDataLeft(1));
}

// Positive test for nextBytesEqual
TEST_CASE("BinaryDataInterpreter: nextBytesEqual positive") {
  uint8_t arr[] = {5, 6, 7, 8};
  util::BinaryDataInterpreter bdi(arr, 4);
  std::vector<uint8_t> cmp = {5, 6};
  REQUIRE(bdi.nextBytesEqual(cmp));
}

// Negative test for nextBytesEqual
TEST_CASE("BinaryDataInterpreter: nextBytesEqual negative") {
  uint8_t arr[] = {5, 6, 7, 8};
  util::BinaryDataInterpreter bdi(arr, 4);
  std::vector<uint8_t> cmp = {6, 7};
  REQUIRE_FALSE(bdi.nextBytesEqual(cmp));
}

// Positive test for advanceCursor
TEST_CASE("BinaryDataInterpreter: advanceCursor positive") {
  uint8_t arr[] = {1, 2, 3, 4};
  util::BinaryDataInterpreter bdi(arr, 4);
  REQUIRE(bdi.advanceCursor(2));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for advanceCursor
TEST_CASE("BinaryDataInterpreter: advanceCursor negative") {
  uint8_t arr[] = {1, 2};
  util::BinaryDataInterpreter bdi(arr, 2);
  REQUIRE_FALSE(bdi.advanceCursor(3));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for advanceCursorIfEqual
TEST_CASE("BinaryDataInterpreter: advanceCursorIfEqual positive") {
  uint8_t arr[] = {9, 8, 7};
  util::BinaryDataInterpreter bdi(arr, 3);
  std::vector<uint8_t> cmp = {9, 8};
  REQUIRE(bdi.advanceCursorIfEqual(cmp));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for advanceCursorIfEqual
TEST_CASE("BinaryDataInterpreter: advanceCursorIfEqual negative") {
  uint8_t arr[] = {9, 8, 7};
  util::BinaryDataInterpreter bdi(arr, 3);
  std::vector<uint8_t> cmp = {8, 7};
  REQUIRE_FALSE(bdi.advanceCursorIfEqual(cmp));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for findNextBytesAndAdvance
TEST_CASE("BinaryDataInterpreter: findNextBytesAndAdvance positive") {
  uint8_t arr[] = {1, 2, 3, 4, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 6);
  std::vector<uint8_t> cmp = {2, 3};
  REQUIRE(bdi.findNextBytesAndAdvance(cmp, false));
  REQUIRE(bdi.getCursor() == 1);
}

// Positive test for findNextBytesAndAdvance beyond
TEST_CASE("BinaryDataInterpreter: findNextBytesAndAdvance beyond positive") {
  uint8_t arr[] = {1, 2, 3, 4, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 6);
  std::vector<uint8_t> cmp = {2, 3};
  REQUIRE(bdi.findNextBytesAndAdvance(cmp, true));
  REQUIRE(bdi.getCursor() == 3);
}

// Negative test for findNextBytesAndAdvance
TEST_CASE("BinaryDataInterpreter: findNextBytesAndAdvance negative") {
  uint8_t arr[] = {1, 2, 3, 4};
  util::BinaryDataInterpreter bdi(arr, 4);
  std::vector<uint8_t> cmp = {5, 6};
  REQUIRE_FALSE(bdi.findNextBytesAndAdvance(cmp, true));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for setCursor
TEST_CASE("BinaryDataInterpreter: setCursor positive") {
  uint8_t arr[] = {1, 2, 3, 4};
  util::BinaryDataInterpreter bdi(arr, 4);
  REQUIRE(bdi.setCursor(2));
  REQUIRE(bdi.getCursor() == 2);
}

// Negative test for setCursor
TEST_CASE("BinaryDataInterpreter: setCursor negative") {
  uint8_t arr[] = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 3);
  REQUIRE_FALSE(bdi.setCursor(5));
  REQUIRE(bdi.getCursor() == 0);
}

// Positive test for readNext (trivially copyable)
TEST_CASE("BinaryDataInterpreter: readNext trivially copyable positive") {
  uint8_t arr[] = {0x12, 0x34, 0x56, 0x78};
  util::BinaryDataInterpreter bdi(arr, 4);
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
  uint8_t arr[] = {1, 2, 3};
  util::BinaryDataInterpreter bdi(arr, 3);
  std::string out;
  REQUIRE_FALSE(bdi.readNext(&out, 5));
  REQUIRE(bdi.getCursor() == 0);
}
