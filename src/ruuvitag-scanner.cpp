#include "ruuvitag-scanner.h"


using namespace RuuviTagScanner;

namespace RuuviTagScanner {
  BLEScan* pBLEScan = nullptr;
  TaskHandle_t scanTaskHandle = NULL;
}

void RuuviTagScanner::AdvertisedDeviceCallback::onResult(BLEAdvertisedDevice advertisedDevice) {
  uint8_t* mData = (uint8_t*)advertisedDevice.getManufacturerData().data();

  // Funny thing: advertisedDevice.getAddress().toString() changes the content
  // of mData. Why? I don't know... Therefore, it's only called after we're
  // already done with parsing manufacturer data :) Fought with this issue
  // for over 3 hours before realizing what was happening.

  // To measure task stack usage, uncomment below.
  // Serial.println(uxTaskGetStackHighWaterMark(scanTaskHandle));

  // Check for RuuviTag manufacturer ID
  if (mData[0] == 0x99 && mData[1] == 0x04) {

    // Protocol version 3
    if (mData[2] == 0x03) {
      RuuviTagParser::parseV3(mData);
      const char* address = advertisedDevice.getAddress().toString().c_str();
      Serial.printf("Found RuuviTag %s with protocol v3\n", address);
    } else {
      // mData gets screwed up after we call address.toString().c_str(), therefore
      // we have to get the protocol version safe before calling it :)
      char version = mData[2];
      const char* address = advertisedDevice.getAddress().toString().c_str();
      Serial.printf("Found RuuviTag %s with unsupported protocol v%i\n", address, version);
    }
  }
}

void RuuviTagScanner::init() {
  if (pBLEScan != nullptr) return;

  // Initialize bluetooth device with empty name. Name is not really needed
  // as we will not advertise ourself.
  BLEDevice::init("");

  // BLEScan object is a singleton, so let's get it and store to a variable
  pBLEScan = BLEDevice::getScan();

  // Set callback which is called when an advertised device is discovered.
  // Second parameter enabled callback for duplicates, therefore allowing us
  // to hear updates from the RuuviTags.
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallback(), true);
  pBLEScan->setActiveScan(true);

  // Scan for 1 second window, every 2 seconds.
  pBLEScan->setWindow(1000);
  pBLEScan->setInterval(2000);
}

void RuuviTagScanner::scanTask(void* params) {
  // Use endless loop in order to restart Bluetooth scan if it stops for some reason.
  while(true) {
    Serial.println("Starting Bluetooth scan");
    pBLEScan->start(0, false);
    Serial.println("Bluetooth scan ended");
  }
}

void RuuviTagScanner::start() {
  Serial.println("Creating a new task for bluetooth scanning");

  // BLEScan::start is a blocking function, therefore let's start a task
  // for it so it runs in the background.
  // Params: task function, name, stack size, parameter, priority, task handle
  if (xTaskCreate(scanTask, "ScanTask", 10000, NULL, 1, &scanTaskHandle) != pdPASS) {
    Serial.println("Failed to create the task");
  }
}

void RuuviTagScanner::stop() {
  if (scanTaskHandle == NULL) {
    Serial.println("Failed to stop bluetooth scanning task: not running");
  } else {
    // Kill the scanning task
    vTaskDelete(scanTaskHandle);
    scanTaskHandle = NULL;

    // Stop ongoing scan
    pBLEScan->stop();

    Serial.println("Successfully stopped bluetooth scanning task");
  }
}
