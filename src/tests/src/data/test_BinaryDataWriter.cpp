/**
 * @file test_BinaryDataWriter.cpp
 * @brief contains tests for the BinaryDataWriter class
 *
 * @date 2025
 * @author Jakob Wandel
 * @version 1.0
 **/


#include <catch2/catch_test_macros.hpp>
#include <utils/data/BinaryDataWriter.hpp>
#include <utils/data/BinaryDataReader.hpp>
#include <vector>
#include <map>
#include <set>

using namespace serialize;

TEST_CASE("BinaryDataWriter: writeNext string success and failure") {
  BinaryDataWriter writer(0, 1024, std::endian::little);
  REQUIRE(writer.writeNext(std::string("hi")));
  REQUIRE(writer.setWritingFinished());
  auto buf = writer.releaseBuffer();
  // reader should decode it
  BinaryDataReader r(std::move(buf), std::endian::little);
  std::string out;
  REQUIRE(r.readNext(&out));
  REQUIRE(out == "hi");
}

TEST_CASE(
  "BinaryDataWriter: writeNext scalars endianness and overflow failure") {
  BinaryDataWriter writer(0, 3, std::endian::little);  // small maxExpectedSize to force overflow (3 < 4)
  REQUIRE_FALSE(writer.writeNext(uint32_t(0x12345678)));  // would exceed (4 bytes)
  BinaryDataWriter writer2(0, 1024, std::endian::little);
  REQUIRE(writer2.writeNext(uint32_t(0x01020304)));
  REQUIRE(writer2.setWritingFinished());
  auto buf = writer2.releaseBuffer();
  BinaryDataReader r(std::move(buf), std::endian::little);
  uint32_t v;
  REQUIRE(r.readNext(&v));
  REQUIRE(v == 0x01020304);
}

TEST_CASE("BinaryDataWriter: container writes success and failure") {
  BinaryDataWriter writer(0, 1024, std::endian::little);
  std::vector<int> v{1, 2, 3};
  REQUIRE(writer.writeNext(v));
  std::set<int> s{4, 5};
  REQUIRE(writer.writeNext(s));
  std::map<int, int> m;
  m.emplace(1, 10);
  REQUIRE(writer.writeNext(m));
  REQUIRE(writer.setWritingFinished());
  auto buf = writer.releaseBuffer();
  BinaryDataReader r(std::move(buf), std::endian::little);
  std::vector<int> rv;
  REQUIRE(r.readNext(&rv));
  REQUIRE(rv.size() == 3);
  std::set<int> rs;
  REQUIRE(r.readNext(&rs));
  REQUIRE(rs.size() == 2);
  std::map<int, int> rm;
  REQUIRE(r.readNext(&rm));
  REQUIRE(rm.size() == 1);

  // Failure: create writer with tiny maxExpectedSize and try to write container
  BinaryDataWriter w2(0, 4, std::endian::little);
  std::vector<int> big(1000, 1);
  REQUIRE_FALSE(w2.writeNext(big));
}
