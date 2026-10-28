#include "TwoWire.h"


#if defined(__arm__) || defined(__aarch64__)
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#endif

#include <iostream>
#include <sys/stat.h>

TwoWire::TwoWire() : _fd(-1), _address(0), _rxIndex(0), _initialized(false), _devicePath("/dev/i2c-1") {
    _buffer.reserve(32);
    _rxBuffer.reserve(32);
}

TwoWire::~TwoWire() {
    end();
}

bool TwoWire::begin(int bus) {

    if (_initialized) {
        end();
    }

    _devicePath = "/dev/i2c-" + std::to_string(bus);

    _fd = open(_devicePath.c_str(), O_RDWR);
    // std::cout << "fd: " << _fd << std::endl;
    if (_fd < 0) {
        std::cerr << "TwoWire: Не удалось открыть " << _devicePath
                  << " (ошибка: " << strerror(errno) << ")" << std::endl;
        return false;
    }

    _initialized = true;
    return true;
}

void TwoWire::end() {
    if (_fd >= 0) {
        close(_fd);
        _fd = -1;
    }
    _initialized = false;
    _buffer.clear();
    _rxBuffer.clear();
    _rxIndex = 0;
}

// ===================== ОСНОВНЫЕ МЕТОДЫ =====================

void TwoWire::beginTransmission(uint8_t address) {
    if (!_initialized) return;
    _address = address;
    _buffer.clear();
}

uint8_t TwoWire::endTransmission() {
#if defined(__arm__) || defined(__aarch64__)
    if (!_initialized) return 4;  // WIRE_ERROR_OTHER
    // Если буфер пуст, просто проверяем наличие устройства
    if (_buffer.empty()) {
        // Отправляем пустое сообщение для проверки
        struct i2c_msg msg = {
            .addr = _address,
                    .flags = 0,
                    .len = 0,
                    .buf = nullptr
        };
        struct i2c_rdwr_ioctl_data transfer = {
            .msgs = &msg,
                    .nmsgs = 1
        };
        int ret = ioctl(_fd, I2C_RDWR, &transfer);
        return _handleError(ret);
    }

    // Отправляем данные
    struct i2c_msg msg = {
        .addr = _address,
                .flags = 0,
                .len = static_cast<uint16_t>(_buffer.size()),
                .buf = _buffer.data()
    };
    struct i2c_rdwr_ioctl_data transfer = {
        .msgs = &msg,
                .nmsgs = 1
    };

    int ret = ioctl(_fd, I2C_RDWR, &transfer);
    return _handleError(ret);
#else
    return 0;
#endif
}

uint8_t TwoWire::endTransmission(bool) {
    return endTransmission();
}

size_t TwoWire::write(uint8_t data) {
    if (!_initialized) return 0;
    _buffer.push_back(data);
    return 1;
}

size_t TwoWire::write(const uint8_t* data, size_t length) {
    if (!_initialized) return 0;
    _buffer.insert(_buffer.end(), data, data + length);
    return length;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
#if defined(__arm__) || defined(__aarch64__)
    if (!_initialized || quantity == 0) return 0;

    _rxBuffer.clear();
    _rxBuffer.resize(quantity);

    struct i2c_msg msg = {
        .addr = address,
                .flags = I2C_M_RD,
                .len = static_cast<uint16_t>(quantity),
                .buf = _rxBuffer.data()
    };
    struct i2c_rdwr_ioctl_data transfer = {
        .msgs = &msg,
                .nmsgs = 1
    };

    int ret = ioctl(_fd, I2C_RDWR, &transfer);

    if (ret == 1) {  // Успех
        _rxIndex = 0;
        return quantity;  // Возвращаем количество запрошенных байт
    }

    // Ошибка - очищаем буфер
    _rxBuffer.clear();
    return 0;
#else
    (void)address; (void)quantity;
    return 0;
#endif
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, bool) {
    return requestFrom(address, quantity);
}

int TwoWire::available() {
    if (!_initialized) return 0;
    return _rxBuffer.size() - _rxIndex;
}

int TwoWire::read() {
    if (!_initialized || _rxIndex >= _rxBuffer.size()) {
        return -1;
    }
    return _rxBuffer[_rxIndex++];
}

uint8_t TwoWire::peek() {
    if (!_initialized || _rxIndex >= _rxBuffer.size()) {
        return 0;
    }
    return _rxBuffer[_rxIndex];
}

// ===================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ =====================

bool TwoWire::isConnected(uint8_t address) {
#if defined(__arm__) || defined(__aarch64__)
    if (!_initialized) return false;

    struct i2c_msg msg = {
        .addr = address,
                .flags = 0,
                .len = 0,
                .buf = nullptr
    };

    struct i2c_rdwr_ioctl_data transfer = {
        .msgs = &msg,
                .nmsgs = 1
    };

    return ioctl(_fd, I2C_RDWR, &transfer) == 1;
#else
    (void)address;
    return false;
#endif
}

void TwoWire::setClock(uint32_t frequency) {
    // В Linux стандартный I2C драйвер не позволяет менять частоту динамически
    // Это можно сделать через device tree или config.txt
    // Оставляем заглушку для совместимости
    (void)frequency;
}

// ===================== ПРИВАТНЫЕ МЕТОДЫ =====================

// int TwoWire::_writeData(const uint8_t* data, size_t length) {
// #if defined(__arm__) || defined(__aarch64__)
//     struct i2c_msg msg = {
//         .addr = _address,
//                 .flags = 0,                  // Запись
//                 .len = static_cast<uint16_t>(length),
//                 .buf = const_cast<uint8_t*>(data)
//     };

//     struct i2c_rdwr_ioctl_data transfer = {
//         .msgs = &msg,
//                 .nmsgs = 1
//     };

//     int ret = ioctl(_fd, I2C_RDWR, &transfer);
//     if (ret == 1) {
//         return length;
//     }
//     return ret;
// #else
//     return -1;
// #endif
// }

// int TwoWire::_readData(uint8_t* buffer, size_t length) {
// #if defined(__arm__) || defined(__aarch64__)
//     struct i2c_msg msg = {
//         .addr = _address,
//                 .flags = I2C_M_RD,           // Чтение
//                 .len = static_cast<uint16_t>(length),
//                 .buf = buffer
//     };

//     struct i2c_rdwr_ioctl_data transfer = {
//         .msgs = &msg,
//                 .nmsgs = 1
//     };

//     int ret = ioctl(_fd, I2C_RDWR, &transfer);
//     if (ret == 1) {
//         return length;
//     }
//     return ret;
// #else
//     return -1;
// #endif
// }

uint8_t TwoWire::_handleError(int ret) {
    if (ret == 1) {
        return WIRE_SUCCESS;
    }

    if (ret == -1) {
        switch (errno) {
        case ENXIO:      // Нет такого устройства или адреса
#if defined(__arm__) || defined(__aarch64__)
        case EREMOTEIO:
#endif
            return WIRE_ERROR_NACK_ADDR;
        case EIO:        // Ошибка ввода-вывода (обычно NACK)
            return WIRE_ERROR_NACK_DATA;
        case ETIMEDOUT:  // Таймаут
            return WIRE_ERROR_TIMEOUT;
        default:
            return WIRE_ERROR_OTHER;
        }
    }

    return WIRE_ERROR_OTHER;
}
