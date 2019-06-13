# This Projects is an arduino based controller for a chicken barn.


## Hardware 
We use a BigDutchman System for our chickens. And a Arduino based PLC that is called Controllino.
Right now the Sketch is written for the Controllino Automation Model. It will be extended in the future
to work with any other Model to that is based on the Arduino Mega (2560).

The sketch could be adopted to run with any other Arduino based PLC. 

Right now this controller controlls the nest mechanism over a digital out (relay) and the light in the
barn over an analog out 0..10V. 

## Used Libraries
 - [MegunoLink]('https://www.megunolink.com/')
   - [CommandHandler]('https://www.megunolink.com/documentation/arduino-libraries/serial-command-handler/')
   - [EEPROMStore]('https://www.megunolink.com/documentation/arduino-libraries/eepromstore/')
   - [Arduinotimer]('https://www.megunolink.com/documentation/arduino-libraries/arduino-timer/')
 - [Controllino]('https://controllino.biz/')
   - [Controllino-Library]('https://github.com/CONTROLLINO-PLC/CONTROLLINO_Library')
