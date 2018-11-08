#!/usr/bin/zsh
# This script is a simple example how to use basic linux tools to interact with the
# light controller. 

serial_port=/dev/ttyACM0

# configure the serial connection
stty -F $serial_port 9600 cs8 -cstopb -parenb
# send commands via echo

## Available Commands ##
########################
# GetConfig
# GetClock (alias for GetConfig)
# SetSunset <hour> <minute>
# SetClock <day> <weekday> <month> <year> <hour> <minute> <second>
# SetLightDuration <hours> <minutes> <sunrise-duration-minutes> <sunset-duration-minutes>
# SetMaxBrightness <channel0> <channel1> <channel3>
# SetDelays <Sunset Delay ch0> <Sunset Delay ch1> <Sunset Delay ch2>   
# SetNestOffset <nest open sunset offset> <nest close sunset offset>

echo -n "#SetLightDuration 11 30 45 75;" > $serial_port
echo -n "#SetMaxBrightness 178 178 53;" > $serial_port
echo -n "#SetDelays 0 10 20;" > $serial_port


