#include "mahaon/packetassembler.h"
// #include <iostream>

PacketAssembler::PacketAssembler()
    : m_startByteFirstRead(0)
    , m_startByteSecondRead(0)
    , m_functionCodeRead(0)
    , m_functionCode(0)
    , m_dataSizeRead(0)
    , m_dataSize(0) { }

void PacketAssembler::addData(const std::vector<uint8_t> &data) {
    // std::cout << "addData" << std::endl;
    for (auto &byte : data) {
        if (!m_startByteFirstRead) {
            if (byte == 0xFF) {
                m_startByteFirstRead = true;
                // std::cout << "m_startByteFirstRead" << std::endl;
            }
            continue;
        }
        if (!m_startByteSecondRead) {
            if (byte == 0xFF) {
                m_startByteSecondRead = true;
                // std::cout << "m_startByteSecondRead" << std::endl;
            } else {
                m_startByteFirstRead = false;
            }
            continue;
        }
        if (!m_functionCodeRead) {
            m_functionCodeRead = true;
            m_functionCode = byte;
            // std::cout << "m_functionCodeRead: " << int(m_functionCode) << std::endl;
            continue;
        }
        if (!m_dataSizeRead) {
            m_dataSizeRead = true;
            m_dataSize = byte;
            // std::cout << "m_dataSizeRead: " << int(m_dataSize) << std::endl;
            continue;
        }
        if (m_data.size() < m_dataSize) {
            m_data.push_back(byte);
            continue;
        }
        // std::cout << "check sum: " << std::endl;

        uint8_t crc = m_functionCode + m_dataSize;
        for (auto &b : m_data) crc += b;
        if (crc == byte) {
            m_packetAssembledCallback(m_functionCode, m_data);
        } else {
            // std::cout << "Ошибка контрольной суммы" << std::endl;
        }
        m_startByteFirstRead = false;
        m_startByteSecondRead = false;
        m_functionCodeRead = false;
        m_dataSizeRead = false;
        m_data.clear();
    }
}
