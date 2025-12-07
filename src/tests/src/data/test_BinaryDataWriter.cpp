/**
 * @file test_BinaryDataWriter.cpp
 * @brief contains tests for the BinaryDataWriter class
 *
 * @date 2025
 * @author Jakob Wandel
 * @version 1.0
 **/


#include <bit>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <utility>
#include <cstdint>
#include <utils/data/BinaryDataWriter.hpp>
#include <utils/data/BinaryDataReader.hpp>
#include <vector>
#include <map>
#include <set>

using namespace serialize;
// NOLINTBEGIN (readability-magic-numbers) This test uses some random numbers, there is no value in giving them a name

TEST_CASE("BinaryDataWriter: writeNext string success and failure") {
  BinaryDataWriter writer(0, 1024, std::endian::little);
  REQUIRE(writer.writeNext(std::string("hi")));
  REQUIRE(writer.setWritingFinished(true));
  auto buf = writer.releaseBuffer();
  // reader should decode it
  BinaryDataReader const reader(std::move(buf), std::endian::little);
  std::string out;
  REQUIRE(reader.readNext(&out));
  REQUIRE(out == "hi");
}

TEST_CASE(
  "BinaryDataWriter: writeNext scalars endianness and overflow failure") {
  BinaryDataWriter writer(0, 3, std::endian::little);  // small maxExpectedSize to force overflow (3 < 4)
  REQUIRE_FALSE(writer.writeNext(uint32_t(0x12345678)));  // would exceed (4 bytes)
  BinaryDataWriter writer2(0, 1024, std::endian::little);
  REQUIRE(writer2.writeNext(uint32_t(0x01020304)));
  REQUIRE(writer2.setWritingFinished(true));
  auto buf = writer2.releaseBuffer();
  BinaryDataReader const reader2(std::move(buf), std::endian::little);
  uint32_t decodedValue = 0;
  REQUIRE(reader2.readNext(&decodedValue));
  REQUIRE(decodedValue == 0x01020304);
}

TEST_CASE("BinaryDataWriter: container writes success and failure") {
  BinaryDataWriter writer(0, 1024, std::endian::little);
  std::vector<int> const verctor{1, 2, 3};
  REQUIRE(writer.writeNext(verctor));
  std::set<int> const set{4, 5};
  REQUIRE(writer.writeNext(set));
  std::map<int, int> map;
  map.emplace(1, 10);
  REQUIRE(writer.writeNext(map));
  REQUIRE(writer.setWritingFinished(true));
  auto buf = writer.releaseBuffer();
  BinaryDataReader const reader(std::move(buf), std::endian::little);
  std::vector<int> readVector;
  REQUIRE(reader.readNext(&readVector));
  REQUIRE(readVector.size() == 3);
  std::set<int> readSet;
  REQUIRE(reader.readNext(&readSet));
  REQUIRE(readSet.size() == 2);
  std::map<int, int> readMap;
  REQUIRE(reader.readNext(&readMap));
  REQUIRE(readMap.size() == 1);

  // Failure: create writer with tiny maxExpectedSize and try to write container
  BinaryDataWriter smallWriter(0, 4, std::endian::little);
  std::vector<int> const big(1000, 1);
  REQUIRE_FALSE(smallWriter.writeNext(big));
}
// NOLINTEND (readability-magic-numbers)
