# Amp-Reverb-Control
Ever wish that you could control your amplifier's reverb effect on the fly without major modification to you amplifier? Well now your wish can become a reality! 

This is a reverb control platform that can be placed into an ampilifer and has a remote pedal that controls the reverb effect. 

It has two parts: the master mcu unit that goes into the amplifier and a slave mcu unit in a pedal. You replace the reverb pot in the amplifier with the digital one on the PCB so the audio signal stays inside of the ampilifer. The exisiting reverb pot -- if it is 100kA -- to the master mcu board. 
Connect the master mcu to the pedal mcu via a 6 pin GX12 cable and you are now able to remotely control the reverb. You can select between two different levels and turn the effect on or off. 

## The Build

The mcu on both boards is an Atmel ATtiny26L. The PCB boards were designed to be easy to install into an amplifier and a pedal. 

### AMP PCB

![PCB of the Master MCU](https://github.com/user-attachments/assets/38d03010-a9dd-4948-8d6f-b7264e759740) 

### Pedal PCB 

<img width="654" height="466" alt="Screenshot 2026-06-22 at 11 38 00 AM" src="https://github.com/user-attachments/assets/26b5bde9-525a-4d88-9d88-888aa9a15acc" />

### Pedal Assembly
<img width="1081" height="608" alt="Screenshot 2026-06-11 at 2 40 46 PM" src="https://github.com/user-attachments/assets/e5b2a410-f1b5-4b91-adf3-a603eb471b1b" />
<img width="1058" height="572" alt="Screenshot 2026-06-11 at 2 40 34 PM" src="https://github.com/user-attachments/assets/02cef310-be4f-4beb-81ac-962bbe457029" />

### Real pics of pedal and inside of amp

## Schematic for conneting the master MCU 

## Coding 

To flash the mcus be sure to use the correct `.hex` file for the associated board. Connect your flasher to the ISP header. 

First update the fuses

```
avrdude -c usbtiny -p t26 -U lfuse:w:0xc4:m -U hfuse:w:0xf6:m
```

Flashing the master board
```
avrdude -c usbtiny -p t26 -U flash:w:amp_master_rev1.hex
```

Flashing the pedal board 
```
avrdude -c usbtiny -p t26 -U flash:w:pedal_slave_rev1.hex
```
