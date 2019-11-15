/*
* Changes the DEBUG Output.
* Right now it is a integer that represents a Level.
*/
#define DEBUG_OUTPUT 0
/*
* This disables the clock and mocks it with millis(), for testing purposes
* it is usefull. 1 day runs in about 864s (14min and 24s)
*/
#define MOCK_CLOCK 0

#define VERSION "v0.02"

#include "MegunoLink.h"
#include "CommandHandler.h"
#include "EEPROMStore.h"
#include "ArduinoTimer.h"
#if defined(CONTROLLINO_MAXI)
  #include "Configuration_Controllino_Maxi.h"
#elif defined(CONTROLLINO_MAXI_AUTOMATION)
  #include "Configuration_Controllino_Maxi_Automation.h"
#else 
  #error Please, select one of the supported CONTROLLINO variants in Tools->Board
#endif

struct NestControllerConfiguration
{
    int32_t NestOpenSunsetOffset;
    int32_t NestCloseSunsetOffset;
    void Reset() {
        NestOpenSunsetOffset = 5l*60l*60l; // 5 hours after sunset
        NestCloseSunsetOffset = 0l*60l*60l; // 0 hour before sunset
    }
};


struct GateControllerConfiguration
{
  int32_t GateOpenSunriseOffset;
  int32_t GateCloseSunsetOffset;
  void Reset() {
    GateOpenSunriseOffset = 1l*60l*60l;
    GateCloseSunsetOffset = 2l*60l*60l;
  }
};


struct FeedControllerConfiguration
{
  unsigned long FeedMotorTimeoutMillis;
  void Reset() {
    FeedMotorTimeoutMillis = 5ul*60ul*1000ul;
  }
};


struct LightControllerConfiguration
{
  uint32_t SunsetTime;
  uint32_t SunsetDuration;
  uint32_t SunriseDuration;
  uint32_t LightDuration;
  int MaxBrightness0;
  int MaxBrightness1;
  int MaxBrightness2;
  uint32_t SSDelayA0;
  uint32_t SSDelayA1;
  uint32_t SSDelayDO0;
  uint32_t SRDelayA0;
  uint32_t SRDelayA1;
  uint32_t SRDelayDO0;
  void Reset() {
    SunsetTime = 17ul*60ul*60ul;
    SunsetDuration = 75ul*60ul;
    SunriseDuration = 30ul*60ul;
    LightDuration = 14ul*60ul*60ul;
    MaxBrightness0 = 229;
    MaxBrightness1 = 229;
    MaxBrightness2 = 95;
    SSDelayA0 = 0ul;
    SSDelayA1 = 10ul*60ul;
    SSDelayDO0 = 20ul*60ul;
    SRDelayA0 = 0ul;
    SRDelayA1 = 10ul*60ul;
    SRDelayDO0 = 20ul*60ul;
  }
};


struct WaterControllerConfiguration
{
    unsigned long WaterFlushDuration;
    void Reset() {
        WaterFlushDuration = 1ul*60ul*1000ul; // 1 Minutes
    }
};

uint32_t timestamp = 0ul;
uint32_t sunriseTime;
const uint32_t one_day = 24ul*60ul*60ul;
int waterBtnRead = 0;
unsigned long waterBtnPressedTime = 0;
unsigned long motorSwitchedOnMillis = 0;
bool alarmState = false;
bool feedAlarmState = false;
bool freezeTime = false;
bool manualGateControl = false;
int currentBrightnessCh0 = 0;
int currentBrightnessCh1 = 0;
int currentBrightnessCh2 = 0;

EEPROMStore<LightControllerConfiguration> LightCfg;
EEPROMStore<NestControllerConfiguration> NestCfg;
EEPROMStore<WaterControllerConfiguration> WaterCfg;
EEPROMStore<GateControllerConfiguration> GateCfg;
EEPROMStore<FeedControllerConfiguration> FeedCfg;
CommandHandler<22, 35, 7> SerialCommandHandler(Serial,'#',';');
ArduinoTimer UpdateAoTimer;

#if MOCK_CLOCK == 1
  unsigned int timestamp_hour = 0;
#endif



/*
  This function defines the SetFeedMotorTimeout Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetMaximumFeedMotorRuntime [runtime in seconds];
  
  This prevets the feed motor from running all time. It limits the duration
  the motor runs. When the timeout is reached the motor stopps until the reset
  button is pressed. Also an alarm lamp is switched on to indicate an issue.

*/
void Cmd_SetFeedMotorTimeoutMillis(CommandParameter &Parameters) { 
  FeedCfg.Data.FeedMotorTimeoutMillis = Parameters.NextParameterAsInteger(1)*1000ul; //runtime in seconds 
  FeedCfg.Save();
}

/*
  This function defines the SetGateOffset Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetGateOffset [Sunrise Offset] [Sunset Offset];

  Both Sunrise Offset and Sunset Offset are integers. These define when the 
  Gate is opened and closed based on the current sunset and sunrise. Offset is
  specified in minutes.
*/
void Cmd_SetGateOffsets(CommandParameter &Parameters) { 
  GateCfg.Data.GateOpenSunriseOffset = Parameters.NextParameterAsInteger(1)*60ul; //offset in Minutes 
  GateCfg.Data.GateCloseSunsetOffset = Parameters.NextParameterAsInteger(1)*60ul; //offset in Minutes
  GateCfg.Save();
}


/*
  This function defines the SetWaterFlushDuration Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetWaterFlushDuration [Duration in minutes];

  This defines how long the Water is flushed trough the drinking lines when
  the flush Button is pressed. The Button can be pushed again to stop the 
  flushing.
*/
void Cmd_SetWaterFlushDuration(CommandParameter &Parameters) {
  uint32_t water_fd = Parameters.NextParameterAsInteger(10); //duration in minutes
  WaterCfg.Data.WaterFlushDuration = water_fd*60ul*1000ul;
  WaterCfg.Save();
}


/*
  This function defines the GetWaterFlushDuration Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #GetWaterFlushDuration;

  Prints out the current Water Flush Duration Setting.
*/
void Cmd_GetWaterFlushDuration(CommandParameter &Parameters) {
  Parameters.GetSource().print(WaterCfg.Data.WaterFlushDuration/(60ul*1000ul));
  Parameters.GetSource().println(F(" minutes"));
}


/*
  This function defines the SetSunset Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetSunset [hour] [minute];

  Sets the sunsset time.
*/
void Cmd_SetSunset(CommandParameter &Parameters)
{
  uint32_t ss_hour = Parameters.NextParameterAsInteger(19);
  uint32_t ss_minute = Parameters.NextParameterAsInteger(15);
  LightCfg.Data.SunsetTime = (ss_hour*60ul*60ul+ss_minute*60ul)%one_day;
  LightCfg.Save();
}


/*
  This function defines the SetLightDuration Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetLightDuration [hours] [minutes];

  Sets the length of the day (in the chickehouse).
*/
void Cmd_SetLightDuration(CommandParameter &Parameters)
{
  uint32_t lightd_hours = Parameters.NextParameterAsInteger(9);
  uint32_t lightd_minutes = Parameters.NextParameterAsInteger(30);
  if(lightd_hours>15 || lightd_hours < 0 || lightd_minutes < 0 || lightd_minutes > 59){
      Parameters.GetSource().println(F("Invalid light duration! "));
      return;
  } else {
      LightCfg.Data.LightDuration = lightd_minutes*60ul+lightd_hours*60ul*60ul;
      LightCfg.Save();
  }
}


/*
  This function defines the SetSunriseDuration Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetSunriseDuration [minutes];

  Sets the duration off the dimming phase. The duration is limited between 15
  and 120 minutes.
*/
void Cmd_SetSunriseDuration(CommandParameter &Parameters)
{
  uint32_t sr_duration = Parameters.NextParameterAsInteger(30);
  if(sr_duration < 15 || sr_duration > 120) {
      Parameters.GetSource().println(F("Invalid sunrise duration! Durations between 15 and 120 minutes allowed!"));
      return;
  } else {
      LightCfg.Data.SunriseDuration = sr_duration*60ul;
      LightCfg.Save();
  }
}


/*
  This function defines the SetSunsetDuration Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetSunsetDuration [minutes];

  Sets the duration off the dimming phase. The duration is limited between 15
  and 120 minutes.
*/
void Cmd_SetSunsetDuration(CommandParameter &Parameters)
{
  uint32_t ss_duration = Parameters.NextParameterAsInteger(45);
  if(ss_duration < 15 || ss_duration > 120) {
      Parameters.GetSource().println(F("Invalid sunset duration! Durations between 15 and 120 minutes allowed!"));
      return;
  } else {
      LightCfg.Data.SunsetDuration = ss_duration*60ul;
      LightCfg.Save();
  }
}


/*
  This function defines the SetMaxBrightness Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetMaxBrightness [percent] [percent] [percent];

  Sets the maximum brightness for each channel. The PLC has three channels
  Each channel is connected to a different light dimmer.
*/
void Cmd_SetMaxBrightness(CommandParameter &Parameters)
{
  int user_p0 = Parameters.NextParameterAsInteger(178);
  int user_p1 = Parameters.NextParameterAsInteger(178);
  int user_p2 = Parameters.NextParameterAsInteger(53);
  if(user_p0<0 || user_p0 > 100) {
    Parameters.GetSource().
    	println(F("First parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
    user_p0 = 75;
  }
  if(user_p1<0 || user_p1 > 100) {
    Parameters.GetSource().
    	println(F("Second parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
    user_p1 = 75;
  }
  if(user_p2<0 || user_p2 > 100) {
    Parameters.GetSource().
    	println(F("Third parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
    user_p2 = 75;
  }
  LightCfg.Data.MaxBrightness0 = user_p0*255/100;
  LightCfg.Data.MaxBrightness1 = user_p1*255/100;
  LightCfg.Data.MaxBrightness2 = user_p2*106/100;
  LightCfg.Save();
}


/*
  This function defines the SetMaxBrightness Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetMaxBrightness [percent] [channel nr];

  Sets the maximum brightness for a specific channel. The PLC has three channels
  Each channel is connected to a different light dimmer.
*/
void Cmd_SetMaxBrightnessForChannel(CommandParameter &Parameters)
{
  int value = Parameters.NextParameterAsInteger(178);
  int channelNr = Parameters.NextParameterAsInteger(0);
  if(value<0 || value > 100) {
    Parameters.GetSource().
    	println(F("First parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
    value = 75;
  }
  if(channelNr<0 || channelNr > 2) {
    Parameters.GetSource().
    	println(F("Channel parameter out of range (values between 0 and 2 are allowed). "));
    channelNr = 0;
  }
  if(channelNr == 0) {
      LightCfg.Data.MaxBrightness0 = value*255/100;
  } else if(channelNr == 1) {
      LightCfg.Data.MaxBrightness1 = value*255/100;
  } else if(channelNr == 2) {
      LightCfg.Data.MaxBrightness2 = value*106/100; // diffrent ADC that creates a voltage between 0-24V
  } else {
      Parameters.GetSource().println(F("invalid channel"));
  }
  LightCfg.Save();
}

/*
  This function defines the SetClock Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetClock [day] [weekday] [month] [year] [hour] [minute] [second];

  Sets the internal Clock to the given date and time.
*/
void Cmd_SetClock(CommandParameter &Parameters)
{
  int aDay = Parameters.NextParameterAsInteger(1);
  int aWeekDay = Parameters.NextParameterAsInteger(1);
  int aMonth = Parameters.NextParameterAsInteger(1);
  int aYear = Parameters.NextParameterAsInteger(1);
  int aHour = Parameters.NextParameterAsInteger(1);
  int aMinute = Parameters.NextParameterAsInteger(1);
  int aSecond = Parameters.NextParameterAsInteger(1);
  if(Controllino_SetTimeDate(aDay, aWeekDay, aMonth, aYear, aHour, aMinute, aSecond) == 0) {
    Parameters.GetSource().println(F("Clock is set!"));
  } else {
    Parameters.GetSource().println(F("RTC library was not initialized before"));
  }
}


/*
  This function defines the GetConfig Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #GetConfig;

  Prints out all the Settings and Parameters off the PLC as a human readable
  text.
*/
void Cmd_GetConfig(CommandParameter &Parameters)
{
  #if MOCK_CLOCK
    Parameters.GetSource().println(F("MOCKED TIME!!!"));
    Parameters.GetSource().print(timestamp/60ul/60ul);
    Parameters.GetSource().print(F(":"));
    Parameters.GetSource().println(timestamp/60ul%60ul);
  #else
    Controllino_PrintTimeAndDate();
  #endif
  Parameters.GetSource().print(F("timestamp = "));
  Parameters.GetSource().println(timestamp);
  Parameters.GetSource().print(F("\nSunset @ "));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime/60ul/60ul);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime/60ul%60ul);
  Parameters.GetSource().print(F("("));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("Sunrise @ "));
  Parameters.GetSource().print(sunriseTime/60ul/60ul);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(sunriseTime/60ul%60ul);
  Parameters.GetSource().print(F("("));
  Parameters.GetSource().print(sunriseTime);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("\nNestCloseTime @ "));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset)/60ul/60ul);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset)/60ul%60ul);
  Parameters.GetSource().print(F(" ("));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("NestOpenTime @ "));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset)/60ul/60ul);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset)/60ul%60ul);
  Parameters.GetSource().print(F(" ("));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("\nGateCloseTime @ "));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + GateCfg.Data.GateCloseSunsetOffset)/60ul/60ul);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + GateCfg.Data.GateCloseSunsetOffset)/60ul%60ul);
  Parameters.GetSource().print(F(" ("));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime + GateCfg.Data.GateCloseSunsetOffset);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("GateOpenTime @ "));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime - LightCfg.Data.LightDuration + GateCfg.Data.GateOpenSunriseOffset)/60ul/60ul);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime - LightCfg.Data.LightDuration + GateCfg.Data.GateOpenSunriseOffset)/60ul%60ul);
  Parameters.GetSource().print(F(" ("));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime - LightCfg.Data.LightDuration + GateCfg.Data.GateOpenSunriseOffset);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("\nLight Duration: "));
  Parameters.GetSource().print(LightCfg.Data.LightDuration/60ul/60ul);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(LightCfg.Data.LightDuration/60ul%60ul);
  Parameters.GetSource().print(F("Sunrise duration is "));
  Parameters.GetSource().print(LightCfg.Data.SunriseDuration/60ul);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Sunset duration is "));
  Parameters.GetSource().print(LightCfg.Data.SunsetDuration/60ul);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Brightness Settings\nLight MaxBrightness0="));
  Parameters.GetSource().println(LightCfg.Data.MaxBrightness0);
  Parameters.GetSource().print(F("Light MaxBrightness1="));
  Parameters.GetSource().println(LightCfg.Data.MaxBrightness1);
  Parameters.GetSource().print(F("Light MaxBrightness2="));
  Parameters.GetSource().println(LightCfg.Data.MaxBrightness2);
  Parameters.GetSource().print(F("Light SunSetDelayA0="));
  Parameters.GetSource().print(LightCfg.Data.SSDelayA0/60ul);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Light SunSetDelayA1="));
  Parameters.GetSource().print(LightCfg.Data.SSDelayA1/60ul);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Light SunSetDelayDO0="));
  Parameters.GetSource().print(LightCfg.Data.SSDelayDO0/60ul);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Light SunRiseDelayA0="));
  Parameters.GetSource().print(LightCfg.Data.SRDelayA0/60ul);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Light SunRiseDelayA1="));
  Parameters.GetSource().print(LightCfg.Data.SRDelayA1/60ul);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Light SunRiseDelayDO0="));
  Parameters.GetSource().print(LightCfg.Data.SRDelayDO0/60ul);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("FeedMotorTimeoutMillis="));
  Parameters.GetSource().print(FeedCfg.Data.FeedMotorTimeoutMillis/1000ul);
  Parameters.GetSource().println(F(" seconds"));

}

/*
  This function defines the SetDelays Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetDelays  [SunriseDelay 0] [SunriseDelay 1] [SunriseDelay 2][SunSetDelay 0] [SunSetDelay 1] [SunSetDelay 2];

  Sets the internal Clock to the given date and time.
*/
void Cmd_SetDelays(CommandParameter &Parameters) {
  uint32_t delay3 = Parameters.NextParameterAsInteger(0);
  uint32_t delay4 = Parameters.NextParameterAsInteger(1);
  uint32_t delay5 = Parameters.NextParameterAsInteger(2);
  uint32_t delay0 = Parameters.NextParameterAsInteger(0);
  uint32_t delay1 = Parameters.NextParameterAsInteger(2);
  uint32_t delay2 = Parameters.NextParameterAsInteger(1);
  LightCfg.Data.SSDelayA0 = delay0*60ul;
  LightCfg.Data.SSDelayA1 = delay1*60ul;
  LightCfg.Data.SSDelayDO0 = delay2*60ul;
  LightCfg.Data.SRDelayA0 = delay3*60ul;
  LightCfg.Data.SRDelayA1 = delay4*60ul;
  LightCfg.Data.SRDelayDO0 = delay5*60ul;
  LightCfg.Save();
}


/*
  This function defines the SetSSDelay Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetSSDelay  [Sunset Delay in minutes] [Channel Nr];

  Sets the internal Clock to the given date and time.
*/
void Cmd_SetSSDelay(CommandParameter &Parameters) {
  uint32_t value = Parameters.NextParameterAsInteger(0);
  int channelNr = Parameters.NextParameterAsInteger(0);
  if(channelNr<0 || channelNr > 2) {
    Parameters.GetSource().
      println(F("Channel parameter out of range (values between 0 and 2 are allowed). "));
    channelNr = 0;
  }
  if(channelNr == 0) {
      LightCfg.Data.SSDelayA0 = value*60ul;
  } else if(channelNr == 1) {
      LightCfg.Data.SSDelayA1 = value*60ul;
  } else if(channelNr == 2) {
      LightCfg.Data.SSDelayDO0 = value*60ul;
  } else {
      Parameters.GetSource().println(F("invalid channel"));
  }
  LightCfg.Save();
}


/*
  This function defines the SetSRDelay Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetSRDelay  [Sunset Delay in minutes] [Channel Nr];

  Sets the internal Clock to the given date and time.
*/
void Cmd_SetSRDelay(CommandParameter &Parameters) {
    uint32_t value = Parameters.NextParameterAsInteger(0);
    int channelNr = Parameters.NextParameterAsInteger(0);
    if(channelNr<0 || channelNr > 2) {
      Parameters.GetSource().
        println(F("Channel parameter out of range (values between 0 and 2 are allowed). "));
      channelNr = 0;
    }
    if(channelNr == 0) {
        LightCfg.Data.SRDelayA0 = value*60ul;
    } else if(channelNr == 1) {
        LightCfg.Data.SRDelayA1 = value*60ul;
    } else if(channelNr == 2) {
        LightCfg.Data.SRDelayDO0 = value*60ul;
    } else {
        Parameters.GetSource().println(F("invalid channel"));
    }
    LightCfg.Save();
}


/*
  This function defines the SetNestOffset Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #SetNestOffset  [NestOpenSunsetOffset in minutes] [NestCloseSunsetOffset];

  Sets the internal Clock to the given date and time.
*/
void Cmd_SetNestSunsetOffset(CommandParameter &Parameters) {
    int32_t offset0 = Parameters.NextParameterAsInteger(5*60);
    int32_t offset1 = Parameters.NextParameterAsInteger(0);
    NestCfg.Data.NestOpenSunsetOffset = offset0*60l;
    NestCfg.Data.NestCloseSunsetOffset = offset1*60l;
    NestCfg.Save();
}

/*
  This function defines the GetCurrentLightBrightness Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #GetCurrentLightBrightness  [channel];

  prints out the current brightness off a channel
*/
void Cmd_GetCurrentLightBrightness(CommandParameter &Parameters) {
  int channel = Parameters.NextParameterAsInteger(0);
  if(channel == 0){
    Serial.print("channel 0 - current value = ");
    Serial.println(currentBrightnessCh0);
  }
  if(channel == 1){
    Serial.println(currentBrightnessCh1);
  }
  if(channel == 2){
    Serial.println(currentBrightnessCh2);
  }
}

/*
  This function defines the FreezeTimeTo Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #FreezeTimeTo [hour] [minute] [second];

  freezes the time to given time. This can be used to test the time based stuff.
*/
void Cmd_FreezeTimeTo(CommandParameter &Parameters) {
  int hour = Parameters.NextParameterAsInteger(0);
  int minute = Parameters.NextParameterAsInteger(0);
  int second = Parameters.NextParameterAsInteger(0);
  timestamp = second;
  timestamp += minute*60ul;
  timestamp += hour*60ul*60ul;
  freezeTime = true;
}


/*
  This function defines the MoveGateManual Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #MoveGateManual;

  resumes the time to the current RTC time.
*/
void Cmd_MoveGateManual(CommandParameter &Parameters) {
  int userinput = Parameters.NextParameterAsInteger(0);
  manualGateControl = true;
  if( userinput == 1) {
    digitalWrite(CHICKEN_GATE_DIGITAL_OUT, HIGH);
  }
  else if (userinput == 0) {
    digitalWrite(CHICKEN_GATE_DIGITAL_OUT, LOW);
  }
}


/*
  This function defines the MoveGateAutomatic Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #MoveGateAutomatic;

  resumes the time to the current RTC time.
*/
void Cmd_MoveGateAutomatic(CommandParameter &Parameters) {
  manualGateControl = false;
}


/*
  This function defines the UnfreezeTime Command

  Parameters: The cmdArguments
  

  Syntax off the serial command:
  #UnfreezeTime;

  resumes the time to the current RTC time.
*/
void Cmd_UnfreezeTime(CommandParameter &Parameters) {
  freezeTime = false;
}



void UpdateLightAO(uint32_t srDelay, uint32_t ssDelay, int maxBrightness, int arduinoPin, bool digital_out, int* currentBrightnessChX) {
  #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
    Serial.print(F("Channel 0 - "));
  #endif
  if (timestamp >= (sunriseTime+srDelay)%one_day
    && timestamp < (sunriseTime+srDelay+LightCfg.Data.SunriseDuration)%one_day) // if(sunrise)
  {
    int brightness = maxBrightness*((timestamp-sunriseTime-srDelay)%one_day)/LightCfg.Data.SunriseDuration;
    if (digital_out) {
      digitalWrite(arduinoPin, LOW);
      *currentBrightnessChX = LOW;
    } else {
      analogWrite(arduinoPin, brightness);
      *currentBrightnessChX = brightness;
    }
    #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Sunrise: "));
      Serial.println(brightness);
    #endif
  }
  else if ( timestamp >= (sunriseTime+srDelay+LightCfg.Data.SunriseDuration)%one_day // if(day)
    && timestamp < (LightCfg.Data.SunsetTime+ssDelay)%one_day )
  {
    if(digital_out) {
      digitalWrite(arduinoPin, HIGH);
      *currentBrightnessChX = HIGH;
    } else {
      analogWrite(arduinoPin, maxBrightness);
      *currentBrightnessChX = maxBrightness;
    }
    #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Day:     "));
      Serial.println(maxBrightness);
    #endif
  }
  else if(timestamp >= (LightCfg.Data.SunsetTime+ssDelay)%one_day
    && timestamp < (LightCfg.Data.SunsetTime+LightCfg.Data.SunsetDuration+ssDelay)%one_day ) // if(sunset)
  {
    int brightness = maxBrightness-(maxBrightness*((timestamp-LightCfg.Data.SunsetTime-ssDelay)%one_day)/LightCfg.Data.SunsetDuration);
    if(digital_out){
      digitalWrite(arduinoPin, LOW);
      *currentBrightnessChX = LOW;
    } else {
      analogWrite(arduinoPin, brightness);
      *currentBrightnessChX = brightness;
    }
    #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Sunset:  "));
      Serial.println(brightness);
    #endif
  }
  else
  {
    if(digital_out) {
      digitalWrite(arduinoPin, LOW);
      *currentBrightnessChX = LOW;
    } else {
      analogWrite(arduinoPin, 0);
      *currentBrightnessChX = 0;
    }
    #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Night:    "));
      Serial.println(0);
    #endif
  }
}


void UpdateLightOutputs(){
  sunriseTime = (LightCfg.Data.SunsetTime-LightCfg.Data.LightDuration)%one_day;
  UpdateLightAO(LightCfg.Data.SRDelayA0, LightCfg.Data.SSDelayA0, LightCfg.Data.MaxBrightness0, LIGHT_ANALOG_OUT_0, false, &currentBrightnessCh0);
  UpdateLightAO(LightCfg.Data.SRDelayA1, LightCfg.Data.SSDelayA1, LightCfg.Data.MaxBrightness1, LIGHT_ANALOG_OUT_1, false, &currentBrightnessCh1);
  UpdateLightAO(LightCfg.Data.SRDelayDO0, LightCfg.Data.SSDelayDO0, LightCfg.Data.MaxBrightness2, LIGHT_ANALOG_OUT_2, true, &currentBrightnessCh2);
}


/*
  This function updates the digital output off the plc to open and close the
  nest. This function is called in the main loop.
  The Nest is closed before Sunset and a few hours after sunset. This prevents
  that chicken sleep in the nest.

  Parameters: None
*/
void UpdateNestOutputs() { //HIGH == Nest UP; LOW == Nest Down;
  uint32_t nestCloseTime = (LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset)%one_day;
  uint32_t nestOpenTime = (LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset)%one_day;
  UpdateOutputTimeBased(nestOpenTime, nestCloseTime, NEST_DIGITAL_OUT);
}


/*
  This function updates the digital output off the plc to open and close the gate.
  this function is called in the main loop.
  The Gate is usually closed after Sunset and opened right after sunrise.

  Parameters: None
*/
void UpdateGateOutput() { //HIGH == Gate open; LOW == Gate closed
  uint32_t gateOpenTime = (LightCfg.Data.SunsetTime - LightCfg.Data.LightDuration + GateCfg.Data.GateOpenSunriseOffset)%one_day;
  uint32_t gateCloseTime = (LightCfg.Data.SunsetTime + GateCfg.Data.GateCloseSunsetOffset)%one_day;
  if(! manualGateControl) {
    UpdateOutputTimeBased(gateOpenTime, gateCloseTime, CHICKEN_GATE_DIGITAL_OUT);
  }
}


/*
  This function updates the a given digital output off the plc.

  Parameters: 
    onTimestamp - The Timestamp when the Digital Output switch to high
    offTimestamp - the Timestamp when the digital output switch to low
    pinNumbe - the Number off the pin that shoud be changed

  This function is used to open and close the nest and gate for the chickens.
*/
void UpdateOutputTimeBased(uint32_t onTimestamp, uint32_t offTimestamp, int pinNumber) {
  if(offTimestamp < onTimestamp ) {
    if(timestamp > offTimestamp && timestamp < onTimestamp) {
      if( digitalRead(pinNumber) == HIGH )
        digitalWrite(pinNumber, LOW);
    }
    else
    {
      if( digitalRead(pinNumber) == LOW )
        digitalWrite(pinNumber, HIGH);
    }
  }
  else // (onTimestamp < offTimestamp)
  {
    if(timestamp > onTimestamp && timestamp < offTimestamp ) {
      if( digitalRead(pinNumber) == LOW) 
        digitalWrite(pinNumber, HIGH);
    }
    else
    {
      if( digitalRead(pinNumber) == HIGH) 
        digitalWrite(pinNumber, LOW);
    }
  }
}


/*
  Controlls the water flush process. It reads a button to start the flush
  process off the water pipes. The flush process can be aborted witch
  another press off the button. This function also gets called in the main
  loop().
*/
void UpdateWaterOutputs() {
  int currentBtnInput = digitalRead(WATER_BTN);
  if(waterBtnRead != currentBtnInput) {
    if(currentBtnInput == HIGH) {
      #if DEBUG_OUTPUT == 3
        Serial.println("WaterFlush Btn pressed!");
      #endif
      waterBtnPressedTime = millis();
      if(digitalRead(WATER_VALVE_DO) == HIGH) {
        digitalWrite(WATER_VALVE_DO , LOW);
      } 
      else {
        digitalWrite(WATER_VALVE_DO , HIGH);
      }
    } 
    #if DEBUG_OUTPUT == 3
    else {
       Serial.println("WaterFlush Btn released!");
    }
    #endif
    waterBtnRead = currentBtnInput;
  }
  if( ( currentBtnInput == HIGH ) && ( waterBtnPressedTime+3000 < millis() ) ) {
    #if DEBUG_OUTPUT == 3
      Serial.println("long press of WaterBtn");
    #endif
  }
  if( millis() - waterBtnPressedTime >= WaterCfg.Data.WaterFlushDuration ) { 
    #if DEBUG_OUTPUT == 3
      Serial.println(millis() - waterBtnPressedTime);
      Serial.println(millis());
    #endif
    digitalWrite(WATER_VALVE_DO , LOW);
  }
}


/*
  This updates the feed sensor and limits the time the motor runs. If a limit is exceeded there is something wrong. Like feed is empty or stuck in thje silo.
  Then an Alarm contact is used to show something is wrong. The normal operation is when the sensor givees the signal to deliver feed the motor is switched on to deliver more feed.

  It is just a capacitive switch with an integrated timer.
*/
void UpdateFeedMotor() {
  if( (digitalRead(FEED_SENSOR_DI) == HIGH)){
    if ( (digitalRead(FEED_TRANSPORT_MOTOR_DO) == LOW) && (!feedAlarmState)) {
      digitalWrite(FEED_TRANSPORT_MOTOR_DO, HIGH);
      motorSwitchedOnMillis = millis();
      #if DEBUG_OUTPUT == 5
        Serial.println("feeding motor on");
      #endif
    }
    if ( ((millis() - motorSwitchedOnMillis) >= FeedCfg.Data.FeedMotorTimeoutMillis) && !feedAlarmState ) {
      #if DEBUG_OUTPUT == 5
        Serial.println("ALARM - FEEDING MOTOR TIMEOUT!");
      #endif
      digitalWrite(FEED_TRANSPORT_MOTOR_DO, LOW);
      feedAlarmState = true;
    }
  }
  else { 
    /* If the Sensor Recognizes feed it resets the alarm state. To restart the
    feeding process a push button (opener) das to be connected to the sensor 
    in series. This will start the Motors for another round. Repeat till the 
    feeding process works again. So this code is than run when the Button is 
    pushed ar the Sensor delivers a LOW signal.
    */
    digitalWrite(FEED_TRANSPORT_MOTOR_DO, LOW);
    feedAlarmState = false;
  }
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //AddCommands to theHandler 
  SerialCommandHandler.AddCommand(F("GetClock"), Cmd_GetConfig);
  SerialCommandHandler.AddCommand(F("GetConfig"), Cmd_GetConfig);
  SerialCommandHandler.AddCommand(F("SetSunset"), Cmd_SetSunset);
  SerialCommandHandler.AddCommand(F("SetClock"), Cmd_SetClock);
  SerialCommandHandler.AddCommand(F("SetLightDuration"), Cmd_SetLightDuration);
  SerialCommandHandler.AddCommand(F("SetSunriseDuration"), Cmd_SetSunriseDuration);
  SerialCommandHandler.AddCommand(F("SetSunsetDuration"), Cmd_SetSunsetDuration);
  SerialCommandHandler.AddCommand(F("SetMaxBrightness"), Cmd_SetMaxBrightness);
  SerialCommandHandler.AddCommand(F("SetMaxBrightnessForChannel"), Cmd_SetMaxBrightnessForChannel);
  SerialCommandHandler.AddCommand(F("SetDelays"), Cmd_SetDelays);
  SerialCommandHandler.AddCommand(F("SetSSDelay"), Cmd_SetSSDelay);
  SerialCommandHandler.AddCommand(F("SetSRDelay"), Cmd_SetSRDelay);
  SerialCommandHandler.AddCommand(F("SetNestOffset"), Cmd_SetNestSunsetOffset);
  SerialCommandHandler.AddCommand(F("SetWaterFlushDuration"), Cmd_SetWaterFlushDuration);
  SerialCommandHandler.AddCommand(F("GetWaterFlushDuration"), Cmd_GetWaterFlushDuration);
  SerialCommandHandler.AddCommand(F("SetGateOffsets"), Cmd_SetGateOffsets);
  SerialCommandHandler.AddCommand(F("SetFeedMotorTimeoutMillis"), Cmd_SetFeedMotorTimeoutMillis);
  SerialCommandHandler.AddCommand(F("GetCurrentLightBrightness"), Cmd_GetCurrentLightBrightness);
  SerialCommandHandler.AddCommand(F("FreezeTimeTo"), Cmd_FreezeTimeTo);
  SerialCommandHandler.AddCommand(F("UnfreezeTime"), Cmd_UnfreezeTime);
  SerialCommandHandler.AddCommand(F("MoveGateAutomatic"), Cmd_MoveGateAutomatic);
  SerialCommandHandler.AddCommand(F("MoveGateManual"), Cmd_MoveGateManual);


  Controllino_RTC_init(0);
  //Load the configs
  LightCfg.Load();
  NestCfg.Load();
  WaterCfg.Load();
  GateCfg.Load();
  FeedCfg.Load();

  pinMode(LIGHT_ANALOG_OUT_0, OUTPUT);
  pinMode(LIGHT_ANALOG_OUT_1, OUTPUT);
  pinMode(LIGHT_ANALOG_OUT_2, OUTPUT);
  pinMode(LIGHT_DIGITAL_OUT_0, OUTPUT);
  pinMode(NEST_DIGITAL_OUT, OUTPUT);
  pinMode(WATER_VALVE_DO, OUTPUT);
  pinMode(WATER_BTN, INPUT);
  pinMode(CHICKEN_GATE_DIGITAL_OUT, OUTPUT);
  pinMode(FEED_SENSOR_DI, INPUT);
  pinMode(FEED_TRANSPORT_MOTOR_DO, OUTPUT);
  pinMode(ALARM_OUT, OUTPUT);

  digitalWrite(NEST_DIGITAL_OUT, HIGH);
  digitalWrite(WATER_VALVE_DO, LOW);

  Serial.print("chicken-coop-light-controller-");
  Serial.println(VERSION);
}

void loop() {
  SerialCommandHandler.Process();
  #if MOCK_CLOCK == 1 // A Day runs 100 times as fast.
    if(UpdateAoTimer.TimePassed_Milliseconds(10))
  #elif DEBUG_OUTPUT == 2 || DEBUG_OUTPUT == 1
    if(UpdateAoTimer.TimePassed_Milliseconds(3000)) //slow down in debug mode 1 or 2
  #else
    if(UpdateAoTimer.TimePassed_Milliseconds(100)) //faster in production
  #endif
  {
      uint32_t controllino_sec = Controllino_GetSecond();
      uint32_t controllino_min = Controllino_GetMinute();
      uint32_t controllino_hour = Controllino_GetHour();
      #if MOCK_CLOCK == 0
        if( ! freezeTime) {
          timestamp = controllino_sec;
          timestamp += controllino_min*60ul;
          timestamp += controllino_hour*60ul*60ul;
        }
      #elif MOCK_CLOCK == 1
        timestamp = millis()/10ul%one_day;
        if((timestamp/60ul/60ul) != timestamp_hour){
            timestamp_hour = timestamp/60/60;
            Serial.print(timestamp_hour);
            Serial.println(":00");
        }
      #endif
      #if DEBUG_OUTPUT == 2 || DEBUG_OUTPUT == 4
        Serial.print("timestamp=");
        Serial.println(timestamp);
      #endif
      UpdateLightOutputs();
      UpdateNestOutputs();
      UpdateWaterOutputs();
      UpdateGateOutput();
    }
  //This runs as frequent as possible
  UpdateFeedMotor();
  if( alarmState || feedAlarmState ) { 
    /*Add aditional alarm States as needed. 
    (Right now only feedAlarmState is used)*/
    if( digitalRead(ALARM_OUT) == LOW )
      digitalWrite(ALARM_OUT, HIGH);
  }
  else {
    if( digitalRead(ALARM_OUT) == HIGH )
      digitalWrite(ALARM_OUT, LOW);
  }
}
