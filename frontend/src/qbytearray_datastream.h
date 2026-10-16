#ifndef QBYTEARRAY_DATASTREAM_H
#define QBYTEARRAY_DATASTREAM_H

#include <QByteArray>

class QByteArrayWriter {
public:
    explicit QByteArrayWriter(QByteArray& buffer) : m_buffer(buffer) {}

    // запись
    template <typename T>
    friend QByteArrayWriter& operator<<(QByteArrayWriter& writer, const T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "operator<< поддерживает только POD-типы");
        const auto old = writer.m_buffer.size();
        writer.m_buffer.resize(old + static_cast<int>(sizeof(T)));
        std::memcpy(writer.m_buffer.data() + old, &value, sizeof(T));
        return writer;
    }

    friend QByteArrayWriter& operator<<(QByteArrayWriter& writer, const QByteArray& s) {
        const uint32_t n = s.size();
        writer << n;
        const auto old = writer.m_buffer.size();
        writer.m_buffer.resize(old + s.size());
        std::memcpy(writer.m_buffer.data() + old, s.constData(), s.size());
        return writer;
    }

    friend QByteArrayWriter& operator<<(QByteArrayWriter& writer, const char* str) {
        return writer << QByteArray(str);
    }

private:
    QByteArray& m_buffer;
};

class QByteArrayReader {
public:
    explicit QByteArrayReader(const QByteArray& buffer) : m_offset(0), m_buffer(buffer) {}

    // template <typename T>
    // friend QByteArrayReader& operator>>(QByteArrayReader& reader, T& value) {
    //     static_assert(std::is_trivially_copyable_v<T>, "operator>> поддерживает только POD-типы");
    //     if (reader.m_buffer.size() < static_cast<int>(sizeof(T)))
    //         throw std::runtime_error("QByteArrayReader: недостаточно данных");
    //     std::memcpy(&value, reader.m_buffer.constData(), sizeof(T));
    //     reader.m_buffer.remove(0, static_cast<int>(sizeof(T)));
    //     return reader;
    // }

    // friend QByteArrayReader& operator>>(QByteArrayReader& reader, QByteArray& s) {
    //     quint32 n = 0;
    //     reader >> n;
    //     if (reader.m_buffer.size() < static_cast<int>(n))
    //         throw std::runtime_error("QByteArrayReader: недостаточно данных для строки");
    //     s = QByteArray(reader.m_buffer.constData(), static_cast<int>(n));
    //     reader.m_buffer.remove(0, static_cast<int>(n));
    //     return reader;
    // }

    template <typename T>
    friend QByteArrayReader& operator>>(QByteArrayReader& reader, T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "operator>> поддерживает только POD-типы");
        if (reader.remaining() < uint32_t(sizeof(T)))
            throw std::runtime_error("QByteArrayReader: недостаточно данных");
        std::memcpy(&value, reader.m_buffer.constData() + reader.m_offset, sizeof(T));
        reader.m_offset += sizeof(T);
        return reader;
    }

    friend QByteArrayReader& operator>>(QByteArrayReader& reader, QByteArray& s) {
        uint32_t n = 0;
        reader >> n;
        if (reader.remaining() < n)
            throw std::runtime_error("QByteArrayReader: недостаточно данных для строки");
        s = QByteArray(reader.m_buffer.constData() + reader.m_offset, n);
        reader.m_offset += n;
        return reader;
    }

    inline uint32_t remaining() const noexcept {
        return m_buffer.size() - m_offset;
    }

private:
    uint32_t m_offset;
    const QByteArray& m_buffer;
};

#endif // QBYTEARRAY_DATASTREAM_H
