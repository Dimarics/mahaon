#ifndef BUFDATASTREAM_H
#define BUFDATASTREAM_H

#include <ranges>
#include <cstring>

template <typename Buffer>
class BufWriter {
public:
    explicit BufWriter(Buffer& buffer) : m_buffer(buffer) {}

    template <typename T>
    BufWriter& operator<<(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "operator<< поддерживает только POD-типы");
        const auto pos = m_buffer.size();
        m_buffer.resize(pos + sizeof(T));
        std::memcpy(m_buffer.data() + pos, &value, sizeof(T));
        return *this;
    }

    template <typename Container>
    requires std::ranges::range<Container>
    BufWriter& operator<<(const Container& container) {
        *this << uint32_t(std::ranges::distance(container));
        for (const auto& item : container) *this << item;
        return *this;
    }

protected:
    Buffer& m_buffer;
};

template <typename Buffer>
class BufReader {
public:
    explicit BufReader(Buffer& buffer) : m_buffer(buffer) {}

    template <typename T>
    BufReader& operator>>(T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "operator>> поддерживает только POD-типы");
        if (m_buffer.size() < sizeof(T))
            throw std::runtime_error("BufReader: недостаточно данных");
        std::memcpy(&value, m_buffer.data(), sizeof(T));
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + sizeof(T));
        return *this;
    }

protected:
    Buffer& m_buffer;
};

#endif // BUFDATASTREAM_H
