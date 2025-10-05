#include "DWIN_Screen.h"

DWINScreen::DWINScreen(int8_t cs, int8_t sck, int8_t mosi, int8_t miso) {
    csPin = cs;
    sckPin = sck;
    mosiPin = mosi;
    misoPin = miso;
    spi = &SPI;
    spiFrequency = 1000000; // 1MHz por defecto
}

DWINScreen::DWINScreen(SPIClass* spiInstance, int8_t cs) {
    spi = spiInstance;
    csPin = cs;
    sckPin = -1;
    mosiPin = -1;
    misoPin = -1;
    spiFrequency = 1000000;
}

void DWINScreen::begin(uint32_t frequency) {
    spiFrequency = frequency;
    
    // Configurar pin CS
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    
    // Inicializar SPI
    if (sckPin >= 0 && mosiPin >= 0 && misoPin >= 0) {
        // Inicializar con pines personalizados
        spi->begin(sckPin, misoPin, mosiPin, csPin);
    } else {
        // Usar pines por defecto
        spi->begin();
    }
    
    delay(100);
}

void DWINScreen::beginTransaction() {
    // Configuración SPI: MSB primero, Modo 0 (CPOL=0, CPHA=0)
    spi->beginTransaction(SPISettings(spiFrequency, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);
    delayMicroseconds(1);
}

void DWINScreen::endTransaction() {
    delayMicroseconds(1);
    digitalWrite(csPin, HIGH);
    spi->endTransaction();
}

void DWINScreen::sendByte(uint8_t data) {
    spi->transfer(data);
}

void DWINScreen::sendWord(uint16_t data) {
    sendByte(data >> 8);
    sendByte(data & 0xFF);
}

uint8_t DWINScreen::receiveByte() {
    return spi->transfer(0xFF); // Enviar dummy byte para recibir
}

void DWINScreen::writeText(uint16_t address, const char* text) {
    uint8_t len = strlen(text);
    
    beginTransaction();
    
    // Header: 5A A5
    sendWord(DWIN_FRAME_HEADER);
    
    // Longitud de datos (comando + dirección + datos + terminador)
    sendByte(len + 4);
    
    // Comando de escritura
    sendByte(DWIN_WRITE);
    
    // Dirección VP
    sendWord(address);
    
    // Enviar texto
    for (uint8_t i = 0; i < len; i++) {
        sendByte(text[i]);
    }
    
    // Agregar terminador nulo
    sendByte(0x00);
    
    endTransaction();
    delayMicroseconds(100);
}

void DWINScreen::writeText(uint16_t address, String text) {
    writeText(address, text.c_str());
}

void DWINScreen::clearTextArea(uint16_t address, uint8_t length) {
    beginTransaction();
    
    // Header
    sendWord(DWIN_FRAME_HEADER);
    
    // Longitud
    sendByte(length + 3);
    
    // Comando
    sendByte(DWIN_WRITE);
    
    // Dirección
    sendWord(address);
    
    // Llenar con espacios
    for (uint8_t i = 0; i < length; i++) {
        sendByte(' ');
    }
    
    endTransaction();
    delayMicroseconds(100);
}

void DWINScreen::writeVariable(uint16_t address, uint16_t value) {
    beginTransaction();
    
    // Header
    sendWord(DWIN_FRAME_HEADER);
    
    // Longitud (comando + dirección + 2 bytes de datos)
    sendByte(5);
    
    // Comando
    sendByte(DWIN_WRITE);
    
    // Dirección
    sendWord(address);
    
    // Valor (16 bits)
    sendWord(value);
    
    endTransaction();
    delayMicroseconds(100);
}

void DWINScreen::writeVariable(uint16_t address, int32_t value) {
    beginTransaction();
    
    // Header
    sendWord(DWIN_FRAME_HEADER);
    
    // Longitud (comando + dirección + 4 bytes de datos)
    sendByte(7);
    
    // Comando
    sendByte(DWIN_WRITE);
    
    // Dirección
    sendWord(address);
    
    // Valor (32 bits) - Big endian
    sendByte((value >> 24) & 0xFF);
    sendByte((value >> 16) & 0xFF);
    sendByte((value >> 8) & 0xFF);
    sendByte(value & 0xFF);
    
    endTransaction();
    delayMicroseconds(100);
}

void DWINScreen::setBacklight(uint8_t brightness) {
    // Dirección típica del backlight: 0x0082
    writeVariable(0x0082, (uint16_t)brightness);
}

uint16_t DWINScreen::readVariable(uint16_t address) {
    uint16_t value = 0;
    
    beginTransaction();
    
    // Header
    sendWord(DWIN_FRAME_HEADER);
    
    // Longitud (comando + dirección + longitud a leer)
    sendByte(4);
    
    // Comando de lectura
    sendByte(DWIN_READ);
    
    // Dirección
    sendWord(address);
    
    // Longitud a leer (1 word = 2 bytes)
    sendByte(0x01);
    
    endTransaction();
    
    // Esperar respuesta
    delay(10);
    
    // Leer respuesta
    beginTransaction();
    
    // Saltar header de respuesta (5A A5)
    receiveByte();
    receiveByte();
    
    // Saltar longitud
    receiveByte();
    
    // Leer valor
    value = (uint16_t)receiveByte() << 8;
    value |= receiveByte();
    
    endTransaction();
    
    return value;
}