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
  const std::vector<uint8_t> raw_data_big_endian{
    1,   16,  23,  107, 0,   1,   1,   6,   0,   0,   0,   0,   0,   0,   1,
    111, 0,   0,   1,   154, 250, 222, 205, 100, 89,  222, 16,  156, 0,   0,
    1,   6,   0,   0,   0,   0,   0,   0,   1,   87,  0,   0,   1,   154, 250,
    222, 205, 100, 1,   244, 2,   143, 255, 255, 207, 199, 0,   0,   0,   0,
    222, 173, 190, 239, 64,  72,  245, 195, 64,  25,  30,  184, 81,  235, 133,
    31,  0,   0,   0,   0,   0,   0,   0,   11,  104, 101, 108, 108, 111, 32,
    119, 111, 114, 108, 100, 0,   0,   0,   0,   0,   0,   0,   16,  119, 227,
    129, 147, 227, 130, 147, 227, 129, 171, 227, 129, 161, 227, 129, 175, 1,
    0,   0,   0,   42,  0,   0,   0,   0,   0,   0,   0,   1,   0,   0,   0,
    0,   0,   0,   0,   14,  118, 97,  114, 105, 97,  110, 116, 45,  115, 116,
    114, 105, 110, 103, 255, 255, 255, 249, 0,   0,   0,   77,  0,   0,   0,
    0,   0,   0,   0,   4,   0,   0,   0,   1,   0,   0,   0,   2,   0,   0,
    0,   3,   0,   0,   0,   4,   0,   0,   0,   0,   0,   0,   0,   2,   0,
    0,   0,   0,   0,   0,   0,   3,   111, 110, 101, 0,   0,   0,   0,   0,
    0,   0,   3,   116, 119, 111, 0,   0,   0,   0,   0,   0,   0,   2,   63,
    192, 0,   0,   64,  32,  0,   0,   0,   0,   0,   10,  0,   0,   0,   20,
    0,   0,   0,   30,  0,   0,   0,   0,   0,   0,   0,   2,   0,   0,   0,
    0,   0,   0,   0,   2,   107, 49,  0,   0,   0,   11,  0,   0,   0,   0,
    0,   0,   0,   2,   107, 50,  0,   0,   0,   22,  0,   0,   0,   0,   0,
    0,   0,   2,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   3,
    116, 119, 111, 0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   3,
    111, 110, 101, 0,   0,   0,   0,   0,   0,   0,   3,   0,   0,   0,   5,
    0,   0,   0,   6,   0,   0,   0,   7,   0,   0,   0,   0,   0,   0,   0,
    2,   0,   0,   0,   0,   0,   0,   0,   2,   117, 50,  0,   0,   0,   0,
    0,   0,   0,   2,   117, 49,  0,   0,   0,   99,  0,   0,   0,   0,   0,
    0,   0,   5,   116, 117, 112, 108, 101, 0,   0,   0,   0,   0,   1,   226,
    64};
  const std::vector<uint8_t> raw_data_little_endian{
    154, 194, 198, 249, 1,   0,   1,   7,   111, 1,   0,   0,   0,   0,   0,
    0,   149, 16,  189, 250, 154, 1,   0,   0,   169, 162, 189, 197, 0,   0,
    1,   7,   87,  1,   0,   0,   0,   0,   0,   0,   149, 16,  189, 250, 154,
    1,   0,   0,   1,   244, 143, 2,   199, 207, 255, 255, 239, 190, 173, 222,
    0,   0,   0,   0,   195, 245, 72,  64,  31,  133, 235, 81,  184, 30,  25,
    64,  11,  0,   0,   0,   0,   0,   0,   0,   104, 101, 108, 108, 111, 32,
    119, 111, 114, 108, 100, 16,  0,   0,   0,   0,   0,   0,   0,   119, 227,
    129, 147, 227, 130, 147, 227, 129, 171, 227, 129, 161, 227, 129, 175, 1,
    42,  0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   14,  0,   0,
    0,   0,   0,   0,   0,   118, 97,  114, 105, 97,  110, 116, 45,  115, 116,
    114, 105, 110, 103, 249, 255, 255, 255, 77,  0,   0,   0,   4,   0,   0,
    0,   0,   0,   0,   0,   1,   0,   0,   0,   2,   0,   0,   0,   3,   0,
    0,   0,   4,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   3,
    0,   0,   0,   0,   0,   0,   0,   111, 110, 101, 3,   0,   0,   0,   0,
    0,   0,   0,   116, 119, 111, 2,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   192, 63,  0,   0,   32,  64,  10,  0,   0,   0,   20,  0,   0,   0,
    30,  0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   2,   0,   0,
    0,   0,   0,   0,   0,   107, 49,  11,  0,   0,   0,   2,   0,   0,   0,
    0,   0,   0,   0,   107, 50,  22,  0,   0,   0,   2,   0,   0,   0,   0,
    0,   0,   0,   2,   0,   0,   0,   3,   0,   0,   0,   0,   0,   0,   0,
    116, 119, 111, 1,   0,   0,   0,   3,   0,   0,   0,   0,   0,   0,   0,
    111, 110, 101, 3,   0,   0,   0,   0,   0,   0,   0,   5,   0,   0,   0,
    6,   0,   0,   0,   7,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,
    0,   2,   0,   0,   0,   0,   0,   0,   0,   117, 50,  2,   0,   0,   0,
    0,   0,   0,   0,   117, 49,  99,  0,   0,   0,   5,   0,   0,   0,   0,
    0,   0,   0,   116, 117, 112, 108, 101, 64,  226, 1,   0,   0,   0,   0,
    0};

  // serialize
  serialize::BinaryDataWriter writer_native(367, 1024, std::endian::native);
  REQUIRE(writer_native.writeNext(testClass2));
  REQUIRE(writer_native.setWritingFinished(true));

  const std::vector<uint8_t> data_native = std::move(writer_native.releaseBuffer());

  /*
  for(uint8_t dieda : data_native){
    std::cerr << static_cast<short>(dieda) << ", ";
  }
  std::cerr << std::endl;
  */

  // deserialize
  serialize::BinaryDataReader rdr_native(data_native, std::endian::native);

  /*
  serialize::Header h1,h2;
  REQUIRE(rdr_native.readNext(&h1));
  REQUIRE(rdr_native.readNext(&h2));
  std::cerr << h1 << std::endl;
  std::cerr << h2 << std::endl;

  REQUIRE(rdr_native.setCursorToStart)
  */

  TestClass_2 deserialized_native;
  const bool serializedSuccess{rdr_native.readNext(&deserialized_native)};
  REQUIRE(serializedSuccess);

  auto checkClassEqual = [&testClass1](const TestClass_2& testClass) {
    REQUIRE(testClass.getClass().a_array == testClass1.a_array);
    REQUIRE(testClass.getClass().a_bool == testClass1.a_bool);
    REQUIRE(testClass.getClass().a_int8 == testClass1.a_int8);
    REQUIRE(testClass.getClass().a_uint16 == testClass1.a_uint16);
    REQUIRE(testClass.getClass().a_int32 == testClass1.a_int32);
    REQUIRE(testClass.getClass().a_uint64 == testClass1.a_uint64);
    REQUIRE(testClass.getClass().a_float == testClass1.a_float);
    REQUIRE(testClass.getClass().a_double == testClass1.a_double);
    REQUIRE(testClass.getClass().a_string == testClass1.a_string);
    REQUIRE(testClass.getClass().a_wstring == testClass1.a_wstring);
    REQUIRE(testClass.getClass().a_optional == testClass1.a_optional);
    REQUIRE(testClass.getClass().a_variant == testClass1.a_variant);
    REQUIRE(testClass.getClass().a_pair == testClass1.a_pair);
    REQUIRE(testClass.getClass().a_vector == testClass1.a_vector);
    REQUIRE(testClass.getClass().a_list == testClass1.a_list);
    REQUIRE(testClass.getClass().a_deque == testClass1.a_deque);
    REQUIRE(testClass.getClass().a_map == testClass1.a_map);
    REQUIRE(testClass.getClass().a_umap == testClass1.a_umap);
    REQUIRE(testClass.getClass().a_set == testClass1.a_set);
    REQUIRE(testClass.getClass().a_uset == testClass1.a_uset);
    REQUIRE(testClass.getClass().a_tuple == testClass1.a_tuple);
    REQUIRE(testClass.getClass().a_size == testClass1.a_size);
  };

  checkClassEqual(deserialized_native);

  // extract class payload from raw_data_little_endian buffer
  serialize::BinaryDataReader rdr_little(raw_data_little_endian, std::endian::little);
  TestClass_2 deserialized_liottle;
  REQUIRE(rdr_little.readNext(&deserialized_liottle));
  checkClassEqual(deserialized_liottle);

  // extract class payload from raw_data_big_endian buffer
  serialize::BinaryDataReader rdr_big(raw_data_big_endian, std::endian::big);
  TestClass_2 deserialized_big;
  REQUIRE(rdr_big.readNext(&deserialized_big));

  checkClassEqual(deserialized_big);
}

// NOLINTEND(readability-magic-numbers)
