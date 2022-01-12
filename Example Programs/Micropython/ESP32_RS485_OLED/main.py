from machine import Pin, SoftI2C, UART
import ssd1306
from time import sleep

oled_power = Pin(19, Pin.OUT)
oled_power.on()

# ESP32 I2CPin Assignment
i2c = SoftI2C(scl=Pin(5), sda=Pin(4))

oled_width = 128
oled_height = 32
oled = ssd1306.SSD1306_I2C(oled_width, oled_height, i2c)

oled.fill(0)
oled.show()

# ESP32 Modbus
modbus_power = Pin(18, Pin.OUT)
modbus_power.on()

modbus_uart = UART(1, baudrate=4800, tx=22, rx=23)

byte_request = b'\x01\x03\x00\x00\x00\x07\x04\x08'

def sensor_value(start, last):
  t = 0
  t = start * 256
  t = t + last
  
  return t

def display_soil(m, t, p):
    oled.fill(0)
    
    oled.text('M: {} %'.format(m), 2, 2)
    oled.text('T: {} C'.format(t), 2, 12)
    oled.text('P: {}'.format(p), 2, 22)

    oled.show()
    
    sleep(3)

def display_npk(n, p, k):
    oled.fill(0)
    
    oled.text('N: {} mg/kg'.format(n), 2, 0)
    oled.text('P: {} mg/kg'.format(p), 2, 10)
    oled.text('K: {} mg/kg'.format(k), 2, 20)

    oled.show()
    
    sleep(3)

while True:
    
    modbus_uart.write(byte_request)
    
    if modbus_uart.any():
        byte_response = modbus_uart.read()
        
        print('Response Type: {}'.format(type(byte_response)))
        print('Response Bytes String: {}'.format(byte_response))
        
        print('Response Bytes Hex: ', end='')
        
        for byte in byte_response:
            print('0x{:02X}'.format(byte), end='')
            print(' ', end='')
        print()
        
        ints = list(byte_response)
        print('Response Bytes Int: {}'.format(ints))
        
        moisture = sensor_value(byte_response[3], byte_response[4]) * 0.1 ;
        temperature = sensor_value(byte_response[5], byte_response[6]) * 0.1;
        ph = sensor_value(byte_response[9], byte_response[10]) * 0.1;
        nitrogen = sensor_value(byte_response[11], byte_response[12]);
        phosphorus = sensor_value(byte_response[13], byte_response[14]);
        potassium = sensor_value(byte_response[15], byte_response[16]);
        
        print('Moisture: {} %'.format(moisture))
        print('Temperature: {} °C'.format(temperature))
        print('pH: {}'.format(ph))
        print('Nitrogen: {} mg/kg'.format(nitrogen))
        print('Phosphorus: {} mg/kg'.format(phosphorus))
        print('Potassium: {} mg/kg'.format(potassium))
        
        print()
        
        display_soil(moisture, temperature, ph)
        display_npk(nitrogen, phosphorus, potassium)
