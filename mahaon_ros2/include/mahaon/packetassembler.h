#ifndef PACKETASSEMBLER_H
#define PACKETASSEMBLER_H

#include <cstdint>
// #include <iostream>
#include <vector>
#include <functional>
#include "buf_datastream.h"

/*
 * struct:
 * 1 - start byte first
 * 2 - start byte second
 * 3 - function code
 * 4 - packet size
 * 5-n - data
 * n+1 - crc
 */

class PacketAssembler
{
public:
    PacketAssembler();
    void addData(const std::vector<uint8_t> &data);
    inline void onPacketAssembled(const std::function<void(uint8_t, const std::vector<uint8_t>&)> &callback) {
        m_packetAssembledCallback = callback;
    }

    // template <typename Buffer, typename... Args>
    // std::vector<uint8_t> buildPacket(uint8_t cmd, const Args&... args) {
    //     uint8_t crc = cmd, size = 0;
    //     std::vector<uint8_t> buffer {0xFF, 0xFF, cmd};
    //     // BufWriter writer(buffer);
    //     // writer << uint16_t(0xFFFF) << cmd << (sizeof(args) + ... + 0);
    //     // (writer << ... << args);
    //     auto addArg = [&crc, &size, &buffer](const auto& arg) {
    //         size += sizeof(arg);
    //         const auto pos = buffer.size();
    //         buffer.resize(pos + sizeof(arg));
    //         for (size_t i = 0; i < sizeof(arg); ++i) {
    //             uint8_t byte = reinterpret_cast<uint8_t*>(&arg)[i];
    //             crc += byte;
    //             buffer[size + i] = byte;
    //         }
    //     };
    //     (addArg(args), ...);
    // }

private:
    bool m_startByteFirstRead;
    bool m_startByteSecondRead;
    bool m_functionCodeRead;
    uint8_t m_functionCode;
    bool m_dataSizeRead;
    uint8_t m_dataSize;
    std::vector<uint8_t> m_data;
    std::function<void(uint8_t, const std::vector<uint8_t>&)> m_packetAssembledCallback;
};

template <typename Buffer>
class PacketBuilder : public BufWriter<Buffer>
{
public:
    using BufWriter<Buffer>::operator<<;
    PacketBuilder(Buffer &buffer) : BufWriter<Buffer>(buffer) {}
    void begin() {
        *this << uint8_t(0xFF) << uint8_t(0xFF);
    }
    void end() {
        uint8_t crc = 0;
        this->m_buffer.insert(this->m_buffer.begin() + 3, this->m_buffer.size() - 3);
        for (size_t i = 2; i < this->m_buffer.size(); ++i) crc += this->m_buffer.at(i);
        *this << crc;
    }
};

#endif // SERIALPORT_H
