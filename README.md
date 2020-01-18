# Simple EPROM programmer

This programmer is based on MCUDude Arduino Major/Mighty Core and can program 27C64, 27C128, 27C256 chips (with "C"!).

# Features

* Upload and download data over 5V UART in Intel HEX format with you favorite serial communication program.

* Small count of components.

* Based on wide range of cheap microcontrollers.

# Schematic

Compatible microcontrollers are all https://github.com/MCUdude/MajorCore and https://github.com/MCUdude/MightyCore (standard pinout) ATmegas.

Tested with 12MHz crystal - 8MHz (lesser pre-built MCUDude core profile) may be too slow.

You can find example schematic that uses ATmega8515 in kicad folder:

![Schematic](https://github.com/dvarkin-ru/eprom_programmer/blob/master/kicad/programmer.png)
![Image](https://github.com/dvarkin-ru/eprom_programmer/blob/master/image.jpg)

# Usage

1. Connect device to PC over any UART converter @ 9600 baud.

2. In your serial communication program (minicom as example) press inserted chip code (w/o Enter):

 "a" - 27C64,
 
 "b" - 27C128,
 
 "c" - 27C256.
 
3. Modes:

  * For erase verify press "f": if all bytes are 0xFF - chip is clear.
  * For dump the chip press "r" and copy HEX to .hex file (or Ctrl-A L in minicom after step 2).
  * For write press "w", check Vpp (correct it before with R8 w/o chip) and paste HEX file content .
  * For verify press "v" and paste HEX file content (or Ctrl-A S "ascii" in minicom).

 ## Possible error messages:

  * Bad HEX : error on HEX reception.
  * Bad 0xXXXX : count of unsucsessful write attemts in 0xXXX0 block =10 and last "bad" byte is 0xXXXX
  (or first mismatch in verify mode).
  * Huge HEX : HEX record must be no more than 16 bytes of HEX data.
  * UNK REC TYPE : unknown HEX record type.
  * CHKSUM ERR @ XXXX : HEX checksum error.
  
  # See also
  
  * This projest is based on https://github.com/walhi/arduino_eprom27_programmer
  * 27C512 programmer @ http://kovlev.ru/tools.html
  * objcopy (GNU tool, part of Arduino IDE as avr-objcopy) - convert from binary to ihex and vice-versa.
