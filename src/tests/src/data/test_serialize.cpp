/**
 * @file test_serialize.cpp
 * @brief contains tests for the Serialize class
 *
 * @date 2025
 * @author Jakob Wandel
 * @version 1.0
 **/


#include <catch2/catch_test_macros.hpp>
#include <utils/data/BinaryDataReader.hpp>
#include <utils/data/BinaryDataWriter.hpp>
#include <utils/data/Serialize.hpp>

#include <array>
#include <vector>
#include <map>
#include <cstdint>
#include <filesystem>
#include <string>
#include <bit>


// NOLINTBEGIN (readability-magic-numbers) This test uses some random numbers, there is no value in giving them a name
// Construction from valid data array
// Ensures that the class can be constructed from a valid byte array and is ready.



namespace test {

enum class Id : uint16_t { TestClass_1, TestClass_2 };

class TestClass_1 : public serialize::Serializable {

  static constexpr Id ID           = Id::TestClass_1;
  static constexpr int16_t VERSION = 1;

 public:
  TestClass_1()
      : serialize::Serializable(VERSION, ID) {};

  // Make members public for easy construction in the test
  bool a_bool{};
  int8_t a_int8{};
  uint16_t a_uint16{};
  int32_t a_int32{};
  uint64_t a_uint64{};
  float a_float{};
  double a_double{};
  std::string a_string{};
  std::wstring a_wstring{};
  std::optional<int32_t> a_optional{};
  std::variant<int32_t, std::string> a_variant{};
  std::pair<int32_t, uint32_t> a_pair{};
  std::vector<int32_t> a_vector{};
  std::list<std::string> a_list{};
  std::deque<float> a_deque{};
  std::array<int32_t, 3> a_array{};
  std::map<std::string, int32_t> a_map{};
  std::unordered_map<int32_t, std::string> a_umap{};
  std::set<int32_t> a_set{};
  std::unordered_set<std::string> a_uset{};
  std::tuple<int32_t, std::string> a_tuple{};
  std::size_t a_size{};

 private:
  bool serializeClass(serialize::BinaryDataWriter& writer) const override {
    return writer.writeNext(a_bool) && writer.writeNext(a_int8) &&
           writer.writeNext(a_uint16) && writer.writeNext(a_int32) &&
           writer.writeNext(a_uint64) && writer.writeNext(a_float) &&
           writer.writeNext(a_double) && writer.writeNext(a_string) &&
           writer.writeNext(a_wstring) && writer.writeNext(a_optional) &&
           writer.writeNext(a_variant) && writer.writeNext(a_pair) &&
           writer.writeNext(a_vector) && writer.writeNext(a_list) &&
           writer.writeNext(a_deque) && writer.writeNext(a_array) &&
           writer.writeNext(a_map) && writer.writeNext(a_umap) &&
           writer.writeNext(a_set) && writer.writeNext(a_uset) &&
           writer.writeNext(a_tuple) && writer.writeNext(a_size);
  }

  bool deserializeClass(const serialize::BinaryDataReader& reader) override {
    return reader.readNext(&a_bool) && reader.readNext(&a_int8) &&
           reader.readNext(&a_uint16) && reader.readNext(&a_int32) &&
           reader.readNext(&a_uint64) && reader.readNext(&a_float) &&
           reader.readNext(&a_double) && reader.readNext(&a_string) &&
           reader.readNext(&a_wstring) && reader.readNext(&a_optional) &&
           reader.readNext(&a_variant) && reader.readNext(&a_pair) &&
           reader.readNext(&a_vector) && reader.readNext(&a_list) &&
           reader.readNext(&a_deque) && reader.readNext(&a_array) &&
           reader.readNext(&a_map) && reader.readNext(&a_umap) &&
           reader.readNext(&a_set) && reader.readNext(&a_uset) &&
           reader.readNext(&a_tuple) && reader.readNext(&a_size);
  }
};

class TestClass_2 : public serialize::Serializable {

  static constexpr Id ID           = Id::TestClass_2;
  static constexpr int16_t VERSION = 1;

  TestClass_1 m_class;

 public:
  TestClass_2()
      : serialize::Serializable(VERSION, ID) {}

  TestClass_2(const TestClass_1& cls)
      : serialize::Serializable(VERSION, ID),
        m_class(cls) {}

  const TestClass_1& getClass() const { return m_class; }

 private:
  bool serializeClass(serialize::BinaryDataWriter& writer) const override {
    // write the contained serializable by value (will call its serialize)
    return writer.writeNext(m_class);
  }

  bool deserializeClass(const serialize::BinaryDataReader& reader) override {
    // read into the contained object by pointer
    return reader.readNext(&m_class);
  }
};
}  // namespace test

TEST_CASE("Test serialization deserialization") {
  using namespace test;

  // create and populate TestClass_1
  TestClass_1 testClass1;
  testClass1.a_bool      = true;
  testClass1.a_int8      = -12;
  testClass1.a_uint16    = 655;
  testClass1.a_int32     = -12345;
  testClass1.a_uint64    = 0xDEADBEEFull;
  testClass1.a_float     = 3.14f;
  testClass1.a_double    = 6.28;
  testClass1.a_string    = "hello world";
  testClass1.a_wstring   = L"wこんにちは";
  testClass1.a_optional  = 42;
  testClass1.a_variant   = std::string("variant-string");
  testClass1.a_pair      = {-7, 77};
  testClass1.a_vector    = {1, 2, 3, 4};
  testClass1.a_list      = {"one", "two"};
  testClass1.a_deque     = {1.5f, 2.5f};
  testClass1.a_array     = {10, 20, 30};
  testClass1.a_map["k1"] = 11;
  testClass1.a_map["k2"] = 22;
  testClass1.a_umap[1]   = "one";
  testClass1.a_umap[2]   = "two";
  testClass1.a_set       = {5, 6, 7};
  testClass1.a_uset      = {"u1", "u2"};
  testClass1.a_tuple     = std::make_tuple(99, std::string("tuple"));
  testClass1.a_size      = static_cast<std::size_t>(123456);

  TestClass_2 testClass2(testClass1);

  // serialize
  serialize::BinaryDataWriter writer(1024, 1024 * 1024, std::endian::native);
  REQUIRE(writer.writeNext(testClass2));
  REQUIRE(writer.setWritingFinished());

  std::cerr << "\n size after write: " << writer.size() << "\n" << std::flush;

  const std::vector<uint8_t> data = std::move(writer.releaseBuffer());

  std::cerr << "\n again: " << data.size() << "\n" << std::flush;

  // extract class payload from original buffer
  serialize::BinaryDataReader rdr_for_payload(data, std::endian::native);
  const auto header_orig  = serialize::Header::getHeader(rdr_for_payload);
  const size_t start_orig = rdr_for_payload.getCursor();
  const size_t size_orig  = static_cast<size_t>(header_orig.getSize());
  std::vector<uint8_t> payload_orig(
    std::next(data.begin(), static_cast<ptrdiff_t>(start_orig)),
    std::next(data.begin(), static_cast<ptrdiff_t>(start_orig + size_orig)));

  std::cerr << "\n header: " << header_orig << "\n" << std::flush;

  // deserialize
  serialize::BinaryDataReader rdr(data, std::endian::native);
  TestClass_2 deserialized;
  const bool serializedSuccess{rdr.readNext(&deserialized)};
  REQUIRE(serializedSuccess);

  // re-serialize deserialized object
  serialize::BinaryDataWriter writer2(1024, 1024 * 1024, std::endian::native);
  REQUIRE(writer2.writeNext(deserialized));
  REQUIRE(writer2.setWritingFinished());
  auto data2 = writer2.releaseBuffer();

  // extract class payload from re-serialized buffer
  serialize::BinaryDataReader rdr_for_payload2(data2, std::endian::native);
  const auto header_new  = serialize::Header::getHeader(rdr_for_payload2);
  const size_t start_new = rdr_for_payload2.getCursor();
  const size_t size_new  = static_cast<size_t>(header_new.getSize());
  std::vector<uint8_t> payload_new(
    std::next(data2.begin(), static_cast<ptrdiff_t>(start_new)),
    std::next(data2.begin(), static_cast<ptrdiff_t>(start_new + size_new)));

  REQUIRE(size_orig == size_new);
  REQUIRE(payload_orig == payload_new);
}

// NOLINTEND(readability-magic-numbers)
