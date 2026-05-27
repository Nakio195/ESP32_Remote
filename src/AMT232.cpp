#include "AMT232.h"

AMT232::AMT232() : mPosition(0), mErrorCount(0), mHasError(false)
{
    pinMode(PIN_MISO, INPUT);
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_SCK, OUTPUT);
    pinMode(SS_PIN, OUTPUT);
    digitalWrite(SS_PIN, HIGH);

    SPI.begin(PIN_SCK, PIN_MISO, -1, SS_PIN);  // No MOSI
    SPI.setClockDivider(SPI_CLOCK_DIV16); // vitesse SPI (1 MHz env. à 16 MHz CPU)
    SPI.setDataMode(SPI_MODE3);
    SPI.setBitOrder(MSBFIRST);
}

uint16_t AMT232::read(uint8_t pin, bool* error)
{
    int16_t rx = 0;
	int16_t Data = 0;
    bool Checksum_K1 = 0;
    bool Checksum_K2 = 0;
    uint16_t txDummy = 0x0000;

    *error = false;

	digitalWrite(pin, LOW);
	rx = (int16_t)SPI.transfer16(txDummy);
    digitalWrite(pin, HIGH);

	Data = int16_t(rx & 0x3FFF) >> 2;
	Checksum_K1 = bool(rx & 0x8000);
	Checksum_K2 = bool(rx & 0x4000);
    Serial.print(Data);

    Data += -2484; //setting offset to center at 0
    Data = float(Data)/3.0;
    //Data *= 360.0/90.0; //scaling to degrees

    Serial.print(" : "); Serial.print(Data);
    Data = constrain(Data, -127, 127);

    
    Serial.print(" : "); Serial.println(Data);

	if (!checksumAMT232(rx, Checksum_K1, Checksum_K2))
		*error = true;

    if(!*error)
        mPosition = Data;
    
	return Data;

}

bool AMT232::checksumAMT232(uint16_t data, bool k1, bool k0)
{
    bool h5 = (data >> 13) & 1;
    bool h3 = (data >> 11) & 1;
    bool h1 = (data >> 9) & 1;
    bool l7 = (data >> 7) & 1;
    bool l5 = (data >> 5) & 1;
    bool l3 = (data >> 3) & 1;
    bool l1 = (data >> 1) & 1;
    bool h4 = (data >> 12) & 1;
    bool h2 = (data >> 10) & 1;
    bool h0 = (data >> 8) & 1;
    bool l6 = (data >> 6) & 1;
    bool l4 = (data >> 4) & 1;
    bool l2 = (data >> 2) & 1;
    bool l0 = (data >> 0) & 1;

    bool isValid = false;

    k1 == !(h5 ^ h3 ^ h1 ^ l7 ^ l5 ^ l3 ^ l1) ? isValid = true : isValid = false;
    k0 == !(h4 ^ h2 ^ h0 ^ l6 ^ l4 ^ l2 ^ l0) ? isValid = true : isValid = false;

    return isValid;
}