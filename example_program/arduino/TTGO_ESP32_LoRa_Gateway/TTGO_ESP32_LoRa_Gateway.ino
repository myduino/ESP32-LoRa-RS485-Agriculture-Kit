#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// OLED Pin Definitions
#define ESP32SDA            22
#define ESP32SCL            21

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64

#define OLED_RESET          4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LoRa Module Pin Definition
#define SPI_MOSI            27
#define SPI_MISO            19
#define SPI_SCK             5

#define LORA_RST            14
#define LORA_CS             18

#define LORA_DIO0           26

#define LORA_FREQUENCY      923E6
#define LORA_BANDWIDTH      125.0
#define LORA_SPREADING      7
#define LORA_CODING_RATE    7
#define LORA_OUTPUT_POWER   10
#define LORA_PREAMBLE_LEN   8
#define LORA_GAIN           0
#define LORA_SYNC_WORD      0x34

void setup() {
  Serial.begin(115200);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  delay(1000);
  Serial.println();

  Serial.println("TTGO ESP32 LoRa Gateway:");

  Wire.begin(ESP32SDA, ESP32SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("OLED Display failed!"));
    while(1);
  }

  Serial.println("> OLED Display OK!");

  display.clearDisplay();

  display.setCursor(0, 10);
  display.print(F("LoRa Gateway!"));

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("> LoRa Init failed!");
    while(1);
  }

  LoRa.setSyncWord(LORA_SYNC_WORD);
  LoRa.setSpreadingFactor(LORA_SPREADING);

  Serial.println("> LoRa Init OK!");

  display.setCursor(0, 10);
  display.print(F("LoRa Init OK!"));

  display.display();
  
}

void loop() {

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }

}
