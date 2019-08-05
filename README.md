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
