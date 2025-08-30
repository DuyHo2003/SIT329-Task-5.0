#ifndef GPIO_DEAKIN_H
#define GPIO_DEAKIN_H

#include <sam.h>
#include <Arduino.h>   // include Arduino core for INPUT, OUTPUT, LOW, HIGH

// Use Arduino types: PinMode and HIGH/LOW
void Config_GPIO(char port, uint8_t pin, PinMode mode);
void Write_GPIO(char port, uint8_t pin, uint8_t value);
uint8_t Read_GPIO(char port, uint8_t pin);

#endif
