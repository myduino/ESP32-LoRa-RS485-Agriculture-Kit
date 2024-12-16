/*
 * Agriculture Kit
 * Author: Mohamad Ariffin Zulkifli
 * 
 * This is an example sketch for Agricluture Kit
 * Read the Soil parameters from soil sensor using RS485
 * Print the measure soil on Serial Monitor.
 * Show the measured soil parameters on the OLED display.
 * 
 * Purchase?
 * Shopee - https://shopee.com.my/Agriculture-Kit-ESP32-LoRa-915-MHz-RS485-Modbus-Controller-with-Industrial-Grade-Soil-Temperature-Humidity-NPK-Sensor-i.132184430.15335468998
 * 
 */

#include <SoftwareSerial.h>

// Sensor Pin Definition
#define RS485Power                  18

// If you are using different Development Board,
// do change the RX, TX pin appropriately
#define RS485RX                     23
#define RS485TX                     22

SoftwareSerial sensor(RS485RX, RS485TX);

#define sensorFrameSize             11
#define sensorWaitingTime           1000
#define sensorID                    0x01
#define sensorFunction              0x03
#define sensorByteResponse          0x06

// RS485 Byte Address Request to Sensor
unsigned char byteRequest[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB};
unsigned char byteResponse[11] = {};

float moisture, temperature;
int ec;

unsigned long lastMillis = 0;

void setup() {
  // Debug console
  Serial.begin(115200);
  sensor.begin(4800);

  pinMode(RS485Power, OUTPUT);
  digitalWrite(RS485Power, HIGH);

  Serial.println();
  Serial.println("Agriculture Kit Sensor Ready");
  Serial.println("Soil Temperature, Humidity and EC Sensors");

  delay(1000);
}

void loop() {
  Serial.println();
  Serial.println("===== SOIL PARAMETERS");
  Serial.print("Byte Request: ");

  String requestString;
  for (int j = 0; j < 8; j++) {
    requestString += byteRequest[j] < 0x10 ? " 0" : " ";
    requestString += String(byteRequest[j], HEX);
    requestString.toUpperCase();
  }
  Serial.println(requestString);

  // Send byte request to sensor
  sensor.write(byteRequest, 8);

  // Wait for sensor to response
  unsigned long resptime = millis();
  while ((sensor.available() < sensorFrameSize) && ((millis() - resptime) < sensorWaitingTime)) {
    delay(1);
  }

  while (sensor.available()) {
    for (int n = 0; n < sensorFrameSize; n++) {
      byteResponse[n] = sensor.read();
    }

    if (byteResponse[0] != sensorID && byteResponse[1] != sensorFunction && byteResponse[2] != sensorByteResponse) {
      Serial.println(F("SENSOR FAILED!"));
      return;
    }
  }

  Serial.print("Byte Response: ");

  String responseString;
  for (int j = 0; j < sensorFrameSize; j++) {
    responseString += byteResponse[j] < 0x10 ? " 0" : " ";
    responseString += String(byteResponse[j], HEX);
    responseString.toUpperCase();
  }
  Serial.println(responseString);

  // Conversion of byte response to data
  moisture = sensorValue((int)byteResponse[3], (int)byteResponse[4]) * 0.1 ;
  temperature = sensorValue((int)byteResponse[5], (int)byteResponse[6]) * 0.1;
  ec = sensorValue((int)byteResponse[7], (int)byteResponse[8]);

  // Print the data on Serial Monitor
  Serial.println("Moisture: " + (String)moisture + " %");
  Serial.println("Temperature: " + (String)temperature + " °C");
  Serial.println("EC: " + (String)ec + " uS/cm");

  delay(5000);
}

int sensorValue(int x, int y) {
  int t = 0;
  t = x * 256;
  t = t + y;

  return t;
}