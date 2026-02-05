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

 - [Dusk2Dawn]('https://github.com/dmkishi/Dusk2Dawn')
   The Version 1.0.1 doesn't compile on Linux. To compile the sketch on Linux clone the latest version from Github to your libraries folder. 


# PLC Parameters and Comands

## NestControllerConfiguration
uses the following internal parameters.
- Parameters:
 - NestOpenSunsetOffset
     - (default +5 hours)
 - NestCloseSunsetOffset
     - (default 0 hours)

internaly it stores the offset in seconds as a `long` (the internal timer converts the current time into a timestamp in seconds)


### Commands
```SetNestOffset [nest open offset] [nest close offset]```

defines when the Nest opens and closes. It accespts an integer number in minutes as parameter


## LightControllerConfiguration

- Parameters:
 - SunsetTime
 - SunsetDuration
 - SunriseDuration
 - LightDuration
 - MaxBrightness0
 - MaxBrightness1
 - MaxBrightness2
 - SSDelayA0
 - SSDelayA1
 - SSDelayDO0
 - SRDelayA0
 - SRDelayA1
 - SRDelayDO0

### Light Control Commands

Each Command is startet with a `#` and ended with a `;`

to show the current Settings
```
GetClock
GetConfig
```


```
LightManual 0|1
```

```
LightAutomatic
```


```
SetSunset [hour] [minute]
```

```
SetClock [day] [weekday] [month] [year] [hour] [minute] [second]
```


This sets the light duration to the given values in minutes (integer)
```
SetLightDuration [hours] [minutes]
```

This sets the sunset duration to the given values in minutes (integer)
```
SetSunsetDuration [minutes]
```

This sets the sunrise duration to the given values in minutes (integer)
```
SetSunriseDuration [minutes]
```

To define the max brightness you have to specifie an integer between 0 and 100 for each channel
```
SetMaxBrightness [brightness ch1] [brightness ch2] [brightness ch3]
```

To define the max brightness you have to specifie an integer between 0 and 100 for the given channel (0-2)
```
SetMaxBrightness [brightness value] [channel number]
```

```
SetDelays [delay channel 1] [delay channel 2] [delay channel 3]
```
integer value delay in minutes

## WaterControllerConfiguration (Not implemented yet)
    -

# PLC Serial commands

# cclc-client.py
Is a small python script to configure the chicken-coop-light-controller

It requires [python3]() and [pyserial](https://pythonhosted.org/pyserial/)


# Remote Development Setup

A RaspberryPi or similar can be used to access the controller remotely. Usefull are the following tools.

 - tmux
 - openssh


your preferred text editor

 - emacs
 - neovim
 - nano
 - vim


arduino-cli installieren

https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh

```
arduino-cli config init --additional-urls 'https://raw.githubusercontent.com/CONTROLLINO-PLC/CONTROLLINO_Library/master/Boards/package_ControllinoHardware_index.json' --overwrite --verbose
arduino-cli lib install CONTROLLINO
arduino-cli lib install MegunoLink
arduino-cli lib install Dusk2Dawn
arduino-cli lib install Ethernet
arduino-cli core install CONTROLLINO_Boards:avr
arduino-cli core install arduino:avr
```

Compiling and uploading the sketch


```
arduino-cli compile --fqbn CONTROLLINO_Boards:avr:controllino_maxi
arduino-cli upload /home/flo/chicken-coop-controller -p /dev/ttyACM0 -b CONTROLLINO_Boards:avr:controllino_maxi
```

Dusk2Dawn has a bug. It works on Windows but the include is wrong. 


```
sed 's/Math.h/math.h/' ~/Arduino/libraries/Dusk2Dawn/Dusk2Dawn.cpp > ~/Arduino/libraries/Dusk2Dawn/Dusk2Dawn.cpp
sed 's/Math.h/math.h/' ~/Arduino/libraries/Dusk2Dawn/Dusk2Dawn.h > ~/Arduino/libraries/Dusk2Dawn/Dusk2Dawn.h
```

