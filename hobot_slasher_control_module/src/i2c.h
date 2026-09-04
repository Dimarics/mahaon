#ifndef I2C_H
#define I2C_H

#include <cstdint>
#include <cstddef>

#ifdef __arm__

#include "fcntl.h"
#include "unistd.h"
#include "linux/i2c-dev.h"
#include "sys/ioctl.h"
#include <QDebug>

class I2C
{
public:
    I2C(const char *device = "/dev/i2c-1") : m_device(device) {
        begin();
    }
    bool begin() {
        m_fileDescriptor = open(m_device, O_RDWR);
        if (m_fileDescriptor < 0) {
            qDebug() << "file not open";
            return false;
        }
        return true;
    }
    bool writeBytes(uint8_t address, uint8_t *data, size_t size) {
        if (ioctl(m_fileDescriptor, I2C_SLAVE, address) < 0) {
            qDebug() << "ioctl error";
            return false;
        }
        if (write(m_fileDescriptor, data, size) != -1) {
            return true;
        }
        return false;
    }
    bool writeByte(uint8_t address, uint8_t byte) {
        return writeBytes(address, &byte, 1);
    }
    bool readBytes(uint8_t address, uint8_t *data, size_t size) {
        if (ioctl(m_fileDescriptor, I2C_SLAVE, address) < 0) {
            qDebug() << "ioctl error";
            return false;
        }
        if (read(m_fileDescriptor, data, size) != -1) {
            return true;
        }
        return false;
    }

private:
    const char *m_device;
    int m_fileDescriptor;
};

#else

class I2C
{
public:
    size_t t;
    I2C(const char *device = "") : m_device(device) {}
    bool begin() { return false; }
    bool writeBytes(uint8_t address, uint8_t *data, size_t size) { return false; }
    bool writeByte(uint8_t address, uint8_t byte) { return false; }
    bool readBytes(uint8_t address, uint8_t *data, size_t size) { return false; }
    const char *m_device;
};

#endif // __arm__

#endif // I2C_H
