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

Turn the light on or off and sets it to manual mode. use the LightAutomatic Command to reset the PLC to the automatic mode. 
```
LightManual 0|1
```

Set the light to automatic mode.
```
LightAutomatic
```

Set the time of the sunset beginning
```
SetSunset [hour] [minute]
```

Set the clock to the given time
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
 - neovim (I use neovim)
 - nano
 - vim

clone this Repo to the machine


## Install arduino-cli

```
cd ~/.local
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

```

Add ~/.local/bin to your $PATH variable if necessary.

(see Arch Wiki)[https://wiki.archlinux.org/title/Environment_variables#Examples]


```
arduino-cli config init --additional-urls 'https://raw.githubusercontent.com/CONTROLLINO-PLC/CONTROLLINO_Library/master/Boards/package_ControllinoHardware_index.json' --overwrite --verbose
arduino-cli lib install CONTROLLINO MegunoLink Dusk2Dawn Ethernet
arduino-cli core install CONTROLLINO_Boards:avr arduino:avr
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

Now you should be able to compile and upload the Sketch


## Raspberry Pi Setup

I use the following Tools on all of my machines. This tools are optional thats the way I setup every machine that I use.

diff-so-fancy is not in the Repos of Debian Trixie. 

I clone it to the Downloads folder 

```
git clone git@github.com:so-fancy/diff-so-fancy.git
ln -s /home/flo/Downloads/diff-so-fancy/diff-so-fancy /home/flo/.local/bin/diff-so-fancy
```

Also neovim is very old.

```cd ~/Downloads
wget https://github.com/neovim/neovim/releases/latest/download/nvim-linux-arm64.tar.gz
tar xf nvim-linux-arm64.tar.gz                                                                                                                                                                                 130 ↵
cp -av ~/Downloads/nvim-linux-arm64/* ~/.local/
```


Other tools I use
```
sudo apt install eza ripgrep kitty-terminfo tmux zsh npm nodejs
```

Clone mine or your dotfiles.

```
git clone
cd ~/dotfiles/
./install
source ~/.zshrc
```

