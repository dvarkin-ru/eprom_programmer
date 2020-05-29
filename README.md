# Simple EPROM programmer

This programmer is based on MCUDude Arduino Major/Mighty Core and can program 2716, 27C16, 2764, 27C64, 27128, 27C128, 27256, 27C256 chips.

# Features

* Upload and download data over 5V UART in Intel HEX format.

* Small count of components.

* Based on wide range of cheap microcontrollers.

# Schematic

Compatible microcontrollers are all https://github.com/MCUdude/MajorCore and https://github.com/MCUdude/MightyCore (standard pinout) ATmegas.

Tested with 12MHz crystal.

You can find example schematic that uses ATmega8515 in kicad folder:

![Schematic](https://github.com/dvarkin-ru/eprom_programmer/blob/master/kicad/programmer.png)
![Image](https://github.com/dvarkin-ru/eprom_programmer/blob/master/image.jpg)

# Usage

1. Connect device to PC over any UART converter @ 57600 baud.

2. In your serial communication program (minicom) press inserted chip code (w/o Enter):

 "a" - 27C16,
 
 "b" - 27C64,
 
 "c" - 27C128,
 
 "d" - 27C256.

 If you want to program 2716, 2764, 27128 or 27256 press "N" and limit char tx delay to 100 ms (Ctrl-A T F in minicom).

3. Modes:

  * For erase verify press "f": if all bytes are 0xFF - chip is clear.
  * For dump the chip press "r" and copy HEX to .hex file (or Ctrl-A L in minicom after step 2).
  * For write press "w", check Vpp (correct it before w/o chip) and paste HEX file content .
  * For verify press "v" and paste HEX file content (or Ctrl-A S "ascii" in minicom).

 ## Possible error messages:

  * Bad HEX : error on HEX reception.
  * Bad 0xXXXX : count of unsucsessful write attemts in 0xXXX0 block =10 and last "bad" byte is 0xXXXX
  (or first mismatch in verify mode).
  * Huge HEX : HEX record must be no more than 16 bytes of HEX data.
  * UNK REC TYPE : unknown HEX record type.
  * CHKSUM ERR @ XXXX : HEX checksum error.
  * TOO HIGH ADDR : address of HEX record last byte out of selected chip memory space.
  
  # See also
  
  * This projest is based on https://github.com/walhi/arduino_eprom27_programmer
  * 27C512 programmer @ http://kovlev.ru/tools.html
  * objcopy (GNU tool, part of Arduino IDE as avr-objcopy) - convert from binary to ihex and vice-versa.
