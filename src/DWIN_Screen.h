#ifndef DWIN_SCREEN_H
#define DWIN_SCREEN_H

#include <Arduino.h>
#include <SPI.h>

// Comandos DWIN
#define DWIN_FRAME_HEADER 0x5AA5
#define DWIN_WRITE 0x82
#define DWIN_READ 0x83

// Direcciones de memoria comunes
#define DWIN_VP_TEXT_BASE 0x1000  // Dirección base para texto

class DWINScreen {
private:
    SPIClass* spi;
    int8_t csPin;
    int8_t sckPin;
    int8_t mosiPin;
    int8_t misoPin;
    uint32_t spiFrequency;
    
    void beginTransaction();
    void endTransaction();
    void sendByte(uint8_t data);
    void sendWord(uint16_t data);
    uint8_t receiveByte();
    
public:
    // Constructor con pines por defecto del VSPI en ESP32
    // CS=5, SCK=18, MOSI=23, MISO=19
    DWINScreen(int8_t cs = 5, int8_t sck = 18, int8_t mosi = 23, int8_t miso = 19);
    
    // Constructor con SPIClass personalizada
    DWINScreen(SPIClass* spiInstance, int8_t cs);
    
    void begin(uint32_t frequency = 1000000); // 1MHz por defecto
    void writeText(uint16_t address, const char* text);
    void writeText(uint16_t address, String text);
    void clearTextArea(uint16_t address, uint8_t length);
    void writeVariable(uint16_t address, uint16_t value);
    void writeVariable(uint16_t address, int32_t value);
    void setBacklight(uint8_t brightness);
    uint16_t readVariable(uint16_t address);
};

#endif