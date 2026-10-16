#ifndef FLATBUFDATASTREAM_H
#define FLATBUFDATASTREAM_H

#include <boost/asio/buffer.hpp>

template <typename Buffer>
class FlatBufWriter {
public:
    explicit FlatBufWriter(Buffer& buffer) : m_buffer(buffer) {}

    template <typename T>
    friend FlatBufWriter& operator<<(FlatBufWriter& stream, const T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "operator<< поддерживает только POD-типы");
        auto dst = stream.m_buffer.prepare(sizeof(T));
        boost::asio::buffer_copy(dst, boost::asio::buffer(&value, sizeof(T)));
        stream.m_buffer.commit(sizeof(T));
        return stream;
    }

    template <typename Container>
    requires std::ranges::range<Container>
    friend FlatBufWriter& operator<<(FlatBufWriter& stream, const Container& container) {
        stream << uint32_t(std::ranges::distance(container));
        for (const auto& item : container) stream << item;
        return stream;
    }

private:
    Buffer& m_buffer;
};

template <typename Buffer>
class FlatBufReader {
public:
    explicit FlatBufReader(Buffer& buffer) : m_buffer(buffer) {}

    template <typename T>
    friend FlatBufReader& operator>>(FlatBufReader& stream, T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "operator>> поддерживает только POD-типы");
        auto src = stream.m_buffer.data();
        boost::asio::buffer_copy(boost::asio::buffer(&value, sizeof(T)), src);
        stream.m_buffer.consume(sizeof(T));
        return stream;
    }

    // template <typename Container>
    // requires std::ranges::contiguous_range<Container>
    // && std::is_trivially_copyable_v<std::ranges::range_value_t<Container>>
    // && requires(Container& container, size_t n) { container.resize(n); }
    // friend BufReader& operator>>(BufReader& stream, Container& container) {
    //     size_t n = 0;
    //     stream >> n;
    //     container.resize(n);
    //     if (n > 0) {
    //         if (stream.m_buffer.size < n * sizeof(std::ranges::range_value_t<Container>))
    //             throw std::runtime_error("BufReader: мало данных");
    //         boost::asio::buffer_copy(
    //                     boost::asio::buffer(std::ranges::data(container)),
    //                     n * sizeof(std::ranges::range_value_t<Container>),
    //                     stream.m_buffer.data()
    //                     );
    //         stream.m_buffer.consume(n * sizeof(std::ranges::range_value_t<Container>));
    //     }
    // }

private:
    Buffer& m_buffer;
};

#endif // FLATBUFDATASTREAM_H
