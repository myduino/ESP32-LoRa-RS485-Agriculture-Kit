# ESP32 LoRa Modbus Agriculture Kit
Affordable Agriculture Kit ESP32 LoRa 915 MHz RS485 Modbus Controller.

<p align="center"><img src="https://github.com/myinvent/affordable-esp32-lora-modbus-agriculture-kit/blob/main/references/agriculture-kit-01.jpg" width="600"></a></p>

## Consists of 2 Main Components:
1. ESP32 LoRa RS485 Modbus Controller: The heart of this board is a mighty ESP32 Wi-Fi based microcontroller under ESP-WROOM-32 module as it is compatible with Arduino programming. It has RS485 controller, thus able to convert the RS485 signal from the soil sensor. Integrated with LoRa 915 MHz wireless communication, programmable for Malaysia's LoRa frequency which is 923 MHz.
2. Industrial-Grade Soil Sensor: Measure multi-parameters of soil including Temperature, Moisture, pH, EC and Nitrogen, Phosphorus and Potassium (NPK). The output signal is RS485 and can be operated broad range of voltage from 5~30 VDC.

<p align="center"><img src="https://github.com/myinvent/affordable-esp32-lora-modbus-agriculture-kit/blob/main/references/agriculture-kit-02.jpg" width="700"></a></p>
<p align="center"><img src="https://github.com/myinvent/affordable-esp32-lora-modbus-agriculture-kit/blob/main/references/agriculture-kit-03.jpg" width="350"></a></p>
<p align="center"><img src="https://github.com/myinvent/affordable-esp32-lora-modbus-agriculture-kit/blob/main/references/agriculture-kit-04.jpg" width="350"></a></p>

## ESP32 Controller Board Specifications
- ESP32-WROOM-32 Module for microcontroller with Wi-Fi and BLE communication.
- Compatible with Arduino and Micropython Programming
- USB Serial Chip Silicon Labs CP2104
    - [Download CP2104 Driver from Silicon Labs Website](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers#software)
- LoRa Module 915 MHz (Programmable to 923 MHz for Malaysia)
- 0.91" I2C OLED Display
- Power by Micro USB, Battery or 5 VDC
- On-board LiPo Charging Circuit

# Soil Sensors Specifications
- Operating Voltage: 5 ~ 30 VDC
- Communication Protocol: RS485
- Temperature
    - Measurement Range: -40°C ~ 80 °C
    - Measurement Accuracy: ±0.5 °C
- Moisture Content
    - Measurement Range: 0 ~ 100 %
    - Measurement Accuracy: ±3 %
- pH
    - Measurement Range: 3 ~ 9
    - Measurement Resolution: 0.1
- NPK
    - Measurement Range: 0 ~ 2999 mg/kg
    - Measurement Resolution: 1 mg/kg 
- Electrical Conductivity (EC).
    - Measurement Range: 0 ~ 20000 uS/cm
    - Measurement Resolution: 1 uS/cm 

## Shipping List
- Agriculture Kit ESP32 LoRa 915 MHz RS485 Modbus Controller
- Industrial Grade Soil Sensor
- Waterproof Case

## Shopping Now
- [Shopee](https://shopee.com.my/Agriculture-Kit-ESP32-LoRa-915-MHz-RS485-Modbus-Controller-with-Industrial-Grade-Soil-Temperature-Humidity-NPK-Sensor-i.132184430.15335468998)
- [Myduino: Temperature & Moisture, pH, EC and NPK](https://myduino.com/product/agriculture-kit-esp32-lora-915-mhz-rs485-modbus-controller-with-industrial-grade-soil-temperature-humidity-ph-npk-sensor/)
- [Myduino: Temperature & Moisture](https://myduino.com/product/agriculture-kit-esp32-lora-915-mhz-rs485-modbus-controller-with-industrial-grade-soil-temperature-humidity-sensor/)

## How To?
1. Interface and Wiring | Wire Colour: Brown (5~30 VDC), Black (GND), Yellow (RS485 A) and Blue (RS485 B)
    <p><img src="https://github.com/myinvent/affordable-esp32-lora-modbus-agriculture-kit/blob/main/references/agriculture-kit-05.jpg" width="350"></a></p>
    <p><img src="https://github.com/myinvent/affordable-esp32-lora-modbus-agriculture-kit/blob/main/references/agriculture-kit-06.jpg" width="350"></a></p>
2. Choose your development environment, Arduino or Micropython.
3. Use example program from folder [Example Programs](https://github.com/myinvent/esp32-lora-modbus-agriculture-kit/tree/main/Example%20Programs)
4. Upload or flash the program into the microcontroller. For Arduino IDE, choose board "ESP32 Dev Board".

## RS845 Informations
- Data Bits: 8-bit
- Parity Bits: NO
- Stop Bit: 1
- Error Checking: CRC
- Baud Rate: 2400, 4800 (default), 9600
- Device Address: 0x01
- Function Code: 0x03
- Data Code: 16 bits
- Request Frame (8 Bytes)

| Address Code | Function Code | Start Address Register | Length of Register  | CRC        |
| ------------ | ------------- | ---------------------- | ------------------- | ---------- |
| 0x01         | 0x03          | 0x00 0x00              | 0x00 0x07           | 0x04 0x08  |

- Example of Response Frame (19 Bytes)

| Address Code | Function Code | Byte Number | Moisture  | Temperature | EC        | pH        | Nitrogen (N) | Phosphorus (P)  | Potassium (K) | CRC        |
| ------------ | ------------- | ----------- | --------- | ----------- | --------- | --------- | ------------ | --------------- | ------------- | ---------- |
| 0x01         | 0x03          | 0x0E        | 0x01 0xE6 | 0x01 0x55   | 0x05 0xDC | 0x01 0x34 | 0x00 0x20    | 0x00 0x25       | 0x00 0x30     | 0x04 0x08  |
- Byte Response Example Calculation
    - Moisture Content = (0x01 * 256 + 0xE6) * 0.1 = 48.6 %
    - Temperature = (0x01 * 256 + 0x55) * 0.1 = 34.1 °C
    - EC = (0x05 * 256 + 0xDC) = 1500 uS/cm
    - pH = (0x01 * 256 + 0x34) * 0.1 = 3.08
    - N = (0x00 * 256 + 0x20) = 32 mg/kg
    - P = (0x00 * 256 + 0x25) = 37 mg/kg
    - K = (0x00 * 256 + 0x30) = 48 mg/kg
    
## Contact Us
- Call and WhatsApp +6013-2859151
