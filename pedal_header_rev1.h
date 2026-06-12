#ifndef SLAVE_PEDAL_H
#define SLAVE_PEDAL_H

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <stdint.h>

/* 
 * Pin Definitions based on the Slave Pedal Schematic 
 */

// LED Outputs (Port A)
// Note: PA3 is also AREF. We must use AVCC as the ADC reference 
// so the output driver on PA3 is not disabled.
#define PORT_LED     PORTA
#define DDR_LED      DDRA
#define PIN_LED_RED  PA2  // D4 Red LED (On/Off)
#define PIN_LED_RG_G PA3  // D3 RG LED Green (Pot 2 active)
#define PIN_LED_RG_R PA4  // D3 RG LED RED (Pot 1 active)

// USI SPI Pins (Port B)
#define PORT_SPI     PORTB
#define DDR_SPI      DDRB
#define PIN_SPI      PINB
#define PIN_DI       PB0  // USI Data Input
#define PIN_DO       PB1  // USI Data Output
#define PIN_USCK     PB2  // USI Clock
#define PIN_CS       PB4  // Chip Select from Master (Triggering PCINT)

// Switch Pins (Port B)
#define PORT_SW      PORTB
#define DDR_SW       DDRB
#define PIN_SW       PINB
#define PIN_SW1      PB6  // On/Off Switch (Triggers INT0)
#define PIN_SW2      PB5  // Selection Switch (Triggers PCINT)

// Protocol Definitions
#define PACKET_HEADER 0xAA

// Function Prototypes
void io_init(void);
void adc_init(void);
uint8_t adc_read(uint8_t channel);
void usi_init(void);
uint8_t calculate_crc8(uint8_t *data, uint8_t len);

#endif // SLAVE_PEDAL_H