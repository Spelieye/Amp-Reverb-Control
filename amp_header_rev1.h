#ifndef AMP_MASTER_H
#define AMP_MASTER_H

#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <stdint.h>

/* 
 * Pin Definitions based on the Amp Master Schematic 
 */
#define PORT_SPI    PORTB
#define DDR_SPI     DDRB
#define PIN_DI      PB1  // Software SPI Data Input (from Slave DO) 
#define PIN_DO      PB0  // Software SPI Data Output (to Slave DI & MCP SDI)
#define PIN_SCK     PB2  // Software SPI Clock that will be manually pulsed
#define PIN_CS_MCP  PB3  // Chip Select for MCP41100
#define PIN_CS_SLV  PB4  // Chip Select for Slave MCU (Labeled SDO on JP2)
#define PIN_TLP     PB5  // TLP3555 Optorelay Control (AN pin)

// Protocol Definitions
#define PACKET_HEADER       0xAA
#define SWITCH_ON_MASK      0x01

// Function Prototypes
void io_init(void);
void spi_init(void);
uint8_t spi_transfer(uint8_t data);
void adc_init(void);
uint8_t adc_read_8bit(void);
void mcp41100_set(uint8_t value);
uint8_t calculate_crc8(uint8_t *data, uint8_t len);

#endif // AMP_MASTER_H