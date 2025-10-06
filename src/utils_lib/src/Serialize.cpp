/**
 * @file Serialize.cpp
 * @brief implementation for the Serializable class.
 * @date 01.10.2025
 * @author Jakob Wandel
 * @version 1.0
 **/


#include <utils/data/Serialize.hpp>


namespace serialize {


Header::Header(uint16_t id, uint16_t version, uint64_t size, int32_t checksum, int64_t timestamp)
    : checksum(checksum),
      id(id),
      version(version),
      size(size),
      timestamp(timestamp) {}
Header::Header(uint16_t id, uint16_t version, uint64_t size)
    : id(id),
      version(version),
      size(size) {}

bool Header::serialize(BinaryDataWriter& writer) const {
  return writer.writeNext(checksum) && writer.writeNext(id) &&
         writer.writeNext(version) && writer.writeNext(size) &&
         writer.writeNext(timestamp);
}

bool Header::deserialize(const BinaryDataReader& reader) {
  return reader.readNext(&checksum) && reader.readNext(&id) &&
         reader.readNext(&version) && reader.readNext(&size) &&
         reader.readNext(&timestamp);
}

Serializable::Serializable(uint16_t version, uint16_t id)
    : id(id),
      version(version) {}


bool Serializable::serialize(BinaryDataWriter& writer) const {
  const size_t cursor_before_header = writer.getCursor();

  Header header;
  if (!writer.writeNext(header)) {
    return false;
  }

  const size_t cursor_after_checksum = cursor_before_header + Header::CHECKSUM_BYTES;
  const size_t cursor_after_header = cursor_before_header + Header::BYTES;
  if (!writer.writeNext(header)) {
    return false;
  }

  if (!serializeClass(writer)) {
    return false;
  }
  const size_t cursor_after_class = writer.getCursor();

  if (!writer.setCursor(cursor_before_header)) {
    return false;
  }

  const std::vector<uint8_t>& buffer = writer.getBuffer();
  const std::span<const uint8_t> class_data{&buffer[cursor_after_checksum],
                                            &buffer[cursor_after_class]};
  const int32_t checksum = Header::calculateChecksum(class_data);
  const size_t size      = cursor_after_class - cursor_after_header;
  header = Header(id, version, static_cast<uint64_t>(size), checksum, Header::nowInMs());

  if (!writer.writeNext(header)) {
    return false;
  }

  return writer.setCursor(cursor_after_class);
}

bool Serializable::deserialize(const BinaryDataReader& reader) {
  Header header{0, 0, 0};
  if (!reader.readNext(&header)) {
    return false;
  }

  return deserialize(reader, header);
}

bool Serializable::deserialize(const BinaryDataReader& reader, const Header& header_deseriaized) {

  if (header_deseriaized.getId() != id) {
    return false;
  }

  if (header_deseriaized.getVersion() != version) {
    log::warningf(CURRENT_SOURCE_LOCATION,
                  "Deserialize Object No. {} from version {} to version {}",
                  id,
                  header_deseriaized.getVersion(),
                  version);
  }

  if (!reader.hasDataLeft(header_deseriaized.getSize())) {
    log::errorf(CURRENT_SOURCE_LOCATION,
                "Expected {} byts but got only {}",
                header_deseriaized.getSize(),
                reader.getNumUnreadBytes());
    return false;
  }

  const size_t cursor_before_class = reader.getCursor();
  if (!deserializeClass(reader)) {
    return false;
  }
  const size_t cursor_after_class = reader.getCursor();
  const uint64_t readBytes        = cursor_after_class - cursor_before_class;

  if (header_deseriaized.getSize() != static_cast<uint64_t>(readBytes)) {
    log::errorf(CURRENT_SOURCE_LOCATION,
                "Expected size {} does not match number of read bytes {}",
                header_deseriaized.getSize(),
                readBytes);
    return false;
  }

  const std::vector<uint8_t>& buffer = reader.getBuffer();
  const std::span<const uint8_t> class_data{&buffer[cursor_before_class],
                                            &buffer[cursor_after_class]};
  const int32_t checksum = Header::calculateChecksum(class_data);

  if (header_deseriaized.getChecksum() != checksum) {
    log::warning(CURRENT_SOURCE_LOCATION,
                 "The expected checksum does not match the calculated from the "
                 "received data.");
    return false;
  }
  return true;
}


}  // namespace serialize
