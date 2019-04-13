#include <Arduino.h>

#include "ruuvitag-scanner.h"


void setup() {
  Serial.begin(115200);
  Serial.println("Initializing");

  RuuviTagScanner::init();
  RuuviTagScanner::start();
}

void loop() {
  delay(10000);
  RuuviTagScanner::stop();
}