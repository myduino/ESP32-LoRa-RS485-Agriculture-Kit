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

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define displayPower        19
#define screenWidth         128
#define screenHeight        32
#define OLEDReset           -1

Adafruit_SSD1306 display(screenWidth, screenHeight, &Wire, OLEDReset);

#define ESP32SDA            4
#define ESP32SCL            5

#define RS485Power          18

// If you are using different Development Board,
// do change the RX, TX pin appropriately
#define RS485RX             23
#define RS485TX             22

#define sensorFrameSize     19
#define sensorWaitingTime   1000
#define sensorID            0x01
#define sensorFunction      0x03
#define sensorByteResponse  0x0E

SoftwareSerial sensor(RS485RX, RS485TX);

unsigned char byteRequest[8] = {0X01, 0X03, 0X00, 0X00, 0X00, 0X07, 0X04, 0X08};
unsigned char byteResponse[19] = {};

float moisture, temperature, ph, nitrogen, phosphorus, potassium;

void setup() {
  Serial.begin(115200);
  sensor.begin(4800);

  pinMode(displayPower, OUTPUT);
  pinMode(RS485Power, OUTPUT);
  
  digitalWrite(displayPower, HIGH);
  digitalWrite(RS485Power, HIGH);

  delay(1000);
  Serial.println();
  
  Wire.begin(ESP32SDA, ESP32SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }
  
  Serial.println("Agriculture Kit Sensor Ready");

  displayLogo();

}


void loop() {
  
  sensor.write(byteRequest, 8);

  unsigned long resptime = millis();
  while ((sensor.available() < sensorFrameSize) && ((millis() - resptime) < sensorWaitingTime)) {
    delay(1);
  }
  
  while (sensor.available()) {
    for (int n = 0; n < sensorFrameSize; n++) {
      byteResponse[n] = sensor.read();
    }

    if(byteResponse[0] != sensorID && byteResponse[1] != sensorFunction && byteResponse[2] != sensorByteResponse){
      return;
    }
  }

  Serial.println();
  Serial.println("===== SOIL PARAMETERS");
  Serial.print("Byte Response: ");

  String responseString;
  for (int j = 0; j < 19; j++) {
    responseString += byteResponse[j] < 0x10 ? " 0" : " ";
    responseString += String(byteResponse[j], HEX);
    responseString.toUpperCase();
  }
  Serial.println(responseString);

  moisture = sensorValue((int)byteResponse[3], (int)byteResponse[4]) * 0.1 ;
  temperature = sensorValue((int)byteResponse[5], (int)byteResponse[6]) * 0.1;
  ph = sensorValue((int)byteResponse[9], (int)byteResponse[10]) * 0.1;
  nitrogen = sensorValue((int)byteResponse[11], (int)byteResponse[12]);
  phosphorus = sensorValue((int)byteResponse[13], (int)byteResponse[14]);
  potassium = sensorValue((int)byteResponse[15], (int)byteResponse[16]);
  
  Serial.println("Moisture: " + (String)moisture + " %");
  Serial.println("Temperature: " + (String)temperature + " °C");
  Serial.println("pH: " + (String)ph);
  Serial.println("Nitrogen (N): " + (String)nitrogen + " mg/kg");
  Serial.println("Phosporus (P): " + (String)phosphorus + " mg/kg");
  Serial.println("Potassium (K): " + (String)potassium + " mg/kg");

  displaySoil(moisture, temperature, ph);
  displayNPK(nitrogen, phosphorus, potassium);
}

void displayLogo() {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.println(F("Myinvent"));
  display.setTextSize(1);
  display.setCursor(10, 16);
  display.println(F("ESp32-LoRa-RS485"));
  display.display();
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x01);
  delay(4000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}


int sensorValue(int x, int y) {
  int t = 0;
  t = x * 256;
  t = t + y;
  
  return t;
}

void displaySoil(float h, float t, float p) {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 0);
  display.print(F("T:"));
  display.print(t, 1);
  display.print(F(" \367C"));

  display.setCursor(66, 0);
  display.print(F("H:"));
  display.print(h, 1);
  display.print(F(" %"));

  display.setCursor(2, 16);
  display.print(F("pH:"));
  display.print(p, 1);

  display.display();
  delay(3000);
}

void displayNPK(int n, int p, int k) {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 0);
  display.print(F("N:"));
  display.print(n);
  display.print(F(" mg/kg"));

  display.setCursor(66, 0);
  display.print(F("P:"));
  display.print(p);
  display.print(F(" mg/kg"));

  display.setCursor(2, 16);
  display.print(F("K:"));
  display.print(k);
  display.print(F(" mg/kg"));

  display.display();
  delay(3000);
}
