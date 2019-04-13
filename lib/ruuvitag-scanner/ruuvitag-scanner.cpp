#include "ruuvitag-scanner.h"


using namespace RuuviTagScanner;

namespace RuuviTagScanner {
  BLEScan* pBLEScan = nullptr;
  TaskHandle_t scanTaskHandle = NULL;
}

void RuuviTagScanner::AdvertisedDeviceCallback::onResult(BLEAdvertisedDevice advertisedDevice) {
  byte* mData = (byte*)advertisedDevice.getManufacturerData().data();

  // Check for RuuviTag manufacturer ID
  if (mData[0] == 0x99 && mData[1] == 0x04) {
    Serial.printf("RuuviTag: %s \n", advertisedDevice.toString().c_str());
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
