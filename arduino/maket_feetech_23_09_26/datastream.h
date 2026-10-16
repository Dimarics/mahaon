#ifndef DATASTREAM_H
#define DATASTREAM_H

class SerialWriter {
public:
  explicit SerialWriter(Stream *serial) : m_serial(serial) {}

  template<typename T>
  friend SerialWriter& operator<<(SerialWriter& writer, const T& value) {
    uint8_t raw[sizeof(T)];
    memcpy(raw, &value, sizeof(T));
    for (auto &v : raw) writer.m_serial->write(v);
    return writer;
  }

  template<typename T, typename... Rest>
  void write(const T& first, const Rest&... rest) {
    *this << first;
    write(first, rest...);
}

private:
  Stream *m_serial;
};

class SerialReader {
public:
  explicit SerialReader(Stream *serial) : m_serial(serial) {}

  template<typename T>
  friend SerialReader& operator>>(SerialReader& reader, const T& value) {
    return reader;
  }

  

private:
  Stream *m_serial;
};

#endif  // DATASTREAM_H
