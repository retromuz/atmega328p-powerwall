# atmega328p-powerwall

This is the brain behind the DC to DC boost converter that charges 14S Li-Ion battery system. Acts as an I²C slave for ESP8266 master. ESP8266 manages the MPPT function with data provided by ESP32 connected to BMS.