#include <Arduino.h>
#include <BLEDevice.h>
#include <Wire.h>
#include "../lib\LiquidCrystal_I2C\LiquidCrystal_I2C.h"

#define RELAY_PIN 2           // Relay pin
#define SCAN_INTERVAL 1000    // interval between each scan
#define TARGET_RSSI -80       // RSSI approach limit
#define MAX_MISSING_TIME 4000 // Time to turn off the relay

LiquidCrystal_I2C lcd(0x27, 16, 2);

std::string addressBLE[2] = {"e9:48:89:f8:05:88", "c2:a9:c7:96:d8:7b"};
String name[2] = {"Person 1  ", "Person 2  "};
bool markerOfFoundBeacon[2] = {false, false};
uint32_t counter = 0;
uint32_t scanTimeFoundBeacon[2] = {0, 0};

BLEScan *pBLEScan;                // variable scan
uint32_t lastScanTime = 0;        //last scan
boolean found = false;            // If you found the iBeacon in the last scan
uint32_t lastFoundTimeBeacon = 0; // Last time iBeacon was found
int rssi;


// Callback from calls to scan
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {        
        // Check which beacon was found
        for (int i = 0; i < sizeof(addressBLE); i++)
        {
            if (advertisedDevice.getAddress().toString() == addressBLE[i])
            {
                scanTimeFoundBeacon[i] = millis();
                found = true;
                return;
            }
            else
            {
                found = false;
            }
        }
        if (found)
        {
            // Get the RSSI value
            advertisedDevice.getScan()->stop();
            rssi = advertisedDevice.getRSSI();
        }
    }
};

void setup()
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    lcd.begin();
    lcd.clear();

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);

    lcd.setCursor(0, 0);
    lcd.print("Starting...");
    delay(3000);
}

void loop()
{
    uint32_t now = millis();
    if (found && rssi > TARGET_RSSI)
    {
        lastFoundTimeBeacon = millis();
        found = false;
        digitalWrite(RELAY_PIN, HIGH);
        counter = 0;
        for (int i = 0; i < sizeof(markerOfFoundBeacon); i++)
        {
            if (markerOfFoundBeacon[i])
            {
                lcd.setCursor(0, counter);
                lcd.print(name[i]);
                if (i == 1 && counter == 0)
                {
                    lcd.setCursor(0, 1);
                    lcd.print("               ");
                }
                counter++;
            }
        }
        if (counter == 1 && markerOfFoundBeacon[0])
        {
            lcd.setCursor(0, 1);
            lcd.print("               ");
        }
    }
    // Turns off if signal is lost
    else if (now - lastFoundTimeBeacon > MAX_MISSING_TIME)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Searching...");
        digitalWrite(RELAY_PIN, LOW);
    }

    for (int i = 0; i < sizeof(markerOfFoundBeacon); i++)
    {
        if (now - scanTimeFoundBeacon[i] > MAX_MISSING_TIME)
        {
            markerOfFoundBeacon[i] = false;
        }
        else
        {
            markerOfFoundBeacon[i] = true;
        }
    }

    //Scan Time
    if (now - lastScanTime > SCAN_INTERVAL) 
    {
        lastScanTime = now;
        pBLEScan->start(1);
    }
}