# Amp-Reverb-Control
Ever wish that you could control your amplifier's reverb effect on the fly without major modification to your amplifier? Well now your wish can become a reality! 

This is a reverb control platform that can be placed into an amplifier and has a remote pedal that controls the reverb effect. 

It has two parts: the master mcu unit that goes into the amplifier and a slave mcu unit in a pedal. You replace the reverb pot in the amplifier with the digital one on the PCB so the audio signal stays inside of the amplifier. The existing reverb pot -- if it is a 100kA pot-- connects to the master mcu board to control the reverb locally if the pedal is not plugged in. 
Connect the master mcu to the pedal mcu via a 6 pin GX12 cable and you are now able to remotely control the reverb. You can select between two different levels and turn the effect on or off. 

## The Build

The mcu on both boards is an Atmel ATtiny26L. The PCB boards were designed to be easy to install into an amplifier and a pedal. 

### AMP PCB

![PCB of the Master MCU](https://github.com/user-attachments/assets/38d03010-a9dd-4948-8d6f-b7264e759740) 

### Pedal PCB 

<img width="654" height="466" alt="Screenshot 2026-06-22 at 11 38 00 AM" src="https://github.com/user-attachments/assets/26b5bde9-525a-4d88-9d88-888aa9a15acc" />
<img width="3024" height="4032" alt="IMG_1856" src="https://github.com/user-attachments/assets/a524fc26-e8b5-430c-8520-41982746f9c6" />

### Pedal Assembly
<img width="1081" height="608" alt="Screenshot 2026-06-11 at 2 40 46 PM" src="https://github.com/user-attachments/assets/e5b2a410-f1b5-4b91-adf3-a603eb471b1b" />
<img width="1058" height="572" alt="Screenshot 2026-06-11 at 2 40 34 PM" src="https://github.com/user-attachments/assets/02cef310-be4f-4beb-81ac-962bbe457029" />

### Full photo of the master mcu installed in the amp
<img width="3024" height="4032" alt="IMG_2324" src="https://github.com/user-attachments/assets/c53ccf79-e301-437b-af64-6a6dc250827b" />

### Close up of the master MCU showing the audio signal path. Red is input to the digital pot and blue is output of the digital pot. 
<img width="1325" height="1350" alt="IMG_2324 copy" src="https://github.com/user-attachments/assets/8393bde1-38b6-4b60-ab51-0b8426ba9f34" />

### Real pics of pedal
<img width="4028" height="2311" alt="IMG_2333" src="https://github.com/user-attachments/assets/d014f28b-3f06-4176-9636-c1f390d54739" />

## Schematic for connecting the master MCU 

### Signal Connection
<img width="918" height="527" alt="Screenshot 2026-10-20 at 3 49 12 PM" src="https://github.com/user-attachments/assets/ed292f4e-0dad-48ac-8113-3d33ddad3f9b" />

### Power Connection
<img width="1240" height="288" alt="Screenshot 2026-10-20 at 3 49 44 PM" src="https://github.com/user-attachments/assets/cb4eb627-167b-4534-a12d-c457b293b2ed" />

The 6 pin comms header connects to "Slave_MCU" header on both boards. 

## Flashing 

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
## Video of Pedal Demo
![Here](https://youtube.com/shorts/v62EIynH0uw)


