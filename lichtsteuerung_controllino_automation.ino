#include "MegunoLink.h"
#include "CommandHandler.h"
#include "EEPROMStore.h"
#include "ArduinoTimer.h"
#include "Controllino.h"

#define DEBUG_OUPUT 0
/*
* This disables the clock and mocks it with millis(), for testing purposes
* it is usefull. 1 day runs in about 864s (14min and 24s)
*/
#define MOCK_CLOCK 1

struct NestControllerConfiguration
{
    int32_t NestOpenSunsetOffset;
    int32_t NestCloseSunsetOffset;
    void Reset() {
        NestOpenSunsetOffset = 3*60*60; // 3 hours after sunset
        NestCloseSunsetOffset = -1*60*60; // 1 hour before sunset
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

EEPROMStore<LightControllerConfiguration> LightCfg;
EEPROMStore<NestControllerConfiguration> NestCfg;
CommandHandler<12, 35, 7> SerialCommandHandler;
ArduinoTimer UpdateAoTimer;
ArduinoTimer UpdateParametersTimer;

void Cmd_SetSunset(CommandParameter &Parameters)
{
  uint32_t ss_hour = Parameters.NextParameterAsInteger(19);
  uint32_t ss_minute = Parameters.NextParameterAsInteger(15);
  LightCfg.Data.SunsetTime = ss_hour*60*60+ss_minute*60;
  LightCfg.Save();
}

void Cmd_SetLightDuration(CommandParameter &Parameters)
{
  uint32_t lightd_hours = Parameters.NextParameterAsInteger(9);
  uint32_t lightd_minutes = Parameters.NextParameterAsInteger(30);
  uint32_t sr_duration = Parameters.NextParameterAsInteger(30);
  uint32_t ss_duration = Parameters.NextParameterAsInteger(45);
  LightCfg.Data.LightDuration = lightd_minutes*60+lightd_hours*60*60;
  LightCfg.Data.SunriseDuration = sr_duration*60;
  LightCfg.Data.SunsetDuration = ss_duration*60;
  LightCfg.Save();
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
  LightCfg.Data.MaxBrightness0 = user_p0*255/100;
  LightCfg.Data.MaxBrightness1 = user_p1*255/100;
  LightCfg.Data.MaxBrightness2 = user_p2*106/100;
  LightCfg.Save();
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

void Cmd_GetConfig(CommandParameter &Parameters)
{
  Controllino_PrintTimeAndDate();
  Parameters.GetSource().print(F("timestamp = "));
  Parameters.GetSource().println(timestamp);
  Parameters.GetSource().print(F("\nSunset @ "));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime/60%60);
  Parameters.GetSource().print(F("("));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("Sunrise @ "));
  Parameters.GetSource().print(sunriseTime/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(sunriseTime/60%60);
  Parameters.GetSource().print(F("("));
  Parameters.GetSource().print(sunriseTime);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("\nNestOpenTime @ "));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset)/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset)/60%60);
  Parameters.GetSource().print(F(" ("));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("NestCloseTime @ "));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset)/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print((LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset)/60%60);
  Parameters.GetSource().print(F(" ("));
  Parameters.GetSource().print(LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset);
  Parameters.GetSource().println(F(")"));
  Parameters.GetSource().print(F("\nLight Duration: "));
  Parameters.GetSource().print(LightCfg.Data.LightDuration/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(LightCfg.Data.LightDuration/60%60);
  Parameters.GetSource().print(F("Sunrise duration is "));
  Parameters.GetSource().print(LightCfg.Data.SunriseDuration/60);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Sunset duration is "));
  Parameters.GetSource().print(LightCfg.Data.SunsetDuration/60);
  Parameters.GetSource().println(F(" minutes"));
  Parameters.GetSource().print(F("Brightness Settings\nLight MaxBrightness0="));
  Parameters.GetSource().println(LightCfg.Data.MaxBrightness0);
  Parameters.GetSource().print(F("Light MaxBrightness1="));
  Parameters.GetSource().println(LightCfg.Data.MaxBrightness1);
  Parameters.GetSource().print(F("Light MaxBrightness2="));
  Parameters.GetSource().println(LightCfg.Data.MaxBrightness2);
}

void Cmd_SetDelays(CommandParameter &Parameters) {
  uint32_t delay0 = Parameters.NextParameterAsInteger(0);
  uint32_t delay1 = Parameters.NextParameterAsInteger(2);
  uint32_t delay2 = Parameters.NextParameterAsInteger(1);
  LightCfg.Data.SSDelayA0 = delay0*60;
  LightCfg.Data.SSDelayA1 = delay1*60;
  LightCfg.Data.SSDelayDO0 = delay2*60;
  LightCfg.Save();
}

void Cmd_SetNestSunsetOffset(CommandParameter &Parameters) {
    int32_t offset0 = Parameters.NextParameterAsInteger(60);
    int32_t offset1 = Parameters.NextParameterAsInteger(60);
    NestCfg.Data.NestOpenSunsetOffset = offset0 * 60;
    NestCfg.Data.NestCloseSunsetOffset = offset1 * 60;
}

void UpdateOutputAO0() {
  #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
    Serial.print(F("Channel 0 - "));
  #endif
  if (timestamp >= sunriseTime+LightCfg.Data.SRDelayA0
    && timestamp < sunriseTime+LightCfg.Data.SRDelayA0+LightCfg.Data.SunriseDuration) // if(sunrise)
  {
    int brightness = LightCfg.Data.MaxBrightness0*(timestamp-sunriseTime-LightCfg.Data.SRDelayA0)/LightCfg.Data.SunriseDuration;
    analogWrite(CONTROLLINO_AO0, brightness);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Sunrise: "));
      Serial.println(brightness);
    #endif
  }
  else if (timestamp >= sunriseTime+LightCfg.Data.SRDelayA0+LightCfg.Data.SunriseDuration // if(day)
    && timestamp < LightCfg.Data.SunsetTime+LightCfg.Data.SSDelayA0)
  {
    analogWrite(CONTROLLINO_AO0, LightCfg.Data.MaxBrightness0);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Day:     "));
      Serial.println(LightCfg.Data.MaxBrightness0);
    #endif
  }
  else if(timestamp >= LightCfg.Data.SunsetTime+LightCfg.Data.SSDelayA0 &&
    timestamp < LightCfg.Data.SunsetTime+LightCfg.Data.SunsetDuration+LightCfg.Data.SSDelayA0) // if(sunset)
  {
    int brightness = LightCfg.Data.MaxBrightness0-(LightCfg.Data.MaxBrightness0*(timestamp-LightCfg.Data.SunsetTime-LightCfg.Data.SSDelayA0)/LightCfg.Data.SunsetDuration);
    analogWrite(CONTROLLINO_AO0, brightness);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Sunset:  "));
      Serial.println(brightness);
    #endif
  }
  else
  {
    analogWrite(CONTROLLINO_AO0, 0);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Night:    "));
      Serial.println(0);
    #endif
  }
}


void UpdateOutputAO1() {
  #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
    Serial.print(F("Channel 1 - "));
  #endif
  if (timestamp >= sunriseTime+LightCfg.Data.SRDelayA1
    && timestamp < sunriseTime+LightCfg.Data.SRDelayA1+LightCfg.Data.SunriseDuration) // if(sunrise)
  {
    int brightness = LightCfg.Data.MaxBrightness1*(timestamp-sunriseTime-LightCfg.Data.SRDelayA1)/LightCfg.Data.SunriseDuration;
    analogWrite(CONTROLLINO_AO1, brightness);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Sunrise: "));
      Serial.println(brightness);
    #endif
  }
  else if (timestamp >= sunriseTime+LightCfg.Data.SRDelayA1+LightCfg.Data.SunriseDuration // if(day)
    && timestamp < LightCfg.Data.SunsetTime+LightCfg.Data.SSDelayA1)
  {
    analogWrite(CONTROLLINO_AO1, LightCfg.Data.MaxBrightness1);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Day:     "));
      Serial.println(LightCfg.Data.MaxBrightness1);
    #endif
  }
  else if(timestamp >= LightCfg.Data.SunsetTime+LightCfg.Data.SSDelayA1 &&
    timestamp < LightCfg.Data.SunsetTime+LightCfg.Data.SunsetDuration+LightCfg.Data.SSDelayA1) // if(sunset)
  {
    int brightness = LightCfg.Data.MaxBrightness1-(LightCfg.Data.MaxBrightness1*(timestamp-LightCfg.Data.SunsetTime-LightCfg.Data.SSDelayA1)/LightCfg.Data.SunsetDuration);
    analogWrite(CONTROLLINO_AO1, brightness);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Sunset:  "));
      Serial.println(brightness);
    #endif
  }
  else
  {
    analogWrite(CONTROLLINO_AO1, 0);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Night:    "));
      Serial.println(0);
    #endif
  }
}

void UpdateOutputDO0() {
    #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Channel 2 - "));
    #endif
  if (timestamp >= sunriseTime+LightCfg.Data.SRDelayDO0
    && timestamp < sunriseTime+LightCfg.Data.SRDelayDO0+LightCfg.Data.SunriseDuration) // if(sunrise)
  {
    int brightness = LightCfg.Data.MaxBrightness2*(timestamp-sunriseTime-LightCfg.Data.SRDelayDO0)/LightCfg.Data.SunriseDuration;
    //analogWrite(CONTROLLINO_DO7, brightness);
    digitalWrite(CONTROLLINO_R9, LOW);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Sunrise: "));
      Serial.println(brightness);
    #endif
  }
  else if (timestamp >= sunriseTime+LightCfg.Data.SRDelayDO0+LightCfg.Data.SunriseDuration // if(day)
    && timestamp < LightCfg.Data.SunsetTime+LightCfg.Data.SSDelayDO0)
  {
    //analogWrite(CONTROLLINO_DO7, LightCfg.Data.MaxBrightness2);
    digitalWrite(CONTROLLINO_R9, HIGH);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Day:     "));
      Serial.println(LightCfg.Data.MaxBrightness2);
    #endif
  }
  else if(timestamp >= LightCfg.Data.SunsetTime+LightCfg.Data.SSDelayDO0 &&
    timestamp < LightCfg.Data.SunsetTime+LightCfg.Data.SunsetDuration+LightCfg.Data.SSDelayDO0) // if(sunset)
  {
    int brightness = LightCfg.Data.MaxBrightness2-(LightCfg.Data.MaxBrightness2*(timestamp-LightCfg.Data.SunsetTime-LightCfg.Data.SSDelayDO0)/LightCfg.Data.SunsetDuration);
    //analogWrite(CONTROLLINO_DO7, brightness);
    digitalWrite(CONTROLLINO_R9, LOW);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Sunset:  "));
      Serial.println(brightness);
    #endif
  }
  else // if(night)
  {
    //analogWrite(CONTROLLINO_DO7, 50);
    digitalWrite(CONTROLLINO_R9, LOW);
    #if DEBUG_OUPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Night:    "));
      Serial.println(0);
    #endif
  }
}


void UpdateLightOutputs(){
  sunriseTime = LightCfg.Data.SunsetTime-LightCfg.Data.LightDuration;
  UpdateOutputAO0();
  UpdateOutputAO1();
  UpdateOutputDO0();
}

void UpdateNestOutputs() { //HIGH == Nest UP; LOW == Nest Down;
    if(timestamp == LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset) {
        if(digitalRead(CONTROLLINO_R0) == LOW) {
            digitalWrite(CONTROLLINO_R0, HIGH);
        }
    }
    if(timestamp == LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset) {
        if(digitalRead(CONTROLLINO_R0) == HIGH) {
            digitalWrite(CONTROLLINO_R0, LOW);
        }
    }
    return;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SerialCommandHandler.AddCommand(F("SetSunset"), Cmd_SetSunset);
  SerialCommandHandler.AddCommand(F("SetClock"), Cmd_SetClock);
  SerialCommandHandler.AddCommand(F("GetClock"), Cmd_GetConfig);
  SerialCommandHandler.AddCommand(F("GetConfig"), Cmd_GetConfig);
  SerialCommandHandler.AddCommand(F("SetLightDuration"), Cmd_SetLightDuration);
  SerialCommandHandler.AddCommand(F("SetMaxBrightness"), Cmd_SetMaxBrightness);
  SerialCommandHandler.AddCommand(F("SetDelays"), Cmd_SetDelays);
  SerialCommandHandler.AddCommand(F("SetNestOffset"), Cmd_SetNestSunsetOffset);
  Controllino_RTC_init();
  LightCfg.Load();
  NestCfg.Load();

  pinMode(CONTROLLINO_AO0, OUTPUT);
  pinMode(CONTROLLINO_AO1, OUTPUT);
  pinMode(CONTROLLINO_DO7, OUTPUT);
  pinMode(CONTROLLINO_R9, OUTPUT);
  pinMode(CONTROLLINO_R0, OUTPUT);
}

void loop() {
  SerialCommandHandler.Process();
  #if MOCK_CLOCK == 1
    if(UpdateAoTimer.TimePassed_Milliseconds(200)){
  #elif DEBUG_OUPUT == 2 || DEBUG_OUPUT == 1
    if(UpdateAoTimer.TimePassed_Milliseconds(3000)){ //slow down in debug mode 1 or 2
  #else
    if(UpdateAoTimer.TimePassed_Milliseconds(200)){ //faster in production
  #endif
      uint32_t controllino_sec = Controllino_GetSecond();
      uint32_t controllino_min = Controllino_GetMinute();
      uint32_t controllino_hour = Controllino_GetHour();
      #if MOCK_CLOCK == 0
        timestamp = controllino_sec;
        timestamp += controllino_min*60;
        timestamp += controllino_hour*60*60;
      #elif MOCK_CLOCK == 1
        timestamp = millis()/100%(24*60*60);
      #endif
      #if DEBUG_OUPUT == 2 || DEBUG_OUPUT == 4
        Serial.print("timestamp=");
        Serial.print(timestamp);
      #endif
      UpdateLightOutputs();
      UpdateNestOutputs();
    }
}
