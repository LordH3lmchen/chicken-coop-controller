#include "MegunoLink.h"
#include "CommandHandler.h"
#include "EEPROMStore.h"
#include "ArduinoTimer.h"
#include "Controllino.h"

struct LightControllerConfiguration
{
  uint32_t SunsetTime;
  uint32_t SunsetDuration;
  uint32_t SunriseDuration;
  uint32_t LightDuration;
  int MaxBrightness0;
  int MaxBrightness1;
  int MaxBrightness2;

  void Reset() {
    SunsetTime = 21*60*60+15*60;
    SunsetDuration = 45*60;
    SunriseDuration = 30*60;
    LightDuration = 14*60*60+32*60;
    MaxBrightness0 = 178;
    MaxBrightness1 = 178;
    MaxBrightness2 = 53;
  }
};

uint32_t timestamp = 0;

EEPROMStore<LightControllerConfiguration> Configuration;
CommandHandler<> SerialCommandHandler;
ArduinoTimer UpdateAoTimer;
ArduinoTimer UpdateParametersTimer;


void Cmd_SetSunset(CommandParameter &Parameters)
{
  uint32_t ss_hour = Parameters.NextParameterAsInteger(19);
  uint32_t ss_minute = Parameters.NextParameterAsInteger(15);
  Configuration.Data.SunsetTime = ss_hour*60*60+ss_minute*60;
  Configuration.Data.SunsetDuration = Parameters.NextParameterAsInteger(45)*60;
  Configuration.Save();
}

void Cmd_GetSunset(CommandParameter &Parameters)
{
  Parameters.GetSource().print(F("Sunset Time: "));
  Parameters.GetSource().print(Configuration.Data.SunsetTime/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(Configuration.Data.SunsetTime/60%60);
  Parameters.GetSource().print(F("Sunset duration is "));
  Parameters.GetSource().print(Configuration.Data.SunsetDuration/60);
  Parameters.GetSource().println(F(" minutes"));
}

void Cmd_SetLightDuration(CommandParameter &Parameters)
{
  uint32_t lightd_hours = Parameters.NextParameterAsInteger(9);
  uint32_t lightd_minutes = Parameters.NextParameterAsInteger(30);
  Configuration.Data.LightDuration = lightd_minutes*60+lightd_hours*60*60;
  Configuration.Save();
}

void Cmd_GetLightDuration(CommandParameter &Parameters)
{
  Parameters.GetSource().print(F("Light Duration: "));
  Parameters.GetSource().print(Configuration.Data.LightDuration);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(Configuration.Data.LightDuration);
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
  Configuration.Data.MaxBrightness0 = user_p0*255/100;
  Configuration.Data.MaxBrightness1 = user_p1*255/100;
  Configuration.Data.MaxBrightness2 = user_p2*106/100;
  Configuration.Save();
}

void Cmd_GetMaxBrightness(CommandParameter &Parameters) {
  Parameters.GetSource().print(F("Light MaxBrightness: "));
  Parameters.GetSource().print(Configuration.Data.MaxBrightness0);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(Configuration.Data.MaxBrightness1);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(Configuration.Data.MaxBrightness2);
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
  Parameters.GetSource().print(Configuration.Data.SunsetTime/60/60);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().print(Configuration.Data.SunsetTime/60%60);
  Parameters.GetSource().print(F("("));
  Parameters.GetSource().print(Configuration.Data.SunsetTime);
  Parameters.GetSource().println(F(")"));

}

void UpdateOutputs(){
  //if(timestamp>sunset_timestamp && timestamp < sunset_timestamp + sunset_duration_s )
  if(timestamp>Configuration.Data.SunsetTime &&
    timestamp < Configuration.Data.SunsetTime+Configuration.Data.SunsetDuration)
    {
      int brightness0 = Configuration.Data.MaxBrightness0-(Configuration.Data.MaxBrightness0*(timestamp-Configuration.Data.SunsetTime)/Configuration.Data.SunsetDuration);
      Serial.print("Current Brightness =");
      Serial.println(brightness0);
      analogWrite(CONTROLLINO_AO0, brightness0);
    }
  return;
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

  Controllino_RTC_init();
  Configuration.Load();

  pinMode(CONTROLLINO_AO0, OUTPUT);
  pinMode(CONTROLLINO_AO1, OUTPUT);
  pinMode(CONTROLLINO_DO0, OUTPUT);

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
    //Serial.print("timestamp=");
    //Serial.print(timestamp);
    //Serial.print("; sunset_timestamp=");
    //Serial.println(Configuration.Data.SunsetTime);
    UpdateOutputs();

  }

  if( UpdateParametersTimer.TimePassed_Seconds(10)){
    //sunset_timestamp = Configuration.Data.SunsetTimeHour*60*60+
    //  Configuration.Data.SunsetTimeMinute*60;
  }
}
