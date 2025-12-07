/**
 * @file Serialize.cpp
 * @brief implementation for the Serializable class.
 * @date 01.10.2025
 * @author Jakob Wandel
 * @m_version 1.0
 **/


#include <cstdint>
#include <cstddef>
#include <span>
#include <utils/data/Serialize.hpp>
#include <utils/debug/logging.hpp>
#include "utils/data/BinaryDataWriter.hpp"
#include "utils/data/BinaryDataReader.hpp"
#include <vector>


namespace serialize {

bool Flags::serialize(BinaryDataWriter& writer) const {
  return writer.writeNext(m_flags);
}

bool Flags::deserialize(const BinaryDataReader& reader) {
  return reader.readNext(&m_flags);
}


Header::Header(uint16_t id, uint8_t version, uint64_t size, Flags flags, int32_t checksum, int64_t timestamp)
    : m_checksum(checksum),
      m_id(id),
      m_version(version),
      m_flags(flags),
      m_size(size),
      m_timestamp(timestamp) {}
Header::Header(uint16_t id, uint8_t version, uint64_t size, Flags flags)
    : Header(id, version, size, flags, NO_CHECKSUM, NO_TIMESTAMP) {}

bool Header::serialize(BinaryDataWriter& writer) const {
  return writer.writeNext(m_checksum) && writer.writeNext(m_id) &&
         writer.writeNext(m_version) && writer.writeNext(m_flags) &&
         writer.writeNext(m_size) && writer.writeNext(m_timestamp);
}

bool Header::deserialize(const BinaryDataReader& reader) {
  return reader.readNext(&m_checksum) && reader.readNext(&m_id) &&
         reader.readNext(&m_version) && reader.readNext(&m_flags) &&
         reader.readNext(&m_size) && reader.readNext(&m_timestamp);
}


Serializable::Serializable(uint8_t version, uint16_t id)
    : m_id(id),
      m_version(version) {}


bool Serializable::serialize(BinaryDataWriter& writer) const {
  const size_t cursor_before_header = writer.getCursor();
  const size_t cursor_after_checksum = cursor_before_header + Header::CHECKSUM_BYTES;
  const size_t cursor_after_header = cursor_before_header + Header::BYTES;

  // we write the header after the class is beeing written, since we need to calculate the checksum.
  if (!writer.setCursor(cursor_after_header)) {
    return false;
  }

  if (!serializeClass(writer)) {
    return false;
  }
  const size_t cursor_after_class = writer.getCursor();
  const size_t class_size         = cursor_after_class - cursor_after_header;

  if (!writer.setCursor(cursor_before_header)) {
    return false;
  }

  serialize::Flags flags;
  // set endian flag from writer endian
  flags.setEndian(writer.getEndian());
  // enable checksum and timestamp by default
  flags.setControlHash(true);
  flags.setTime(true);
  const Header header{
    m_id, m_version, static_cast<uint64_t>(class_size), flags, Header::NO_CHECKSUM, Header::nowInMs()};

  if (!writer.writeNext(header)) {
    return false;
  }

  if (flags.getControlHash()) {
    const std::span<const uint8_t> class_buffer =
      writer.getBuffer(cursor_after_checksum, class_size);
    const int32_t checksum = Header::calculateChecksum(class_buffer);

    if (!writer.setCursor(cursor_before_header)) {
      return false;
    }
    if (!writer.writeNext(checksum)) {
      return false;
    }
  }

  return writer.setCursor(cursor_after_class);
}

bool Serializable::deserialize(const BinaryDataReader& reader) {
  Header header;
  if (!reader.readNext(&header)) {
    return false;
  }

  return deserialize(reader, header);
}

std::optional<Header> Serializable::deserializeHeader(const BinaryDataReader& reader) {
  Header header;
  if (!reader.readNext(&header)) {
    return {};
  }
  return header;
}

bool Serializable::deserialize(const BinaryDataReader& reader, const Header& header_deseriaized) {

  if (header_deseriaized.getEndian() != reader.getEndian()) {
    dbg::errorf(
      CURRENT_SOURCE_LOCATION,
      "The endian set in the header is not the endian set in the reader.",
      header_deseriaized.getSize(),
      reader.getNumUnreadBytes());
    return false;
  }

  if (header_deseriaized.getVersion() != m_version) {
    dbg::warningf(CURRENT_SOURCE_LOCATION,
                  "Deserialize Object No. {} from version {} to version {}",
                  m_id,
                  header_deseriaized.getVersion(),
                  m_version);
  }

  if (header_deseriaized.getId() != m_id) {
    dbg::errorf(CURRENT_SOURCE_LOCATION,
                "Serializing wrong Object. Expected Object Id from header: {} "
                "current Object id: {}",
                header_deseriaized.getId(),
                m_id);
    return false;
  }

  if (!reader.hasDataLeft(header_deseriaized.getSize())) {
    dbg::errorf(CURRENT_SOURCE_LOCATION,
                "Expected {} byts but got only {}",
                header_deseriaized.getSize(),
                reader.getNumUnreadBytes());
    return false;
  }

  const size_t cursor_before_class = reader.getCursor();

  if (!deserializeClass(reader)) {
    dbg::errorf(
      CURRENT_SOURCE_LOCATION, "Deserialization Error for Class with id: {}", m_id);
    return false;
  }


  const size_t cursor_after_class   = reader.getCursor();
  const uint64_t readBytes          = cursor_after_class - cursor_before_class;
  const size_t cursor_before_header = cursor_before_class - Header::BYTES;
  const size_t cursor_after_checksum = cursor_before_header + Header::CHECKSUM_BYTES;

  if (header_deseriaized.getSize() != readBytes) {
    dbg::errorf(CURRENT_SOURCE_LOCATION,
                "Expected size {} does not match number of read bytes {}",
                header_deseriaized.getSize(),
                readBytes);
    return false;
  }

  if (!header_deseriaized.getFlags().getControlHash()) {
    return true;
  }

  const std::span<const uint8_t> class_data =
    reader.getBuffer(cursor_after_checksum, readBytes);
  const int32_t checksum = Header::calculateChecksum(class_data);

  if (header_deseriaized.getChecksum() != checksum) {
    dbg::warning(CURRENT_SOURCE_LOCATION,
                 "The expected checksum does not match the calculated from the "
                 "received data.");
    return false;
  }
  return true;
}


}  // namespace serialize
