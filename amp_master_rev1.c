/**
 * 
 * This code is for the master MCU that sits inside the amp to control the reverb mix level. By default without the pedal pluged in it will set the digital 
 * potentiometer to the value of the reverb control knob on the front of the amp. 
 * 
 * When the pedal is plugged in the MCU will listen to the pedal, ignoring the reverb control knob on the amp, and will set the digital potentiometer 
 * to the value of either of the pedal's potietiometer settings, which can be selected by pressing the SELECT switch. 
 * The LED will toggle and change color (Red or Green). 
 * 
 * When the ON/OFF switch is pressed the master MCU will mute or turn off the reverb in the amp. The associated LED will also turn on or off to reflect 
 * the state of the Reverb effect. 
 * 
 * The default settings when the pedal is plugged will be: Reverb On, potentiometer 1 (red knob and red led) selected.
 * 
 * 
 * 
 * Note: I had a bit of snafu when designing the PCB and physically swapped the DO and DI pins on the master MCU so it cannot perform proper USI communication. 
 * However, I was able to save the circuit board and correct this in software using a technique called "Bit-Banging" (Software SPI). It's not the 
 * fastest option but given that this MCU is doing only one thing, it will still do it really quickly and isn't noticable to the ear. 
 * This is rev1 anyway.
 * 
 * 
 * This is a do what you want lisence for this open-source code. Enjoy!
 * 
 * DLG 06/10/26
 * 
 * **/
#include "amp_header_rev1.h"

// Initializes all required I/O pins
void io_init(void) {
    // Set DO, USCK, CS_MCP, CS_SLV, and TLP as outputs
    DDR_SPI |= (1 << PIN_DO) | (1 << PIN_SCK) | (1 << PIN_CS_MCP) | (1 << PIN_CS_SLV) | (1 << PIN_TLP);
    
    // Set DI as input and enable internal pull-up resistor to handle physical disconnection
    DDR_SPI &= ~(1 << PIN_DI);
    PORT_SPI |= (1 << PIN_DI);
    
    // Set CS pins HIGH (inactive) by default
    PORT_SPI |= (1 << PIN_CS_MCP) | (1 << PIN_CS_SLV);
    
    // Ensure TLP3555 is off initially
    PORT_SPI &= ~(1 << PIN_TLP);
}

// Transfer a single byte via "bit-banging"
uint8_t software_spi_transfer(uint8_t data) {
    uint8_t received_data = 0;
    
    // Loop through all 8 bits, starting with the Most Significant Bit (MSB)
    for (uint8_t i = 0; i < 8; i++) {
        
        // 1. Set the data output pin HIGH or LOW depending on the data bit
        if (data & 0x80) {
            PORT_SPI |= (1 << PIN_DO);  // Set PIN_DO HIGH
        } else {
            PORT_SPI &= ~(1 << PIN_DO); // Set PIN_DO LOW
        }
        
        // 2. Pull SCK HIGH 
        PORT_SPI |= (1 << PIN_SCK);
        
        // 3. Read the incoming bit from the Slave MCU on the data input pin
        received_data <<= 1; // Shift the received data left
        if (PINB & (1 << PIN_DI)) {
            received_data |= 0x01; // If PIN_DI is HIGH, set the lowest bit to 1
        }
        
        // 4. Pull SCK (PIN_SCK) LOW to complete the clock cycle
        PORT_SPI &= ~(1 << PIN_SCK);
        
        // Shift the outgoing data left to prepare the next bit
        data <<= 1;
    }
    
    return received_data;
}


// Initializes the ADC for the local Master potentiometer (PA0)
void adc_init(void) {
    // Use VCC as reference, Left Adjust Result (easy 8-bit read), select ADC0
    ADMUX = (1 << ADLAR); 
    
    // Enable ADC and set prescaler to 64 (8 MHz / 64 = 125 kHz)
    ADCSR = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); 
}

// Reads the 8-bit value from the local amp's potentiometer
uint8_t adc_read_8bit(void) {
    ADCSR |= (1 << ADSC); // Start conversion
    while (ADCSR & (1 << ADSC)); // Wait for completion
    return ADCH; // Return 8-bit left-adjusted value
}

// Sets the resistance value on the MCP41100 digital potentiometer
void mcp41100_set(uint8_t value) {
    PORT_SPI &= ~(1 << PIN_CS_MCP); // CS LOW to execute command
    software_spi_transfer(0x11);    // Send Command byte (Write Data to Pot 0)
    software_spi_transfer(value);   // Send 8-bit resistance value
    PORT_SPI |= (1 << PIN_CS_MCP);  // CS HIGH to complete
}

// Standard CRC-8 algorithm for robust data integrity verification
uint8_t calculate_crc8(uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Main execution loop
int main(void) {
    io_init();
    adc_init();
    
    uint8_t payload[3]; // Stores switch status and pot_value payload bytes
    uint8_t previous_pot_values[3] = {0}; // [pot_amp, pot1, pot2]
    uint8_t header, crc_received, crc_calculated;
    
    while (1) {
        // 1. Initiate communication and poll the Slave MCU
        PORT_SPI &= ~(1 << PIN_CS_SLV);
        // Give the Slave MCU a few microseconds to handle its CS interrupt 
        // and load the first byte into its USIDR register before clocking starts.
        _delay_us(10); 
        header    = software_spi_transfer(0x00); // send dummy bytes to initiate the clock pulses
        payload[0]   = software_spi_transfer(0x00); // Byte 1: ON/OFF state & selected pot
        payload[1]   = software_spi_transfer(0x00); // Byte 2: Potentiometer 1 reading
        payload[2]   = software_spi_transfer(0x00); // Byte 3: Potentiometer 2 reading
        crc_received = software_spi_transfer(0x00); // Byte 4: Transmitted CRC
        // Pull Slave CS HIGH to complete transfer
        PORT_SPI |= (1 << PIN_CS_SLV);
        
        // 2. Compute CRC to check packet integrity
        crc_calculated = calculate_crc8(payload, 3);
        
        // 3. Verify Connection and Decode Data
        // A disconnected Slave reads `0xFF` indefinitely due to the active Pull-up. 
        // This fails the header check. If connected, the CRC ensures data was not corrupted during hot-swap.
        if (header == PACKET_HEADER && crc_calculated == crc_received) {
            
            /* ---- SLAVE CONNECTED & VERIFIED ---- */
            // DATA IS VALID
            // (Handle switch states and forward payload to MCP41100 via software_spi_transfer)
          
            // Activate or deactivate TLP3555 based on the on/off bit. If ON=1, set PIN_TLP LOW. 
            if (!(payload[0] & SWITCH_ON_MASK)) {
                PORT_SPI |= (1 << PIN_TLP); // Mute the effect by pullng pin HIGH
            } else {
                PORT_SPI &= ~(1 << PIN_TLP); // Pull pin LOW
            }
            
            // Decode which pot is currently selected (pot1=0, pot2=1)
            // Set the MCP41100 based on the values retrieved from the slave
            if ((payload[0] >> 1) & 1) {
                //check for changing values in the pot, i.e. no need to continually set the same value 
                if (payload[2] != previous_pot_values[2]) mcp41100_set(payload[2]);
            } else { 
                // pot 1 is selected
                if (payload[1] != previous_pot_values[1]) mcp41100_set(payload[1]);
            }
            // save current pot values
            previous_pot_values[1] = payload[1];
            previous_pot_values[2] = payload[2];
       
        } else {
            /* ---- SLAVE DISCONNECTED OR CORRUPTED ---- */
            
            // Unmute Reverb --> only do this if PIN_TLP is high 
            if (PINB & (1 << PIN_TLP)) PORT_SPI &= ~(1 << PIN_TLP);

            // Safely fall back to reading the amp's local potentiometer
            uint8_t pot_value = adc_read_8bit();
            // set the value if it has changed
            if (previous_pot_values[0] != pot_value) mcp41100_set(pot_value);
            previous_pot_values[0] = pot_value; 
        }
        
        // Delay to regulate polling frequency
        _delay_ms(10);
    }
    
    return 0;
}