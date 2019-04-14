#include "ruuvitag-parser.h"

void RuuviTagParser::parseV3(uint8_t* data) {
  double humidity = data[3] * 0.5;

  uint8_t tempWhole = data[4] & 0x7fff;
  uint8_t tempFraction = data[5];
  uint8_t tempSign = (data[4] & 0x8000) ? -1 : 1;
  double temperature = tempSign * (tempWhole + tempFraction / 100.0);

  uint32_t pressure = ((data[6] << 8) + data[7]) + 50000;

  // Accelerations and voltage not parsed as we do not need them.

  Serial.printf("Humidity: %f, temp: %f, pressure %i \n", humidity, temperature, pressure);
}
