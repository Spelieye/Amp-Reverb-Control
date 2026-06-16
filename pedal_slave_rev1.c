/**
 * 
 * This code is for the slave MCU that sits inside the pedal to control the reverb effect in the amp. 
 * 
 * By default the pedal (when plugged in) will set reverb ON and select potientiometer 1 (RED LED) as the active pot to use. 
 * It will send data to the master MCU. That is all it does, read inputs and transmits data. No audio signal will flow through the pedal.
 * 
 * The SELECT switch will toggle and change the color of the LED (Red or Green) and set which pot is to be used as reference. 
 * 
 * The ON/OFF switch will toggle the RED led and tell the master MCU to mute the reverb effect.
 * 
 * This is a do what you want lisence for this open-source code. Enjoy!
 * 
 * rev1
 * 
 * DLG 06/10/26
 * 
 * **/

#include "pedal_header_rev1.h"

// Global variables set by interrupts and read by the main loop
volatile uint8_t on_off_status = 1; // 0 = OFF, 1 = ON; Default is to be ON
volatile uint8_t pot_selection = 0; // 0 = Pot1, 1 = Pot2
volatile uint8_t sw1_flag = 0;
volatile uint8_t sw2_flag = 0;

// Transmission state machine variables
volatile uint8_t transmission_step = 0;
uint8_t pot1_val = 0;
uint8_t pot2_val = 0;

void io_init(void) {
    // 1. Configure LEDs as outputs
    DDR_LED |= (1 << PIN_LED_RED) | (1 << PIN_LED_RG_R) | (1 << PIN_LED_RG_G);
    
    // 2. Configure USI SPI Pins
    DDR_SPI |= (1 << PIN_DO); // DO is output
    DDR_SPI &= ~((1 << PIN_DI) | (1 << PIN_USCK) | (1 << PIN_CS)); // DI, USCK, CS are inputs
    PORT_SPI |= (1 << PIN_CS); // Enable pull-up on CS to detect disconnects
    
    // 3. Configure Switches as inputs with internal pull-ups 
    DDR_SW &= ~((1 << PIN_SW1) | (1 << PIN_SW2));
    PORT_SW |= (1 << PIN_SW1) | (1 << PIN_SW2);
    
    // 4. Configure Interrupts
    // Enable Pin Change Interrupt 1 for pins PB[7:4]   
    GIMSK |= (1 << PCIE1); 
}

void adc_init(void) {
    // Use AVCC as voltage reference (REFS1=0, REFS0=0) so PA3 can be an LED output.
    // Left adjust the result (ADLAR=1) to easily read 8-bit values from ADCH.
    ADMUX = (1 << ADLAR); 
    
    // Enable ADC and set prescaler to 64
    ADCSR = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); 
}

uint8_t adc_read(uint8_t channel) {
    // Clear the previous channel and set the new one
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F); 
    
    // Start conversion
    ADCSR |= (1 << ADSC); 
    
    // Wait for the conversion to complete
    while (ADCSR & (1 << ADSC)); 
    
    return ADCH; // Return the 8-bit result
}

void usi_init(void) {
    // Enable USI Three-wire mode (SPI), external clock on USCK, and overflow interrupt
    USICR = (1 << USIOIE) | (1 << USIWM0) | (1 << USICS1);
}

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

// ---------------------------------------------------------
// Interrupt Service Routines
// ---------------------------------------------------------

// Triggered by Pin Change on PB4 (CS), PB6 (SW1), and PB5 (SW2)
ISR(IO_PINS_vect) {
    // 1. Check if Master pulled Chip Select (PB4) LOW to initiate transfer
    if (!(PIN_SPI & (1 << PIN_CS))) {
        transmission_step = 0;          // Reset state machine
        USIDR = PACKET_HEADER;          // Pre-load the Header byte
        USISR = (1 << USIOIF);          // Clear overflow flag to start counting
    }

    // 2. Check if SW1 went LOW (Pressed)
    if (!(PIN_SW & (1 << PIN_SW1))) sw1_flag =1; // Mark sw1 as pressed
    
    // 3. Check if SW2 went LOW (Pressed)
    if (!(PIN_SW & (1 << PIN_SW2))) sw2_flag = 1; // Mark sw2 as pressed
}

// Triggered every 8 clock cycles from the Master
ISR(USI_OVF_vect) {
    transmission_step++;
    
    if (transmission_step == 1) {
        // Load Payload 1: Status Byte
        USIDR = (pot_selection << 1) | on_off_status;
    } 
    else if (transmission_step == 2) {
        // Load Payload 2: Pot 1 Value
        USIDR = pot1_val;
    } 
    else if (transmission_step == 3) {
        // Load Payload 3: Pot 2 Value
        USIDR = pot2_val;
    } 
    else if (transmission_step == 4) {
        // Calculate and load CRC over the 3 payload bytes
        uint8_t payload[3] = {(pot_selection << 1) | on_off_status, pot1_val, pot2_val};
        USIDR = calculate_crc8(payload, 3);
    }
    
    // Clear USI overflow flag to prepare for the next byte 
    USISR = (1 << USIOIF); 
}

// ---------------------------------------------------------
// Main Execution Loop
// ---------------------------------------------------------

int main(void) {
    io_init();
    adc_init();
    usi_init();
    
    // Enable global interrupts
    sei(); 
    
    while (1) {
        // Continuously read both potentiometers
        pot1_val = adc_read(0); // ADC0
        pot2_val = adc_read(1); // ADC1

          // Handle Switch 1 Debounce
        if (sw1_flag) {
            _delay_ms(20); // Safe to delay here!
            
            // Verify it is still LOW after the delay (valid press)
            if (!(PIN_SW & (1 << PIN_SW1))) {
                on_off_status ^= 1; // Toggle the status
            }
            sw1_flag = 0; // Clear the flag so it waits for the next press
        }
        
        // Handle Switch 2 Debounce
        if (sw2_flag) {
            _delay_ms(20); // Safe to delay here!
            
            // Verify it is still LOW after the delay (valid press)
            if (!(PIN_SW & (1 << PIN_SW2))) {
                pot_selection ^= 1; // Toggle the pot selection
            }
            sw2_flag = 0; // Clear the flag so it waits for the next press
        }
        
        // Update Red LED based on On/Off status
        if (on_off_status) {
            if (!(PIN_LED & (1 << PIN_LED_RED))) PORT_LED |= (1 << PIN_LED_RED);
        } else {
            if (PIN_LED & (1 << PIN_LED_RED)) PORT_LED &= ~(1 << PIN_LED_RED);
        }
        
        // Alternate RG LED based on Pot Selection
        if (pot_selection == 0) {
            // Pot 1 selected: Turn Red ON, Green OFF
            if (PIN_LED & (1 << PIN_LED_RG_G)) {
                PORT_LED |= (1 << PIN_LED_RG_R);
                PORT_LED &= ~(1 << PIN_LED_RG_G);
            }
        } else {
            // Pot 2 selected: Turn Red OFF, Green ON
            if (PIN_LED & (1 << PIN_LED_RG_R)) {
                PORT_LED &= ~(1 << PIN_LED_RG_R);
                PORT_LED |= (1 << PIN_LED_RG_G);
           }
        } 
        
        // Small delay to regulate the main loop speed
        _delay_ms(10);
    }
    
    return 0;
}
