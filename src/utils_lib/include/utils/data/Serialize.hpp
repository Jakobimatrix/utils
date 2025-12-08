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
class SystemFingerprint {
 public:
  static constexpr std::size_t BITS = 8U;
  // Bit layout (index => meaning)
  // 0 : char is signed (1) or unsigned (0)
  // 1 : sizeof(size_t) >= 8
  // 2 : sizeof(ptrdiff_t) >= 8
  // 3 : sizeof(long) >= 8
  // 4 : long long is 64-bit
  // remaining bits reserved for future use
  std::bitset<BITS> m_fingerprint;
  std::uint64_t m_size_hash{};

  using CanonicalSize_t = uint64_t;


  constexpr SystemFingerprint() noexcept
      : m_fingerprint(getFingerprint()),
        m_size_hash(buildSizeHash()) {}


  static constexpr unsigned long long getFingerprint() noexcept {
    // char signedness
    int fingerprint{};
    fingerprint = static_cast<int>(std::is_signed_v<char>);

    // size_t, ptrdiff_t, long widths
    fingerprint |= static_cast<int>(sizeof(std::size_t) >= 8) << 1;
    fingerprint |= static_cast<int>(sizeof(std::ptrdiff_t) >= 8) << 2;
    fingerprint |= static_cast<int>(sizeof(long) >= 8) << 3;
    // long long width
    fingerprint |= static_cast<int>(sizeof(long long) >= 8) << 4;
    return static_cast<unsigned long long>(fingerprint);
  }

  // Canonical wire sizes for types we normalize on the wire. If a local
  // type's sizeof() matches the canonical size, it is safe to serialize
  // using the canonical representation without data loss.
  static constexpr std::size_t CANONICAL_SIZE_char      = 1;
  static constexpr std::size_t CANONICAL_SIZE_short     = 2;
  static constexpr std::size_t CANONICAL_SIZE_int       = 4;
  static constexpr std::size_t CANONICAL_SIZE_long      = 8;
  static constexpr std::size_t CANONICAL_SIZE_long_long = 8;
  static constexpr std::size_t CANONICAL_SIZE_float     = 4;
  static constexpr std::size_t CANONICAL_SIZE_double    = 8;
  static constexpr std::size_t CANONICAL_SIZE_size_t    = 8;
  static constexpr std::size_t CANONICAL_SIZE_ptrdiff_t = 8;

  // We treat `long double` specially: we do NOT provide a canonical wire
  // representation here. Instead we record its size in the size-hash so a
  // reader can detect mismatches and refuse deserialization if required.

  // constexpr helper: is the given type's sizeof() equal to our canonical size?
  template <typename T>
  static consteval bool isCanonicalType() noexcept {
    if constexpr (std::is_same_v<T, char>) {
      return sizeof(T) == CANONICAL_SIZE_char;
    } else if constexpr (std::is_same_v<T, short>) {
      return sizeof(T) == CANONICAL_SIZE_short;
    } else if constexpr (std::is_same_v<T, int>) {
      return sizeof(T) == CANONICAL_SIZE_int;
    } else if constexpr (std::is_same_v<T, long>) {
      return sizeof(T) == CANONICAL_SIZE_long;
    } else if constexpr (std::is_same_v<T, long long>) {
      return sizeof(T) == CANONICAL_SIZE_long_long;
    } else if constexpr (std::is_same_v<T, float>) {
      return sizeof(T) == CANONICAL_SIZE_float;
    } else if constexpr (std::is_same_v<T, double>) {
      return sizeof(T) == CANONICAL_SIZE_double;
    } else if constexpr (std::is_same_v<T, std::size_t>) {
      return sizeof(T) == CANONICAL_SIZE_size_t;
    } else if constexpr (std::is_same_v<T, std::ptrdiff_t>) {
      return sizeof(T) == CANONICAL_SIZE_ptrdiff_t;
    } else if constexpr (std::is_same_v<T, long double>) {
      // long double is intentionally non-canonical — exact round-trip only
      // guaranteed when the size-hash matches the writer.
      return false;
    } else {
      // For other types, conservatively return whether size matches the
      // platform sizeof (i.e., always true) so they are treated as canonical.
      return true;
    }
  }

  // Build a compact, stable hash from the sizeof() values of types that may
  // differ across platforms. This is constexpr and deterministic; readers
  // compute the same value and compare with the stored hash to detect
  // incompatibilities. We use FNV-1a 64-bit over the size-bytes.
  constexpr static std::uint64_t buildSizeHash() noexcept {
    constexpr std::array<std::uint8_t, 10> sizes = {
      static_cast<std::uint8_t>(sizeof(char)),
      static_cast<std::uint8_t>(sizeof(short)),
      static_cast<std::uint8_t>(sizeof(int)),
      static_cast<std::uint8_t>(sizeof(long)),
      static_cast<std::uint8_t>(sizeof(long long)),
      static_cast<std::uint8_t>(sizeof(float)),
      static_cast<std::uint8_t>(sizeof(double)),
      static_cast<std::uint8_t>(sizeof(long double)),
      static_cast<std::uint8_t>(sizeof(std::size_t)),
      static_cast<std::uint8_t>(sizeof(std::ptrdiff_t))};
    std::uint64_t size_hash = 14695981039346656037ULL;
    for (std::size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
      size_hash ^= static_cast<std::uint64_t>(sizes[i]);
      size_hash *= 1099511628211ULL;
    }
    return size_hash;
  }

  [[nodiscard]] std::string interpret() const noexcept {
    std::ostringstream ss;
    ss << "char is " << (m_fingerprint[0] ? "signed" : "unsigned") << "; ";
    if (m_fingerprint[1]) {
      ss << "wchar_t = 16-bit; ";
    } else if (m_fingerprint[2]) {
      ss << "wchar_t = 32-bit; ";
    } else {
      ss << "wchar_t = " << (sizeof(wchar_t) * 8) << "-bit; ";
    }
    ss << "size_t = " << (m_fingerprint[3] ? ">=64-bit" : "<64-bit") << "; ";
    ss << "ptrdiff_t = " << (m_fingerprint[4] ? ">=64-bit" : "<64-bit") << "; ";
    ss << "long = " << (m_fingerprint[5] ? ">=64-bit" : "<64-bit") << "; ";
    ss << "pointer = " << (m_fingerprint[6] ? ">=64-bit" : "<64-bit") << "; ";
    ss << "endianness = " << (m_fingerprint[7] ? "little" : "big") << "; ";
    ss << "wchar_t signed = " << (m_fingerprint[8] ? "true" : "false") << "; ";
    ss << "long long = " << (m_fingerprint[9] ? ">=64-bit" : "<64-bit");
    return ss.str();
  }
};


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
       << "\n"
       << "    timestamp: " << (header.m_flags.getTime() ? "enabled" : "disabled") << "\n"
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
