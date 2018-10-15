#include "MegunoLink.h"
#include "CommandHandler.h"
#include "EEPROMStore.h"
#include "Controllino.h"

struct LightControllerConfiguration
{
  int SunsetTimeHour;
  int SunsetTimeMinute;
  int SunsetDurationMinutes;
  int SunriseDurationMinutes;
  int LightDurationHours;
  int LightDurationMinutes;

  void Reset() {
    SunsetTimeHour = 21;
    SunsetTimeMinute = 15;
    SunsetDurationMinutes = 45;
    SunriseDurationMinutes = 30;
    LightDurationHours = 14;
    LightDurationMinutes = 32;
  }
};

EEPROMStore<LightControllerConfiguration> Configuration;
CommandHandler<> SerialCommandHandler;

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SerialCommandHandler.AddCommand(F("SetSunset"), Cmd_SetSunset);
  SerialCommandHandler.AddCommand(F("GetSunset"), Cmd_GetSunset);
  SerialCommandHandler.AddCommand(F("SetClock"), Cmd_SetClock);
  SerialCommandHandler.AddCommand(F("GetClock"), Cmd_GetClock);
  SerialCommandHandler.AddCommand(F("SetLightDuration"), Cmd_SetLightDuration);
  SerialCommandHandler.AddCommand(F("GetLightDuration"), Cmd_GetLightDuration);

  Controllino_RTC_init();
  Configuration.Load();
}

void loop() {
  SerialCommandHandler.Process();
}
