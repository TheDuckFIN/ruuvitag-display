#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "ruuvitag-parser.h"


namespace RuuviTagScanner {
  void init();
  void start();
  void stop();
  void scanTask(void* parameter);

  // Class 
  class AdvertisedDeviceCallback : public BLEAdvertisedDeviceCallbacks {
    public:
      void onResult(BLEAdvertisedDevice advertisedDevice);
  };

  extern BLEScan* pBLEScan;
  extern TaskHandle_t scanTaskHandle;
}