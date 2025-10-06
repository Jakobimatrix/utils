/**
 * @file Serialize.hpp
 * @brief A parent class template for classes which need to be serialized/deserialized.
 *
 * @version 1.0
 * @date 01.10.2025
 */

#pragma once



#include <utils/data/BinaryDataReader.hpp>
#include <utils/data/BinaryDataWriter.hpp>

#include <cstddef>
#include <cstdint>
#include <chrono>

namespace serialize {

class Header {
  int32_t checksum{NO_CHECKSUM};
  uint16_t id{NO_ID};
  uint16_t version{NO_VERSION};
  uint64_t size{0};
  int64_t timestamp{NO_TIMESTAMP};

 public:
  static constexpr int32_t NO_CHECKSUM  = 0;
  static constexpr int64_t NO_TIMESTAMP = 0;
  static constexpr uint16_t NO_ID       = 0;
  static constexpr uint16_t NO_VERSION  = 0;

  Header(uint16_t id, uint16_t version, uint64_t size, int32_t checksum, int64_t timestamp);
  Header(uint16_t id, uint16_t version, uint64_t size);
  Header() = default;

  [[nodiscard]] uint16_t getId() const { return id; }
  [[nodiscard]] uint16_t getVersion() const { return version; }
  [[nodiscard]] uint64_t getSize() const { return size; }
  [[nodiscard]] int32_t getChecksum() const { return checksum; }
  [[nodiscard]] int64_t getTimestamp() const { return timestamp; }
  static constexpr size_t BYTES          = 4 + 2 * 2 + 2 * 8;
  static constexpr size_t CHECKSUM_BYTES = 4;

  int64_t static nowInMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
  }

  template <typename Container>
    requires std::convertible_to<typename Container::value_type, uint8_t>
  int32_t static calculateChecksum(const Container& binary) {
    int32_t checksum = static_cast<int32_t>(binary.size());
    for (const uint8_t b : binary) {
      constexpr int32_t prime = 31;
      checksum                = (checksum * prime) + static_cast<int32_t>(b);
    }
    if (checksum == NO_CHECKSUM) {
      checksum += 1;
    }
    return checksum;
  }

  template <class EnumId>
  EnumId id2Enum() {
    return id2Enum<EnumId>(id);
  }

  template <class EnumId>
  static EnumId id2Enum(uint16_t id) {
    return static_cast<EnumId>(id);
  }

  bool serialize(BinaryDataWriter& writer) const;
  bool deserialize(const BinaryDataReader& reader);

  static Header getHeader(const BinaryDataReader& reader) {
    Header header;
    [[maybe_unused]] const bool unused = header.deserialize(reader);
    return header;
  }
};

class Serializable {

  uint16_t id;
  uint16_t version;

  virtual ~Serializable()                      = default;
  Serializable(const Serializable&)            = default;
  Serializable& operator=(const Serializable&) = default;
  Serializable(Serializable&&)                 = default;
  Serializable& operator=(Serializable&&)      = default;

  Serializable(uint16_t version, uint16_t id);

  template <class EnumId>
  Serializable(uint16_t version, EnumId enum_id)
      : Serializable(version, Header::id2Enum<EnumId>(enum_id)) {}

  virtual bool serializeClass(BinaryDataWriter& writer) const   = 0;
  virtual bool deserializeClass(const BinaryDataReader& reader) = 0;

 public:
  bool serialize(BinaryDataWriter& writer) const;
  bool deserialize(const BinaryDataReader& reader);
  bool deserialize(const BinaryDataReader& reader, const Header& headerDeseriaized);
};



}  // namespace serialize
