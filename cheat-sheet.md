# Stallsteuerung

Die Steuerung ist erreichbar mittles eines Raspberry Pi's. Dieser ist errichbar per SSH unter dem namen stall-raspi und lüft mit Debian.

ssh flo@stall-raspi

oder

ssh flo@stall-raspi.trabauer.org

Die Steuerung selbst ist per USB Verbunden und stellt eine Art CLI zur Verfügung. Jedes Commando startet mit dem **#**-Symbol und endet mit dem **;** Symbol.

## Installierte tools am System

 - picocom
 - tmux
 - neovim
 - zsh

Das moderne Standard Toolset

# picocom 

ist installiert. 


```picocom /dev/ttyACM0``` 

startet und verbindet sich mit der Steuerung des Stalls 

picocom hat eine Leader Key Combo um es zu steuern. 

Control + A 

C-a ist die LeaderKey Combo

Nachdem man den Leader Key gedrückt hat folgs das Commando an picocom

C-x beendet picocom

Mehr Infos findet man in der manpage von picocom. 

```man picocom```


Die Steuerung ist auf 9600 Baud 8 Datenbits 1 Stoppbit eingestellt. 


# Die wichtigsten Commandos


```#SetClock [day] [weekday] [month] [year] [hour] [minute] [second];```

Beispiel

2026-01-22 7:53:00 Dienstag

```#SetClock 22 2 1 26 7 53 0;```

***Achtung! Das Jahr ohne das Jahrtausend angeben. Das Programm verwendet ein Byte (uint8t)***

```#SetBirthday [year] [month] [day];```

Das kann einfach mit python (als Taschenrechner ausgerechnet werden)

Am Lieferschein ist immer die Abweichung des Alters in Tagen angegeben. z.B. 18 Wochen -3 Tage 

```
import datetime
datetime.datetime.today()-datetime.timedelta(weeks=18, days=-3) 
```

Berechnet das Geburtsdatum der Hühner. 




# Commandos der Steuerung


```#SetFeedMotorTimeout [runtime in seconds];```

This prevets the feed motor from running all time. It limits the duration
the motor runs. When the timeout is reached the motor stopps until the reset
button is pressed. Also an alarm lamp is switched on to indicate an issue.


```#SetGateOffset [Sunrise Offset] [Sunset Offset];```

Both Sunrise Offset and Sunset Offset are integers. These define when the 
Gate is opened and closed based on the current sunset and sunrise. Offset is
specified in minutes.



```#SetWaterFlushDuration [Duration in minutes];```

This defines how long the Water is flushed trough the drinking lines when
the flush Button is pressed. The Button can be pushed again to stop the 
flushing.



```#GetWaterFlushDuration;```

Prints out the current Water Flush Duration Setting.



```#SetSunset [hour] [minute];```

Sets the sunsset time.


```#SetSunset;```

Gets the sunsset time and prints it out 



```#SetLocation  [latitude] [longitude];```

Sets the postion to the given gps coordinates



```#GetLocation;```

Gets the location prints it out 


```#SetTimezone  [timezone];```

Sets the timezone to the given value. expects a float.


```#GetTimezone;```

Gets the timezone prints it out 


```#AutomaticSunsetTime  [0]|[1];```

Enables or disables the automatic sunset time feature

```#GetAutomaticSunsetTime;```

Prints the AutomaticSunset configuration.

```#SetAutomaticSunsetOffset [Offset];``` 

Enables or disables the automatic sunset time feature. This is a offset between the real sunset
and the sunset of the light in the chicken coop. 

```#GetAutomaticSunsetOffset;``` 

prints automatic sunset time feature setting

```#GetDaylightsavingtime  [0]|[1]```

Enables or disables daylightsavingtime (to calculate the correct sunset)

```#GetDaylightsavingtime;``` 

prints daylightsavingtime setting

```#SetLightDuration [hours] [minutes];```

Sets the length of the day (in the chickehouse).

```#GetLightDuration;```

prints the length of the day setting.

```#SetAgeBasedLightDuration [age] [hours] [minutes];```

Sets the length of the day (in the chickehouse) based on the age in weeks of the hen

```#SetBirthday [year] [month] [day];```

Sets the brithday of the hens. This is needed if you want autmaticly adjust the light duration based
on the age of the hens.

```#AutomaticLightDuration 0|1;```

Sets the length of the day (in the chickehouse).


```#SetSunriseDuration [minutes];```

Sets the duration off the dimming phase. The duration is limited between 15
and 120 minutes.

```#SetSunsetDuration [minutes];```

Sets the duration off the dimming phase. The duration is limited between 15
and 120 minutes.

```#SetMaxBrightness [percent] [percent] [percent];```

Sets the maximum brightness for each channel. The PLC has three channels
Each channel is connected to a different light dimmer.


```#SetMaxBrightness [percent] [channel nr];```

Sets the maximum brightness for a specific channel. The PLC has three channels
Each channel is connected to a different light dimmer.


```#SetClock [day] [weekday] [month] [year] [hour] [minute] [second];```

Sets the internal Clock to the given date and time.

```#Status;```

Prints Settings and Internal Stuff for debugging


```#SetDelays  [SunriseDelay 0] [SunriseDelay 1] [SunriseDelay 2][SunSetDelay 0] [SunSetDelay 1] [SunSetDelay 2];```

Sets the internal Clock to the given date and time.


```#SetSSDelay  [Sunset Delay in minutes] [Channel Nr];```

Sets the internal Clock to the given date and time.

```#SetSRDelay  [Sunset Delay in minutes] [Channel Nr];```

Sets the internal Clock to the given date and time.

```#SetNestOffset  [NestOpenSunsetOffset in minutes] [NestCloseSunsetOffset];```

Sets the internal Clock to the given date and time.


```#GetCurrentLightBrightness  [channel];```

prints out the current brightness off a channel


```#FreezeTimeTo [hour] [minute] [second];```

freezes the time to given time. This can be used to test the time based stuff.


```#MoveGateManual [0 or 1]```

Open Gate Manual

```#MoveGateAutomatic;```

resumes the Gate Automatic Control


```#UnfreezeTime;```

resumes the time to the current RTC time.




