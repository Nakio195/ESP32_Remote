#include <Arduino.h>

#include <SPI.h>
#define SS_PIN 5
#define PIN_MISO 19
#define PIN_SCK  18
#define PIN_MOSI 13

class AMT232
{
    public:
        AMT232();
        uint16_t read(uint8_t pin, bool* error);
        bool checksumAMT232(uint16_t data, bool k1, bool k0);

        uint16_t getPosition() const { return mPosition; }
        bool hasError() const { return mHasError; }

    private:
        int16_t mPosition;

        uint8_t mErrorCount;
        bool mHasError;
};