#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH      128 // OLED display width, in pixels
#define SCREEN_HEIGHT     32 // OLED display height, in pixels

#define OLED_RESET        -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DisplayPower      19

//ESP32
#define DIO0              25
#define DIO1              27

#define ESP32_SDA         4
#define ESP32_SCL         5

#define LORA_RST          33
#define LORA_CS           32

#define SPI_MOSI          13
#define SPI_MISO          12
#define SPI_SCK           14

#define FREQUENCY         923.0
#define BANDWIDTH         125.0
#define SPREADING_FACTOR  9
#define CODING_RATE       7
#define OUTPUT_POWER      10
#define PREAMBLE_LEN      8
#define GAIN              0

#define RS485Power        18
#define RS485RX           23
#define RS485TX           22

#define sensorFrameSize   19
#define sensorWaitingTime 1000
#define sensorID          0x01
#define sensorFunction    0x03
#define byteResponse      0x0E

SoftwareSerial sensor(RS485RX, RS485TX);

unsigned char request_byte[8] = {0X01, 0X03, 0X00, 0X00, 0X00, 0X07, 0X04, 0X08};
unsigned char response_byte[19] = {};

float moisture, temperature, ph, nitrogen, phosphorus, potassium;

void setup() {
  Serial.begin(9600);
  sensor.begin(4800);

  pinMode(DisplayPower, OUTPUT);
  pinMode(RS485Power, OUTPUT);
  
  digitalWrite(DisplayPower, HIGH);
  digitalWrite(RS485Power, HIGH);
  
  delay(1000);

  Serial.println();

  Wire.begin(ESP32_SDA, ESP32_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  
  Serial.println("Agriculture Sensor Ready");

  logo_show();

}


void loop() {
  
  sensor.write(request_byte, 8);

  unsigned long resptime = millis();
  while ((sensor.available() < sensorFrameSize) && ((millis() - resptime) < sensorWaitingTime)) {
    delay(1);
  }
  
  while (sensor.available()) {
    for (int n = 0; n < sensorFrameSize; n++) {
      response_byte[n] = sensor.read();
    }

    if(response_byte[0] != sensorID && response_byte[1] != sensorFunction && response_byte[2] != byteResponse){
      return;
    }
  }

  Serial.println();
  Serial.println("===== SOIL PARAMETERS");
  Serial.print("Byte Response: ");

  String responseString;
  for (int j = 0; j < 19; j++) {
    responseString += response_byte[j] < 0x10 ? " 0" : " ";
    responseString += String(response_byte[j], HEX);
    responseString.toUpperCase();
  }
  Serial.println(responseString);

  moisture = sensorValue((int)response_byte[3], (int)response_byte[4]) * 0.1 ;
  temperature = sensorValue((int)response_byte[5], (int)response_byte[6]) * 0.1;
  ph = sensorValue((int)response_byte[9], (int)response_byte[10]) * 0.1;
  nitrogen = sensorValue((int)response_byte[11], (int)response_byte[12]);
  phosphorus = sensorValue((int)response_byte[13], (int)response_byte[14]);
  potassium = sensorValue((int)response_byte[15], (int)response_byte[16]);
  
  Serial.println("Moisture: " + (String)moisture + " %");
  Serial.println("Temperature: " + (String)temperature + " °C");
  Serial.println("pH: " + (String)ph);
  Serial.println("Nitrogen (N): " + (String)nitrogen + " mg/kg");
  Serial.println("Phosporus (P): " + (String)phosphorus + " mg/kg");
  Serial.println("Potassium (K): " + (String)potassium + " mg/kg");

  displaySoil(moisture, temperature, ph);
  displayNPK(nitrogen, phosphorus, potassium);
}

void logo_show() {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2); // Draw 2X-scale text
  display.setCursor(10, 0);
  display.println(F("Myinvent"));
  display.setTextSize(1);
  display.setCursor(10, 16);
  display.println(F("RS485-LoRa"));
  display.display(); // Show initial text
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
  display.setTextSize(1); // Draw 2X-scale text
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

  display.display(); // Show initial text
  delay(3000);
}

void displayNPK(int n, int p, int k) {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1); // Draw 2X-scale text
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

  display.display(); // Show initial text
  delay(3000);
}
