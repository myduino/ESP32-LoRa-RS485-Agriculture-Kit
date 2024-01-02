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
 * Myduino - https://myduino.com/product/jhs-268/
 * 
 */

#define BLYNK_PRINT                 Serial
#define BLYNK_TEMPLATE_ID           "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME         "YOUR_BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN            "YOUR_BLYNK_AUTH_TOKEN"
#define WIFI_SSID                   "YOUR_WIFI_NAME"
#define WIFI_PASSWORD               "YOUR_WIFI_PASSWORD"

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32_SSL.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// Sensor Pin Definition
#define RS485Power                  18
#define RS485RX                     23
#define RS485TX                     22

SoftwareSerial sensor(RS485RX, RS485TX);

// OLED Pin Definitions
#define ESP32SDA                    4
#define ESP32SCL                    5

#define displayPower                19
#define screenWidth                 128
#define screenHeight                32
#define OLEDReset                   -1

Adafruit_SSD1306 display(screenWidth, screenHeight, &Wire, OLEDReset);

// Sensor Pin Definition
#define RS485Power                  18
#define RS485RX                     23
#define RS485TX                     22

#define sensorFrameSize             19
#define sensorWaitingTime           1000
#define sensorID                    0x01
#define sensorFunction              0x03
#define sensorByteResponse          0x0E

// RS485 Byte Address Request to Sensor
unsigned char byteRequest[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
unsigned char byteResponse[19] = {};

float moisture, temperature, ph, nitrogen, phosphorus, potassium;
int ec;

unsigned long lastMillis = 0;

void setup() {
  // Debug console
  Serial.begin(115200);
  sensor.begin(4800);

  pinMode(displayPower, OUTPUT);
  pinMode(RS485Power, OUTPUT);

  digitalWrite(displayPower, HIGH);
  digitalWrite(RS485Power, HIGH);

  Serial.println();
  Serial.println("Agriculture Kit Sensor Ready");
  Serial.println("Soil Temperature, Humidity, EC, pH and NPK Sensors\n");

  Wire.begin(ESP32SDA, ESP32SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  Serial.println("> OLED Display Ready");

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print(F("Agri Sensor Kit!"));

  display.setCursor(0, 10);
  display.print(F("Moist,Temp,EC,pH,NPK"));

  display.display();

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  display.setCursor(0, 20);
  display.print(F("Blynk Ready!"));

  display.display();

  delay(1000);
}

void loop() {
  Blynk.run();

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
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextColor(WHITE, BLACK);
      display.print(F("SENSOR FAILED!"));
      display.display();

      return;
    }
  }

  Serial.println();
  Serial.println("===== SOIL DATA SENSING");
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
  ph = sensorValue((int)byteResponse[9], (int)byteResponse[10]) * 0.1;
  nitrogen = sensorValue((int)byteResponse[11], (int)byteResponse[12]);
  phosphorus = sensorValue((int)byteResponse[13], (int)byteResponse[14]);
  potassium = sensorValue((int)byteResponse[15], (int)byteResponse[16]);

  // Print the data on Serial Monitor
  Serial.println("Moisture: " + (String)moisture + " %");
  Serial.println("Temperature: " + (String)temperature + " °C");
  Serial.println("pH: " + (String)ph);
  Serial.println("EC: " + (String)ec + " uS/cm");
  Serial.println("Nitrogen (N): " + (String)nitrogen + " mg/kg");
  Serial.println("Phosporus (P): " + (String)phosphorus + " mg/kg");
  Serial.println("Potassium (K): " + (String)potassium + " mg/kg");

  displaySoil(moisture, temperature, ph, ec, nitrogen, phosphorus, potassium);
  
  delay(3000);

  // Interval 10 seconds push data to Blynk dashboard
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();

    Blynk.virtualWrite(V0, moisture);
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, ec);
    Blynk.virtualWrite(V3, ph);
    Blynk.virtualWrite(V4, nitrogen);
    Blynk.virtualWrite(V5, phosphorus);
    Blynk.virtualWrite(V6, potassium);

  }
}

void displaySoil(float mo, float tp, float ph, float ec, float ni, float ps, float po) {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  display.setCursor(0, 0);
  display.print(F("MO:"));
  display.print(mo, 1);
  display.print(F("%"));

  display.setCursor(66, 0);
  display.print(F("TP:"));
  display.print(tp, 1);
  display.print(F("\367C"));

  display.setCursor(0, 8);
  display.print(F("pH:"));
  display.print(ph, 1);

  display.setCursor(66, 8);
  display.print(F("EC:"));
  display.print(ec);

  display.setCursor(0, 16);
  display.print(F(" <<<NPK>>> "));

  display.setCursor(66, 16);
  display.print(F("NI:"));
  display.print(ni, 1);

  display.setCursor(0, 24);
  display.print(F("PS:"));
  display.print(ps, 1);

  display.setCursor(66, 24);
  display.print(F("PO:"));
  display.print(po, 1);

  display.display();
  
  delay(3000);
}

int sensorValue(int x, int y) {
  int t = 0;
  t = x * 256;
  t = t + y;

  return t;
}