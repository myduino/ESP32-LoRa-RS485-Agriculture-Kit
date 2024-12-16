/*
  ESP32 LoRa RS485 Agriculture Kit
  Author: Mohamad Ariffin Zulkifli

  This is an example sketch for Agricluture Kit
  Read the Soil parameters from soil sensor using RS485
  Print the measure soil on Serial Monitor.
  Show the measured soil parameters on the OLED display.
  Gather sensor's information into String CSV (Comma Separated Value)
  Wirelessly Transmit JSON data using LoRa communication.

  Purchase?
  Shopee - https://shopee.com.my/Agriculture-Kit-ESP32-LoRa-915-MHz-RS485-Modbus-Controller-with-Industrial-Grade-Soil-Temperature-Humidity-NPK-Sensor-i.132184430.15335468998
  
  Please install required library before compiling the sketch.
  1. LoRa by Sandeep Mistry
  2. Adafruit SSD1306 by Adafruit
  3. Adafruit GFX by Adafruit
  4. ESPSoftwareSerial by Dirk Kaar
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

// If you are using different Development Board,
// do change the RX, TX pin appropriately
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

#define LORA_ID             1
#define LORA_FREQUENCY      923E6
#define LORA_BANDWIDTH      125.0
#define LORA_SPREADING      7
#define LORA_CODING_RATE    7
#define LORA_OUTPUT_POWER   10
#define LORA_PREAMBLE_LEN   8
#define LORA_GAIN           0
#define LORA_SYNC_WORD      0x34

SoftwareSerial sensor(RS485RX, RS485TX);

unsigned char byteRequest[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
unsigned char byteResponse[19] = {};

float moisture, temperature, ph, nitrogen, phosphorus, potassium;
int ec;

unsigned long lastLoRaTransmit = 0;
String loRaPacket, loRaData;
int loRaRSSI;

#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  5

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

void setup() {
  Serial.begin(115200);
  sensor.begin(4800);

  pinMode(displayPower, OUTPUT);
  pinMode(RS485Power, OUTPUT);

  digitalWrite(displayPower, HIGH);
  digitalWrite(RS485Power, HIGH);

  delay(1000);
  Serial.println();
  Serial.println("Agriculture Kit Sensor Ready");
  Serial.println("> TID: " + (String)LORA_ID);

  Wire.begin(ESP32SDA, ESP32SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  Serial.println("> OLED Display Ready");

  displayLogo();

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("> Starting LoRa failed!");
    while (1);
  }

  LoRa.setSyncWord(LORA_SYNC_WORD); // default 0x34
  LoRa.setSpreadingFactor(LORA_SPREADING);

  Serial.println("> LoRa Communication Ready");
  Serial.print("> Sleep Mode: ");
  print_wakeup_reason();

  display.setCursor(10, 10);
  display.print(F("LoRa Init OK!"));

  display.setCursor(10, 20);
  display.print(F("TID: "));
  display.print(LORA_ID);
  display.display();
  
  delay(3000);

  sensor.write(byteRequest, 8);

  unsigned long resptime = millis();
  while ((sensor.available() < sensorFrameSize) && ((millis() - resptime) < sensorWaitingTime)) {
    delay(1);
  }

  while (sensor.available()) {
    for (int n = 0; n < sensorFrameSize; n++) {
      byteResponse[n] = sensor.read();
    }

    if (byteResponse[0] != sensorID && byteResponse[1] != sensorFunction && byteResponse[2] != sensorByteResponse) {

      String loRaPacket = "{\"tid\":";
      loRaPacket += (String)LORA_ID;
      loRaPacket +=  ",\"error\":\"unable to read sensor's data\"";
      loRaPacket += "}";

      Serial.println();
      Serial.println("> LoRa Packet: " + loRaPacket);

      
      LoRa.beginPacket();
      LoRa.print(loRaPacket);
      LoRa.endPacket();

      Serial.println("> LoRa Transmit OK!");

      display.setCursor(10, 20);
      display.setTextColor(WHITE, BLACK);
      display.print(F("SENSOR FAILED!"));
      display.display();

      return;

      // delay(5000);
  
      // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR * 60);
      // Serial.println("> Sleep mode for " + String(TIME_TO_SLEEP) + " Minutes");
      // displaySleepMode(TIME_TO_SLEEP);
      
      // esp_deep_sleep_start();
    }
  }

  Serial.println();
  Serial.println("===== SOIL PARAMETERS");
  Serial.print("Byte Response: ");

  String responseString;
  for (int j = 0; j < sensorFrameSize; j++) {
    responseString += byteResponse[j] < 0x10 ? " 0" : " ";
    responseString += String(byteResponse[j], HEX);
    responseString.toUpperCase();
  }
  Serial.println(responseString);

  moisture = sensorValue((int)byteResponse[3], (int)byteResponse[4]) * 0.1 ;
  temperature = sensorValue((int)byteResponse[5], (int)byteResponse[6]) * 0.1;
  ec = sensorValue((int)byteResponse[7], (int)byteResponse[8]);
  ph = sensorValue((int)byteResponse[9], (int)byteResponse[10]) * 0.1;
  nitrogen = sensorValue((int)byteResponse[11], (int)byteResponse[12]);
  phosphorus = sensorValue((int)byteResponse[13], (int)byteResponse[14]);
  potassium = sensorValue((int)byteResponse[15], (int)byteResponse[16]);

  Serial.println("Moisture: " + (String)moisture + " %");
  Serial.println("Temperature: " + (String)temperature + " °C");
  Serial.println("EC: " + (String)ec + " uS/cm");
  Serial.println("pH: " + (String)ph);
  Serial.println("Nitrogen (N): " + (String)nitrogen + " mg/kg");
  Serial.println("Phosporus (P): " + (String)phosphorus + " mg/kg");
  Serial.println("Potassium (K): " + (String)potassium + " mg/kg");

  displaySoil(moisture, temperature, ph, ec, nitrogen, phosphorus, potassium);

  String loRaPacket = "{\"tid\":";
  loRaPacket += (String)LORA_ID;
  loRaPacket +=  ",\"tp\":";
  loRaPacket += (String)temperature;
  loRaPacket +=  ",\"mo\":";
  loRaPacket += (String)moisture;
  loRaPacket +=  ",\"ec\":";
  loRaPacket += (String)ec;
  loRaPacket +=  ",\"ph\":";
  loRaPacket += (String)ph;
  loRaPacket +=  ",\"nt\":";
  loRaPacket += (String)nitrogen;
  loRaPacket +=  ",\"ps\":";
  loRaPacket += (String)phosphorus;
  loRaPacket +=  ",\"po\":";
  loRaPacket += (String)potassium;
  loRaPacket += "}";

  Serial.println();
  Serial.println("> LoRa Packet: " + loRaPacket);

  LoRa.beginPacket();
  LoRa.print(loRaPacket);
  LoRa.endPacket();

  Serial.println("> LoRa Transmit OK!");
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR * 60);
  Serial.println("> Sleep mode for " + String(TIME_TO_SLEEP) + " Minutes");
  displaySleepMode(TIME_TO_SLEEP);
  
  esp_deep_sleep_start();

}

void loop() {
  
}

void displayLogo() {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.print(F("MYINVENT"));
  display.display();
  delay(1000);
}


int sensorValue(int x, int y) {
  int t = 0;
  t = x * 256;
  t = t + y;

  return t;
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
  display.print(F("EC:"));
  display.print(ec);

  display.setCursor(66, 8);
  display.print(F("pH:"));
  display.print(ph, 1);

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

void displaySleepMode(int sleepInterval) {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  display.setCursor(0, 0);
  display.print(F("Sleep Mode: "));
  display.print(sleepInterval);
  display.println(F(" Mins"));
  display.print(F("In 3 seconds..."));

  display.display();
  
  delay(3000);
}
