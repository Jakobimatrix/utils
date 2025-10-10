/**
 * @file Serialize.hpp
 * @brief A parent class template for classes which need to be serialized/deserialized.
 *
 * @version 1.0
 * @date 01.10.2025
 */

#pragma once



#include <concepts>
#include <utils/data/BinaryDataReader.hpp>
#include <utils/data/BinaryDataWriter.hpp>

#include <bit>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <chrono>

namespace serialize {

class Flags {
  // layout (bit indices, LSB = 0):
  // bit 0    : endian (0 = big, 1 = little)
  // bit 1    : control/checksum enabled
  // bit 2    : timestamp enabled
  // bits 3-4 : compression algorithm (2 bits)
  // bits 5-6 : encryption algorithm (2 bits)
  // bit 7    : strict mode (if set, enforce strict version/format checks)
  std::bitset<8> m_flags{0};

 public:
  Flags() = default;

  void setEndian(std::endian endian) noexcept {
    // Only two encodings: big (0) and little (1). Any non-little value is treated as big.
    m_flags[0] = (endian == std::endian::little);
  }

  std::endian getEndian() const noexcept {
    return m_flags[0] ? std::endian::little : std::endian::big;
  }

  void setControlHash(bool enabled) noexcept { m_flags[1] = enabled; }
  bool getControlHash() const noexcept { return m_flags[1]; }

  void setTime(bool enabled) noexcept { m_flags[2] = enabled; }
  bool getTime() const noexcept { return m_flags[2]; }

  enum class Compression : uint8_t {
    None  = 0,
    Algo1 = 1,
    Algo2 = 2,
    Algo3 = 3
  };
  enum class Encryption : uint8_t { None = 0, Algo1 = 1, Algo2 = 2, Algo3 = 3 };

  void setCompression(Compression c) noexcept {
    const uint8_t v = static_cast<uint8_t>(c) & 0x3u;
    // bits 3 (LSB) and 4 (MSB)
    m_flags[3] = static_cast<bool>(v & 0x1u);
    m_flags[4] = static_cast<bool>((v >> 1) & 0x1u);
  }

  Compression getCompression() const noexcept {
    const uint8_t v = (static_cast<uint8_t>(m_flags[3]) ? 1u : 0u) |
                      (static_cast<uint8_t>(m_flags[4]) << 1);
    return static_cast<Compression>(v & 0x3u);
  }

  void setEncryption(Encryption e) noexcept {
    const uint8_t v = static_cast<uint8_t>(e) & 0x3u;
    // bits 5 (LSB) and 6 (MSB)
    m_flags[5] = static_cast<bool>(v & 0x1u);
    m_flags[6] = static_cast<bool>((v >> 1) & 0x1u);
  }

  Encryption getEncryption() const noexcept {
    const uint8_t v = (static_cast<uint8_t>(m_flags[5]) ? 1u : 0u) |
                      (static_cast<uint8_t>(m_flags[6]) << 1);
    return static_cast<Encryption>(v & 0x3u);
  }

  void setStrictMode(bool enabled) noexcept { m_flags[7] = enabled; }
  bool getStrictMode() const noexcept { return m_flags[7]; }
  uint8_t toByte() const noexcept {
    return static_cast<uint8_t>(m_flags.to_ullong() & 0xFFu);
  }
  void fromByte(uint8_t b) noexcept { m_flags = std::bitset<8>(b); }
};

class Header {
  int32_t m_checksum{NO_CHECKSUM};
  uint16_t m_id{NO_ID};
  uint8_t m_version{NO_VERSION};
  Flags m_flags;
  uint64_t m_size{0};
  int64_t m_timestamp{NO_TIMESTAMP};

 public:
  static constexpr int32_t NO_CHECKSUM  = 0;
  static constexpr int64_t NO_TIMESTAMP = 0;
  static constexpr uint16_t NO_ID       = std::numeric_limits<uint16_t>::max();
  static constexpr uint8_t NO_VERSION   = 0;

  Header(uint16_t id, uint8_t version, uint64_t size, Flags flags, int32_t checksum, int64_t timestamp);
  Header(uint16_t id, uint8_t version, uint64_t size, Flags flags);
  Header() = default;

  [[nodiscard]] uint16_t getId() const { return m_id; }
  [[nodiscard]] uint8_t getVersion() const { return m_version; }
  [[nodiscard]] uint64_t getSize() const { return m_size; }
  [[nodiscard]] int32_t getChecksum() const { return m_checksum; }
  [[nodiscard]] int64_t getTimestamp() const { return m_timestamp; }
  [[nodiscard]] const Flags& getFlags() const noexcept { return m_flags; }
  static constexpr size_t BYTES          = 4 + 2 + 1 + 1 + 8 + 8;
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
    return id2Enum<EnumId>(m_id);
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

  bool hasVersion() const { return m_version != NO_VERSION; }

  bool hasId() const { return m_id != NO_ID; }

  bool hasHash() const { return m_checksum != NO_CHECKSUM; }

  bool hasTimestamp() const { return m_timestamp != NO_TIMESTAMP; }
};

class Serializable {

  uint16_t m_id;
  uint16_t m_version;

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
  bool serialize(BinaryDataWriter& writer, Flags flags) const;
  bool deserialize(const BinaryDataReader& reader);
  bool deserialize(const BinaryDataReader& reader, const Header& header_deseriaized);
};



}  // namespace serialize
