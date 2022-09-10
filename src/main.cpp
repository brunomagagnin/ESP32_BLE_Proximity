#include <Arduino.h>
#include <BLEDevice.h>

#define ADDRESS "ff:ff:c2:07:ab:16" //Endereço do iBeacon
#define RELAY_PIN 2 //Pino do Relê
#define SCAN_INTERVAL 1000 //intervalo entre cada scan
#define TARGET_RSSI -100 //RSSI limite de aproximação.
#define MAX_MISSING_TIME 3000 //Tempo para desligar o relê desde o momento que o iBeacon não for encontrado

BLEScan* pBLEScan; //Variável scan
uint32_t lastScanTime = 0; //Último scan
boolean found = false; //Se encontrou o iBeacon no último scan
uint32_t lastFoundTimeBeacon = 0; //Tempo em que encontrou o iBeacon pela última vez
int rssi = 0;

//Callback das chamadas ao scan
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        //Imprime dispositivo encontrado
        Serial.print("Device found: ");      
        Serial.println(advertisedDevice.toString().c_str());

        if(advertisedDevice.getAddress().toString() == ADDRESS)
        {
            //Pega o valor de RSSI
            found = true;
            advertisedDevice.getScan()->stop();
            rssi = advertisedDevice.getRSSI();
            Serial.println("RSSI: " + rssi);
        }
    }
};

void setup()
{
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    BLEDevice::init(""); 
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
}

void loop()
{   
    uint32_t now = millis(); //Tempo em milissegundos desde o boot

    if(found){ //Se econtrou o iBeacon no último scan
        lastFoundTimeBeacon = millis(); 
        found = false;
        
        if(rssi > TARGET_RSSI){ //Verifica faixa de RSSI
            digitalWrite(RELAY_PIN, HIGH);
        }
        else{ //senão desligamos
            digitalWrite(RELAY_PIN, LOW);
        }
    }
    //Desliga se o sinal for perdido
    else if(now - lastFoundTimeBeacon > MAX_MISSING_TIME){
        digitalWrite(RELAY_PIN, LOW);  //Desligamos o relê
    }
    
    if(now - lastScanTime > SCAN_INTERVAL){ //Se está no tempo de fazer scan
        //Marca quando ocorreu o último scan e começa o scan
        lastScanTime = now;
        pBLEScan->start(1);
    }
}