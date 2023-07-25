// Bridge 
#include <Arduino.h>
#include "painlessMesh.h"
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Ticker.h>

#define     MESH_PREFIX       "whateverYouLike"
#define     MESH_PASSWORD     "somethingSneaky"
#define     MESH_PORT         5555
#define     DHTPIN            4
#define     DHTTYPE           DHT22
#define     RAINPIN           33

void receivedCallback( const uint32_t &from, const String &msg );

painlessMesh mesh;
Scheduler userScheduler;

BH1750 lightMeter;
DHT dht(DHTPIN, DHTTYPE);
Ticker myTicker;

float temp_tem, temp_hum, temp_lux;
int temp_rain;

void sendSensorData() { // Hàm này chạy mỗi 20s
    if (temp_tem != dht.readTemperature() || temp_hum != dht.readHumidity() || temp_rain != analogRead(RAINPIN) || temp_lux != lightMeter.readLightLevel()) {
        temp_tem = dht.readTemperature();
        temp_hum = dht.readHumidity();
        temp_rain = analogRead(RAINPIN);
        temp_lux = lightMeter.readLightLevel();
        String payload = "sensorstream|";
        payload += String(temp_tem) + "|";
        payload += String(temp_hum) + "|";
        payload += String(temp_rain) + "|";
        payload += String(temp_lux);
        Serial.println(payload); // Gửi dữ liệu sang RPIthông qua cổng USB
    }
}

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);

  Serial.print("Node: ");
  Serial.println(mesh.getNodeId());

  Wire.begin();
  lightMeter.begin();
  dht.begin();
  myTicker.attach(20, sendSensorData);
  temp_tem = dht.readTemperature();
  temp_hum = dht.readHumidity();
  temp_rain = analogRead(RAINPIN);
  temp_lux = lightMeter.readLightLevel();
}

void loop() {
  mesh.update();
  if (Serial.available()) {
    String income_mess = Serial.readStringUntil('/n');
    Serial.println(income_mess);
    if (income_mess == "give me data") {
      String payload = "sensordata|";
      payload += String(temp_tem) + "|";
      payload += String(temp_hum) + "|";
      payload += String(temp_rain) + "|";
      payload += String(temp_lux);
      Serial.println(payload);
    }
    if (income_mess == "toggle_water") {
      mesh.sendBroadcast("toggle_water");
    }
    if (income_mess == "toggle_light") {
      mesh.sendBroadcast("toggle_light");
    }
    if (income_mess == "take_picture") {
      mesh.sendBroadcast("take_picture");
    }
  }
}

void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.println(msg); // Gửi dữ liệu qua RPI thông qua USB
}

