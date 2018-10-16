#include "MegunoLink.h"
#include "CommandHandler.h"
#include "EEPROMStore.h"
#include "ArduinoTimer.h"
#include "Controllino.h"

struct LightControllerConfiguration
{
  uint32_t SunsetTimeHour;
  uint32_t SunsetTimeMinute;
  uint32_t SunsetDurationMinutes;
  uint32_t SunriseDurationMinutes;
  uint32_t LightDurationHours;
  uint32_t LightDurationMinutes;
  int MaxBrightness0;
  int MaxBrightness1;
  int MaxBrightness2;

  void Reset() {
    SunsetTimeHour = 21;
    SunsetTimeMinute = 15;
    SunsetDurationMinutes = 45;
    SunriseDurationMinutes = 30;
    LightDurationHours = 14;
    LightDurationMinutes = 32;
    MaxBrightness0 = 178;
    MaxBrightness1 = 178;
    MaxBrightness2 = 53;
  }
};

EEPROMStore<LightControllerConfiguration> Configuration;
CommandHandler<> SerialCommandHandler;
ArduinoTimer UpdateAoTimer;
ArduinoTimer UpdateParametersTimer;


void Cmd_SetSunset(CommandParameter &Parameters)
{
  Configuration.Data.SunsetTimeHour = Parameters.NextParameterAsInteger(19);
  Configuration.Data.SunsetTimeMinute = Parameters.NextParameterAsInteger(0);
  Configuration.Data.SunsetDurationMinutes = Parameters.NextParameterAsInteger(45);
  Configuration.Save();
}

void Cmd_GetSunset(CommandParameter &Parameters)
{
  Parameters.GetSource().print(F("Sunset Time: "));
  Parameters.GetSource().print(Configuration.Data.SunsetTimeHour);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(Configuration.Data.SunsetTimeMinute);
  Parameters.GetSource().print(F("Sunset duration is "));
  Parameters.GetSource().print(Configuration.Data.SunsetDurationMinutes);
  Parameters.GetSource().println(F(" minutes"));
}

void Cmd_SetLightDuration(CommandParameter &Parameters)
{
  Configuration.Data.LightDurationHours = Parameters.NextParameterAsInteger(9);
  Configuration.Data.LightDurationMinutes = Parameters.NextParameterAsInteger(30);
  Configuration.Save();
}

void Cmd_GetLightDuration(CommandParameter &Parameters)
{
  Parameters.GetSource().print(F("Light Duration: "));
  Parameters.GetSource().print(Configuration.Data.LightDurationHours);
  Parameters.GetSource().print(F(":"));
  Parameters.GetSource().println(Configuration.Data.LightDurationMinutes);
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
    Parameters.GetSource().println(F("First parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
    user_p1 = 75;
  }
  if(user_p2<0 || user_p2 > 100) {
    Parameters.GetSource().println(F("First parameter out of range (values between 0 and 100 are allowed). Default Value of 75 is used"));
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
}

void UpdateOutputs(){
  return;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SerialCommandHandler.AddCommand(F("SetSunset"), Cmd_SetSunset);
  SerialCommandHandler.AddCommand(F("GetSunset"), Cmd_GetSunset);
  SerialCommandHandler.AddCommand(F("SetClock"), Cmd_SetClock);
  SerialCommandHandler.AddCommand(F("GetClock"), Cmd_GetClock);
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
  uint32_t timestamp;
  uint32_t sunset_timestamp;
  sunset_timestamp = Configuration.Data.SunsetTimeHour*60*60+
    Configuration.Data.SunsetTimeMinute*60;
  SerialCommandHandler.Process();

  if( UpdateAoTimer.TimePassed_Milliseconds(200)){
    timestamp = Controllino_GetSecond();
    timestamp += Controllino_GetMinute()*60;
    timestamp += Controllino_GetHour()*60*60;
    Serial.print("timestamp=");
    Serial.print(timestamp);
    Serial.print("; sunset_timestamp=");
    Serial.println(sunset_timestamp);
    UpdateOutputs();

  }

  if( UpdateParametersTimer.TimePassed_Seconds(10)){
    sunset_timestamp = Configuration.Data.SunsetTimeHour*60*60+
      Configuration.Data.SunsetTimeMinute*60;
  }
}
