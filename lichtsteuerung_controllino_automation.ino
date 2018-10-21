#include "MegunoLink.h"
#include "CommandHandler.h"
#include "EEPROMStore.h"
#include "ArduinoTimer.h"
#include "Controllino.h"

#define DEBUG_OUPUT 0

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
    SunsetTime = 18*60*60;
    SunsetDuration = 60*60;
    SunriseDuration = 30*60;
    LightDuration = 10*60*60+0*60;
    MaxBrightness0 = 178;
    MaxBrightness1 = 178;
    MaxBrightness2 = 53;
    SSDelayA0 = 0;
    SSDelayA1 = 10*60;
    SSDelayDO0 = 20*60;
    SRDelayA0 = 0;
    SRDelayA1 = 10*60;
    SRDelayDO0 = 20*60;
  }
};

uint32_t timestamp = 0;
uint32_t sunriseTime;

EEPROMStore<LightControllerConfiguration> Cfg;
CommandHandler<12, 35, 7> SerialCommandHandler;
ArduinoTimer UpdateAoTimer;
ArduinoTimer UpdateParametersTimer;


void Cmd_SetSunset(CommandParameter &Parameters)
{
  uint32_t ss_hour = Parameters.NextParameterAsInteger(19);
  uint32_t ss_minute = Parameters.NextParameterAsInteger(15);
  Cfg.Data.SunsetTime = ss_hour*60*60+ss_minute*60;
  Cfg.Save();
}

void Cmd_GetSunset(CommandParameter &Parameters)
{
  Parameters.GetSource().print(F("Sunset Time: "));
  Parameters.GetSource().print(Cfg.Data.SunsetTime/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(Cfg.Data.SunsetTime/60%60);
}

void Cmd_SetLightDuration(CommandParameter &Parameters)
{
  uint32_t lightd_hours = Parameters.NextParameterAsInteger(9);
  uint32_t lightd_minutes = Parameters.NextParameterAsInteger(30);
  uint32_t sr_duration = Parameters.NextParameterAsInteger(30);
  uint32_t ss_duration = Parameters.NextParameterAsInteger(45);
  Cfg.Data.LightDuration = lightd_minutes*60+lightd_hours*60*60;
  Cfg.Data.SunriseDuration = sr_duration*60;
  Cfg.Data.SunsetDuration = ss_duration*60;
  Cfg.Save();
}

void Cmd_GetLightDuration(CommandParameter &Parameters)
{
  Parameters.GetSource().print(F("Light Duration: "));
  Parameters.GetSource().print(Cfg.Data.LightDuration/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(Cfg.Data.LightDuration/60%60);
  Parameters.GetSource().print(F("Sunrise duration is "));
  Parameters.GetSource().print(Cfg.Data.SunriseDuration/60);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Sunset duration is "));
  Parameters.GetSource().print(Cfg.Data.SunsetDuration/60);
  Parameters.GetSource().println(F(" minutes"));
}

void Cmd_SetMaxBrightness(CommandParameter &Parameters)
{
  int user_p0 = Parameters.NextParameterAsInteger(178);
  int user_p1 = Parameters.NextParameterAsInteger(178);
  int user_p2 = Parameters.NextParameterAsInteger(53);
  if(user_p0<0 || user_p0 > 100) {
    Parameters.GetSource().println(F("First parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
    user_p0 = 75;
  }
  if(user_p1<0 || user_p1 > 100) {
    Parameters.GetSource().println(F("Second parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
    user_p1 = 75;
  }
  if(user_p2<0 || user_p2 > 100) {
    Parameters.GetSource().println(F("Third parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
    user_p2 = 75;
  }
  Cfg.Data.MaxBrightness0 = user_p0*255/100;
  Cfg.Data.MaxBrightness1 = user_p1*255/100;
  Cfg.Data.MaxBrightness2 = user_p2*106/100;
  Cfg.Save();
}

void Cmd_GetMaxBrightness(CommandParameter &Parameters) {
  Parameters.GetSource().print(F("Light MaxBrightness: "));
  Parameters.GetSource().print(Cfg.Data.MaxBrightness0);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(Cfg.Data.MaxBrightness1);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(Cfg.Data.MaxBrightness2);
}


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

void Cmd_GetClock(CommandParameter &Parameters)
{
  Controllino_PrintTimeAndDate();
  Parameters.GetSource().print(F("timestamp = "));
  Parameters.GetSource().println(timestamp);
  Parameters.GetSource().print(F("\nSunset @ "));
  Parameters.GetSource().print(Cfg.Data.SunsetTime/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(Cfg.Data.SunsetTime/60%60);
  Parameters.GetSource().print(F("("));
  Parameters.GetSource().print(Cfg.Data.SunsetTime);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("Sunrise @ "));
  Parameters.GetSource().print(sunriseTime/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(sunriseTime/60%60);
  Parameters.GetSource().print(F("("));
  Parameters.GetSource().print(sunriseTime);
  Parameters.GetSource().println(F(")"));
}

void Cmd_SetDelays(CommandParameter &Parameters) {
  uint32_t delay0 = Parameters.NextParameterAsInteger(0);
  uint32_t delay1 = Parameters.NextParameterAsInteger(2);
  uint32_t delay2 = Parameters.NextParameterAsInteger(1);
  Cfg.Data.SSDelayA0 = delay0*60;
  Cfg.Data.SSDelayA1 = delay1*60;
  Cfg.Data.SSDelayDO0 = delay2*60;
  Cfg.Save();
}

void UpdateOutputAO0() {
  Serial.print(F("Channel 0 - "));
  if (timestamp >= sunriseTime+Cfg.Data.SRDelayA0
    && timestamp < sunriseTime+Cfg.Data.SRDelayA0+Cfg.Data.SunriseDuration) // if(sunrise)
  {
    int brightness = Cfg.Data.MaxBrightness0*(timestamp-sunriseTime-Cfg.Data.SRDelayA0)/Cfg.Data.SunriseDuration;
    analogWrite(CONTROLLINO_AO0, brightness);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Sunrise: "));
      Serial.println(brightness);
    #endif
  }
  else if (timestamp >= sunriseTime+Cfg.Data.SRDelayA0+Cfg.Data.SunriseDuration // if(day)
    && timestamp < Cfg.Data.SunsetTime+Cfg.Data.SSDelayA0)
  {
    analogWrite(CONTROLLINO_AO0, Cfg.Data.MaxBrightness0);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Day:     "));
      Serial.println(Cfg.Data.MaxBrightness0);
    #endif
  }
  else if(timestamp >= Cfg.Data.SunsetTime+Cfg.Data.SSDelayA0 &&
    timestamp < Cfg.Data.SunsetTime+Cfg.Data.SunsetDuration+Cfg.Data.SSDelayA0) // if(sunset)
  {
    int brightness = Cfg.Data.MaxBrightness0-(Cfg.Data.MaxBrightness0*(timestamp-Cfg.Data.SunsetTime-Cfg.Data.SSDelayA0)/Cfg.Data.SunsetDuration);
    analogWrite(CONTROLLINO_AO0, brightness);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Sunset:  "));
      Serial.println(brightness);
    #endif
  }
  else
  {
    analogWrite(CONTROLLINO_AO0, 0);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Night:    "));
      Serial.println(0);
    #endif
  }
}


void UpdateOutputAO1() {
  Serial.print(F("Channel 1 - "));
  if (timestamp >= sunriseTime+Cfg.Data.SRDelayA1
    && timestamp < sunriseTime+Cfg.Data.SRDelayA1+Cfg.Data.SunriseDuration) // if(sunrise)
  {
    int brightness = Cfg.Data.MaxBrightness1*(timestamp-sunriseTime-Cfg.Data.SRDelayA1)/Cfg.Data.SunriseDuration;
    analogWrite(CONTROLLINO_AO1, brightness);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Sunrise: "));
      Serial.println(brightness);
    #endif
  }
  else if (timestamp >= sunriseTime+Cfg.Data.SRDelayA1+Cfg.Data.SunriseDuration // if(day)
    && timestamp < Cfg.Data.SunsetTime+Cfg.Data.SSDelayA1)
  {
    analogWrite(CONTROLLINO_AO1, Cfg.Data.MaxBrightness1);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Day:     "));
      Serial.println(Cfg.Data.MaxBrightness1);
    #endif
  }
  else if(timestamp >= Cfg.Data.SunsetTime+Cfg.Data.SSDelayA1 &&
    timestamp < Cfg.Data.SunsetTime+Cfg.Data.SunsetDuration+Cfg.Data.SSDelayA1) // if(sunset)
  {
    int brightness = Cfg.Data.MaxBrightness1-(Cfg.Data.MaxBrightness1*(timestamp-Cfg.Data.SunsetTime-Cfg.Data.SSDelayA1)/Cfg.Data.SunsetDuration);
    analogWrite(CONTROLLINO_AO1, brightness);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Sunset:  "));
      Serial.println(brightness);
    #endif
  }
  else
  {
    analogWrite(CONTROLLINO_AO1, 0);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Night:    "));
      Serial.println(0);
    #endif
  }
}

void UpdateOutputDO0() {
  Serial.print(F("Channel 2 - "));
  if (timestamp >= sunriseTime+Cfg.Data.SRDelayDO0
    && timestamp < sunriseTime+Cfg.Data.SRDelayDO0+Cfg.Data.SunriseDuration) // if(sunrise)
  {
    int brightness = Cfg.Data.MaxBrightness2*(timestamp-sunriseTime-Cfg.Data.SRDelayDO0)/Cfg.Data.SunriseDuration;
    //analogWrite(CONTROLLINO_DO7, brightness);
    digitalWrite(CONTROLLINO_R9, LOW);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Sunrise: "));
      Serial.println(brightness);
    #endif
  }
  else if (timestamp >= sunriseTime+Cfg.Data.SRDelayDO0+Cfg.Data.SunriseDuration // if(day)
    && timestamp < Cfg.Data.SunsetTime+Cfg.Data.SSDelayDO0)
  {
    //analogWrite(CONTROLLINO_DO7, Cfg.Data.MaxBrightness2);
    digitalWrite(CONTROLLINO_R9, HIGH);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Day:     "));
      Serial.println(Cfg.Data.MaxBrightness2);
    #endif
  }
  else if(timestamp >= Cfg.Data.SunsetTime+Cfg.Data.SSDelayDO0 &&
    timestamp < Cfg.Data.SunsetTime+Cfg.Data.SunsetDuration+Cfg.Data.SSDelayDO0) // if(sunset)
  {
    int brightness = Cfg.Data.MaxBrightness2-(Cfg.Data.MaxBrightness2*(timestamp-Cfg.Data.SunsetTime-Cfg.Data.SSDelayDO0)/Cfg.Data.SunsetDuration);
    //analogWrite(CONTROLLINO_DO7, brightness);
    digitalWrite(CONTROLLINO_R9, LOW);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Sunset:  "));
      Serial.println(brightness);
    #endif
  }
  else // if(night)
  {
    //analogWrite(CONTROLLINO_DO7, 50);
    digitalWrite(CONTROLLINO_R9, LOW);
    #if DEBUG_OUPUT >= 1
      Serial.print(F("Night:    "));
      Serial.println(0);
    #endif
  }
}


void UpdateOutputs(){
  sunriseTime = Cfg.Data.SunsetTime-Cfg.Data.LightDuration;
  Serial.println(timestamp);
  UpdateOutputAO0();
  UpdateOutputAO1();
  UpdateOutputDO0();

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SerialCommandHandler.AddCommand(F("SetSunset"), Cmd_SetSunset);
  SerialCommandHandler.AddCommand(F("GetSunset"), Cmd_GetSunset);
  SerialCommandHandler.AddCommand(F("SetClock"), Cmd_SetClock);
  SerialCommandHandler.AddCommand(F("GetClock"), Cmd_GetClock);
  SerialCommandHandler.AddCommand(F("GetConfig"), Cmd_GetClock);
  SerialCommandHandler.AddCommand(F("SetLightDuration"), Cmd_SetLightDuration);
  SerialCommandHandler.AddCommand(F("GetLightDuration"), Cmd_GetLightDuration);
  SerialCommandHandler.AddCommand(F("SetMaxBrightness"), Cmd_SetMaxBrightness);
  SerialCommandHandler.AddCommand(F("GetMaxBrightness"), Cmd_GetMaxBrightness);
  SerialCommandHandler.AddCommand(F("SetDelays"), Cmd_SetDelays);

  Controllino_RTC_init();
  Cfg.Load();

  pinMode(CONTROLLINO_AO0, OUTPUT);
  pinMode(CONTROLLINO_AO1, OUTPUT);
  pinMode(CONTROLLINO_DO7, OUTPUT);
  pinMode(CONTROLLINO_R9, OUTPUT);
}

void loop() {
  SerialCommandHandler.Process();

  if( UpdateAoTimer.TimePassed_Milliseconds(1000)){
    uint32_t controllino_sec = Controllino_GetSecond();
    uint32_t controllino_min = Controllino_GetMinute();
    uint32_t controllino_hour = Controllino_GetHour();
    timestamp = controllino_sec;
    timestamp += controllino_min*60;
    timestamp += controllino_hour*60*60;

    #ifdef DEBUG_OUPUT >= 2
      Serial.print("timestamp=");
      Serial.print(timestamp);
    #endif

    UpdateOutputs();

  }
}
