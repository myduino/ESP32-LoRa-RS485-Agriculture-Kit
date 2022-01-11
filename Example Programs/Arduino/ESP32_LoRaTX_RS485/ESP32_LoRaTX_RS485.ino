/*
 * Agriculture Kit
 * Author: Mohamad Ariffin Zulkifli
 * 
 * This is an example sketch for Agricluture Kit
 * Read the Soil parameters from soil sensor using RS485
 * Print the measure soil on Serial Monitor.
 * Show the measured soil parameters on the OLED display.
 * Gather sensor's information into String CSV (Comma Separated Value)
 * Wirelessly Transmit CSV data using LoRa communication.
 * 
 * Purchase?
 * Shopee - https://shopee.com.my/Agriculture-Kit-ESP32-LoRa-915-MHz-RS485-Modbus-Controller-with-Industrial-Grade-Soil-Temperature-Humidity-NPK-Sensor-i.132184430.15335468998
 * 
 */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// OLED Pin Definitions
#define ESP32SDA            4
#define ESP32SCL            5

#define displayPower        19
#define screenWidth         128
#define screenHeight        32
#define OLEDReset           -1

Adafruit_SSD1306 display(screenWidth, screenHeight, &Wire, OLEDReset);

// Sensor Pin Definition
#define RS485Power          18
#define RS485RX             23
#define RS485TX             22

#define sensorFrameSize     19
#define sensorWaitingTime   1000
#define sensorID            0x01
#define sensorFunction      0x03
#define sensorByteResponse  0x0E

// LoRa Module Pin Definition
#define SPI_MOSI            13
#define SPI_MISO            12
#define SPI_SCK             14

#define LORA_RST            33
#define LORA_CS             32

#define LORA_DIO0           25
#define LORA_DIO1           27

#define LORA_FREQUENCY      923.0
#define LORA_BANDWIDTH      125.0
#define LORA_SPREADING      9
#define LORA_CODING_RATE    7
#define LORA_OUTPUT_POWER   10
#define LORA_PREAMBLE_LEN   8
#define LORA_GAIN           0
#define LORA_SYNC_WORD      0x34

SoftwareSerial sensor(RS485RX, RS485TX);

unsigned char byteRequest[8] = {0X01, 0X03, 0X00, 0X00, 0X00, 0X07, 0X04, 0X08};
unsigned char byteResponse[19] = {};

float moisture, temperature, ph, nitrogen, phosphorus, potassium;

unsigned long lastLoRaTransmit = 0;
String loRaPacket;

void setup() {
  Serial.begin(9600);
  sensor.begin(4800);

  pinMode(displayPower, OUTPUT);
  pinMode(RS485Power, OUTPUT);
  
  digitalWrite(displayPower, HIGH);
  digitalWrite(RS485Power, HIGH);

  delay(1000);
  Serial.println();
  Serial.println("Agriculture Kit Sensor Ready");
  
  Wire.begin(ESP32SDA, ESP32SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }
  
  Serial.println("> OLED Display Ready");
  
  displayLogo();

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
  
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("> Starting LoRa failed!");
    while(1);
  }

  // LoRa.setSyncWord(LORA_SYNC_WORD); // default 0x34
  LoRa.setSpreadingFactor(LORA_SPREADING);

  Serial.println("> LoRa Communication Ready");

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

  if(millis() - lastLoRaTransmit > 5000){
    lastLoRaTransmit = millis();
    
    String loRaPacket = (String)moisture + "," + (String)temperature + "," + (String)nitrogen + "," + (String)ph + "," + (String)phosphorus + "," + (String)potassium;

    Serial.println();
    Serial.println("> LoRa Packet: " + loRaPacket);
    
    LoRa.beginPacket();
    LoRa.print(loRaPacket);
    LoRa.endPacket();

    Serial.println("> LoRa TxDone");
  }
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
