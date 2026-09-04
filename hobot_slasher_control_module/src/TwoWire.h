#ifndef TWOWIRE_H
#define TWOWIRE_H

#include <cstdint>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

class TwoWire {
public:
    enum WIRE_ERROR_CODE {
        WIRE_SUCCESS         = 0,
        WIRE_ERROR_DATA      = 1,  // Данные слишком длинные
        WIRE_ERROR_NACK_ADDR = 2,  // Адрес не подтвержден (NACK)
        WIRE_ERROR_NACK_DATA = 3,  // Данные не подтверждены
        WIRE_ERROR_OTHER     = 4,  // Другая ошибка
        WIRE_ERROR_TIMEOUT   = 5,  // Таймаут
    };

    TwoWire();
    ~TwoWire();

    // Инициализация
    bool begin(int bus = 1);           // /dev/i2c-1 по умолчанию
    void end();

    // Основные методы (как в Arduino)
    void beginTransmission(uint8_t address);
    uint8_t endTransmission();
    uint8_t endTransmission(bool stop); // stop игнорируется в Linux, оставлен для совместимости

    size_t write(uint8_t data);
    size_t write(const uint8_t* data, size_t length);

    uint8_t requestFrom(uint8_t address, uint8_t quantity);
    uint8_t requestFrom(uint8_t address, uint8_t quantity, bool stop);
    int available();
    int read();
    uint8_t peek();

    // Дополнительные удобные методы
    bool isConnected(uint8_t address);
    void setClock(uint32_t frequency); // В Linux не поддерживается, но оставим заглушку

private:
    int _fd;                          // Файловый дескриптор I2C
    uint8_t _address;                 // Текущий адрес для передачи
    std::vector<uint8_t> _buffer;     // Буфер для данных
    std::vector<uint8_t> _rxBuffer;   // Буфер для принятых данных
    size_t _rxIndex;                  // Индекс чтения из rxBuffer
    bool _initialized;
    std::string _devicePath;

    // Вспомогательные методы
    uint8_t _handleError(int ret);
    // int _writeData(const uint8_t* data, size_t length);
    // int _readData(uint8_t* buffer, size_t length);
};

#endif // TWOWIRE_H
