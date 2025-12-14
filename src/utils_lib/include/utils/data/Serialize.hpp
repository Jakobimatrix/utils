/**
 * @file Serialize.hpp
 * @brief A parent class template for classes which need to be serialized/deserialized.
 *
 * @date 01.10.2025
 */

#pragma once



#include <concepts>
#include <limits>
#include <ostream>
#include <optional>
#include <utils/data/BinaryDataReader.hpp>
#include <utils/data/BinaryDataWriter.hpp>

#include <type_traits>
#include <string>
#include <sstream>
#include <array>

#include <bit>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <chrono>

namespace serialize {

/**
 * @brief Compile-time fingerprint of the current compilation target.
 *
 * This constexpr helper builds a small bitset that captures properties that
 * can differ between platforms/compilers/ABIs (sizes and signedness of
 * certain fundamental types, pointer size, endianness). The fingerprint is
 * intended to be embedded into serialized data so a reader can detect when
 * the data was produced for a different ABI and react accordingly.
 */
namespace fingerprint {
constexpr std::size_t bit_count = 8;
using fingerprint_t             = std::bitset<bit_count>;

// bit indices
constexpr std::size_t BIT_PTR_WIDTH       = 0;
constexpr std::size_t BIT_LP64            = 1;
constexpr std::size_t BIT_LLP64           = 2;
constexpr std::size_t BIT_WCHAR32         = 3;
constexpr std::size_t BIT_LONG_DOUBLE_GT  = 4;
constexpr std::size_t BIT_X87_LONG_DOUBLE = 5;
constexpr std::size_t BIT_BITFIELD_MSVC   = 6;
constexpr std::size_t BIT_ENUM_INT        = 7;

enum class test_enum { A, B };

struct bitfield_test {
  unsigned a : 3;
  unsigned b : 5;
};

inline fingerprint_t make() {
  fingerprint_t fp{};

  constexpr std::size_t PTR_WIDTH_64       = 8;
  constexpr std::size_t LONG_64            = 8;
  constexpr std::size_t LONG_32            = 4;
  constexpr std::size_t WCHAR_32           = 4;
  constexpr std::size_t BITFIELD_FULL_SIZE = sizeof(unsigned);
  constexpr std::size_t X87_SIZE_1         = 10;
  constexpr std::size_t X87_SIZE_2         = 16;

  fp[BIT_PTR_WIDTH] = (sizeof(void*) >= PTR_WIDTH_64);  // pointer width >=64
  fp[BIT_LP64]      = (sizeof(long) == LONG_64);        // LP64
  fp[BIT_LLP64] = (sizeof(void*) == PTR_WIDTH_64 && sizeof(long) == LONG_32);  // LLP64
  fp[BIT_WCHAR32]        = (sizeof(wchar_t) == WCHAR_32);
  fp[BIT_LONG_DOUBLE_GT] = (sizeof(long double) > sizeof(double));
  fp[BIT_X87_LONG_DOUBLE] =
    (sizeof(long double) == X87_SIZE_1 || sizeof(long double) == X87_SIZE_2);  // x87-style
  fp[BIT_BITFIELD_MSVC] =
    (sizeof(bitfield_test) == BITFIELD_FULL_SIZE);  // MSVC packing heuristic
  fp[BIT_ENUM_INT] = std::is_same_v<std::underlying_type_t<test_enum>, int>;

  return fp;
}

inline fingerprint_t value = make();

inline void explainFingerprint(std::ostream& os, const fingerprint_t& fp) {
  os << "ABI fingerprint (" << bit_count << " bits)\n";

  auto yesno = [&](bool v) { return v ? "yes" : "no"; };

  os << "pointer width >= 64        : " << yesno(fp[BIT_PTR_WIDTH]) << "\n";
  os << "LP64 model                 : " << yesno(fp[BIT_LP64]) << "\n";
  os << "LLP64 model                : " << yesno(fp[BIT_LLP64]) << "\n";
  os << "wchar_t is 32-bit          : " << yesno(fp[BIT_WCHAR32]) << "\n";
  os << "long double > double       : " << yesno(fp[BIT_LONG_DOUBLE_GT]) << "\n";
  os << "x87-style long double      : " << yesno(fp[BIT_X87_LONG_DOUBLE]) << "\n";
  os << "MSVC-like bitfield packing : " << yesno(fp[BIT_BITFIELD_MSVC]) << "\n";
  os << "enum underlying == int     : " << yesno(fp[BIT_ENUM_INT]) << "\n";

  os << "raw bits (MSB->LSB): " << fp.to_string();
  os << '\n';
}
}  // namespace fingerprint


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

  [[nodiscard]] std::endian getEndian() const noexcept {
    return m_flags[0] ? std::endian::little : std::endian::big;
  }

  void setControlHash(bool enabled) noexcept { m_flags[1] = enabled; }
  [[nodiscard]] bool getControlHash() const noexcept { return m_flags[1]; }

  void setTime(bool enabled) noexcept { m_flags[2] = enabled; }
  [[nodiscard]] bool getTime() const noexcept { return m_flags[2]; }

  enum class Compression : uint8_t {
    None  = 0,
    Algo1 = 1,
    Algo2 = 2,
    Algo3 = 3
  };
  enum class Encryption : uint8_t { None = 0, Algo1 = 1, Algo2 = 2, Algo3 = 3 };

  void setCompression(Compression c) noexcept {
    const uint8_t v = static_cast<uint8_t>(c) & 0x3U;
    // bits 3 (LSB) and 4 (MSB)
    m_flags[3] = static_cast<bool>(v & 0x1U);
    m_flags[4] = static_cast<bool>((v >> 1) & 0x1U);
  }

  [[nodiscard]] Compression getCompression() const noexcept {
    const uint8_t v =
      (m_flags[3] ? static_cast<uint8_t>(1U) : static_cast<uint8_t>(0U)) |
      static_cast<uint8_t>(
        (m_flags[4] ? static_cast<uint8_t>(1U) : static_cast<uint8_t>(0U)) << 1);
    return static_cast<Compression>(v & 0x3U);
  }

  void setEncryption(Encryption e) noexcept {
    const uint8_t v = static_cast<uint8_t>(e) & 0x3U;
    // bits 5 (LSB) and 6 (MSB)
    m_flags[5] = static_cast<bool>(v & 0x1U);
    m_flags[6] = static_cast<bool>((v >> 1) & 0x1U);
  }

  [[nodiscard]] Encryption getEncryption() const noexcept {
    const uint8_t v =
      (m_flags[5] ? static_cast<uint8_t>(1U) : static_cast<uint8_t>(0U)) |
      static_cast<uint8_t>(
        (m_flags[6] ? static_cast<uint8_t>(1U) : static_cast<uint8_t>(0U)) << 1);
    return static_cast<Encryption>(v & 0x3U);
  }

  void setStrictMode(bool enabled) noexcept { m_flags[7] = enabled; }
  [[nodiscard]] bool getStrictMode() const noexcept { return m_flags[7]; }

  bool serialize(BinaryDataWriter& writer) const;
  bool deserialize(const BinaryDataReader& reader);
};

class Header {
  int32_t m_checksum{NO_CHECKSUM};
  uint16_t m_id{NO_ID};
  uint8_t m_version{NO_VERSION};
  Flags m_flags;
  std::bitset<8> m_fingerprint{fingerprint::make()};
  uint32_t m_size{0};
  int64_t m_timestamp{NO_TIMESTAMP};

 public:
  static constexpr int32_t NO_CHECKSUM  = 0;
  static constexpr int64_t NO_TIMESTAMP = 0;
  static constexpr uint16_t NO_ID       = std::numeric_limits<uint16_t>::max();
  static constexpr uint8_t NO_VERSION   = 0;

  Header(uint16_t id, uint8_t version, uint32_t size, Flags flags, int32_t checksum, int64_t timestamp);
  Header(uint16_t id, uint8_t version, uint32_t size, Flags flags);
  Header() = default;

  [[nodiscard]] uint16_t getId() const { return m_id; }
  [[nodiscard]] uint8_t getVersion() const { return m_version; }
  [[nodiscard]] uint32_t getSize() const { return m_size; }
  [[nodiscard]] int32_t getChecksum() const { return m_checksum; }
  [[nodiscard]] int64_t getTimestamp() const { return m_timestamp; }
  [[nodiscard]] std::bitset<8> getFingerprint() const { return m_fingerprint; }
  [[nodiscard]] const Flags& getFlags() const noexcept { return m_flags; }
  static constexpr size_t BYTES          = 4 + 2 + 1 + 1 + 4 + 8;
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
  static constexpr uint16_t enum2Id(EnumId enumid) {
    return static_cast<uint16_t>(enumid);
  }

  template <class EnumId>
  static constexpr EnumId id2Enum(uint16_t id) {
    return static_cast<EnumId>(id);
  }

  bool serialize(BinaryDataWriter& writer) const;
  bool deserialize(const BinaryDataReader& reader);

  static Header getHeader(const BinaryDataReader& reader) {
    Header header;
    [[maybe_unused]] const bool unused = header.deserialize(reader);
    return header;
  }

  [[nodiscard]] std::endian getEndian() const { return m_flags.getEndian(); }

  [[nodiscard]] bool hasVersion() const { return m_version != NO_VERSION; }

  [[nodiscard]] bool hasId() const { return m_id != NO_ID; }

  [[nodiscard]] bool hasHash() const { return m_checksum != NO_CHECKSUM; }

  [[nodiscard]] bool hasTimestamp() const {
    return m_timestamp != NO_TIMESTAMP;
  }

  friend std::ostream& operator<<(std::ostream& os, const Header& header) {
    os << "Header {\n"
       << "  id: " << header.m_id << "\n"
       << "  version: " << static_cast<int>(header.m_version) << "\n"
       << "  size: " << header.m_size << "\n"
       << "  checksum: " << header.m_checksum << "\n"
       << "  timestamp: " << header.m_timestamp << "\n"
       << "  flags: {\n"
       << "    endian: "
       << (header.m_flags.getEndian() == std::endian::little ? "little" : "big") << "\n"
       << "    control hash: " << (header.m_flags.getControlHash() ? "enabled" : "disabled")
       << "\n";
    fingerprint::explainFingerprint(os, header.m_fingerprint);
    os << "\n    timestamp: " << (header.m_flags.getTime() ? "enabled" : "disabled") << "\n"
       << "    compression: ";
    switch (header.m_flags.getCompression()) {
      case Flags::Compression::None:
        os << "None";
        break;
      case Flags::Compression::Algo1:
        os << "Algo1";
        break;
      case Flags::Compression::Algo2:
        os << "Algo2";
        break;
      case Flags::Compression::Algo3:
        os << "Algo3";
        break;
      default:
        os << "Unknown";
        break;
    }
    os << "\n"
       << "    encryption: ";
    switch (header.m_flags.getEncryption()) {
      case Flags::Encryption::None:
        os << "None";
        break;
      case Flags::Encryption::Algo1:
        os << "Algo1";
        break;
      case Flags::Encryption::Algo2:
        os << "Algo2";
        break;
      case Flags::Encryption::Algo3:
        os << "Algo3";
        break;
      default:
        os << "Unknown";
        break;
    }
    os << "\n"
       << "    strict mode: " << (header.m_flags.getStrictMode() ? "enabled" : "disabled")
       << "\n"
       << "  }\n";
    os << "}";
    return os;
  }
};

class Serializable {

  uint16_t m_id;
  uint8_t m_version;

 protected:
  virtual ~Serializable()                      = default;
  Serializable(const Serializable&)            = default;
  Serializable& operator=(const Serializable&) = default;
  Serializable(Serializable&&)                 = default;
  Serializable& operator=(Serializable&&)      = default;
  Serializable(uint8_t version, uint16_t id);

  template <class EnumId>
  Serializable(uint8_t version, EnumId enum_id)
      : Serializable(version, Header::enum2Id<EnumId>(enum_id)) {}

  virtual bool serializeClass(BinaryDataWriter& writer) const   = 0;
  virtual bool deserializeClass(const BinaryDataReader& reader) = 0;

 public:
  bool deserialize(const BinaryDataReader& reader, const Header& header_deseriaized);
  bool serialize(BinaryDataWriter& writer) const;
  bool serialize(BinaryDataWriter& writer, Flags flags) const;
  bool deserialize(const BinaryDataReader& reader);
  static std::optional<Header> deserializeHeader(const BinaryDataReader& reader);
};



}  // namespace serialize
