# Introduction
This is a fork of the project [deej](https://github.com/omriharel/deej), I added SSD1306 display support.

## How to use
1. Open your Arduino IDE and go to Sketch > Include Library > Manage Libraries. The Library Manager should open.
2. Type “SSD1306” in the search box and install the SSD1306 library from Adafruit.
3. It will prompt a message to also install other libraries. Install those libraries.

## Note
In the code there is a ```&Wire``` variable that seems odd, looks like the IDE can configure automatically the I2C Pins, in my case (I use the [RP2040-Zero](https://www.waveshare.com/wiki/RP2040-Zero)) the pins I use are 4 and 5.

## Caution
This is a "Dumb" implementation, keep in mind that is a work in progress, but feel free to suggest changes!!