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

#include "MegunoLink.h"
#include "CommandHandler.h"
#include "EEPROMStore.h"
#include "ArduinoTimer.h"
#include "Configuration_Controllino_Maxi.h"

struct NestControllerConfiguration
{
    int32_t NestOpenSunsetOffset;
    int32_t NestCloseSunsetOffset;
    void Reset() {
        NestOpenSunsetOffset = 5l*60l*60l; // 5 hours after sunset
        NestCloseSunsetOffset = 0l*60l*60l; // 0 hour before sunset
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

EEPROMStore<LightControllerConfiguration> LightCfg;
EEPROMStore<NestControllerConfiguration> NestCfg;
EEPROMStore<WaterControllerConfiguration> WaterCfg;
//CommandHandler<12, 35, 7> SerialCommandHandler(Serial,'#',';');
CommandHandler<13, 35, 7> SerialCommandHandler(Serial,'#',';');
ArduinoTimer UpdateAoTimer;

#if MOCK_CLOCK
  unsigned int timestamp_hour = 0;
#endif


void Cmd_SetWaterFlushDuration(CommandParameter &Parameters) {
  uint32_t water_fd = Parameters.NextParameterAsInteger(10); //duration in minutes
  WaterCfg.Data.WaterFlushDuration = water_fd*60ul*1000ul;
  WaterCfg.Save();
}

void Cmd_GetWaterFlushDuration(CommandParameter &Parameters) {
  Parameters.GetSource().print(WaterCfg.Data.WaterFlushDuration/(60ul*1000ul));
  Parameters.GetSource().println(F(" minutes"));
}

void Cmd_SetSunset(CommandParameter &Parameters)
{
  uint32_t ss_hour = Parameters.NextParameterAsInteger(19);
  uint32_t ss_minute = Parameters.NextParameterAsInteger(15);
  LightCfg.Data.SunsetTime = (ss_hour*60ul*60ul+ss_minute*60ul)%one_day;
  LightCfg.Save();
}

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

}

void Cmd_SetDelays(CommandParameter &Parameters) {
  uint32_t delay0 = Parameters.NextParameterAsInteger(0);
  uint32_t delay1 = Parameters.NextParameterAsInteger(2);
  uint32_t delay2 = Parameters.NextParameterAsInteger(1);
  LightCfg.Data.SSDelayA0 = delay0*60ul;
  LightCfg.Data.SSDelayA1 = delay1*60ul;
  LightCfg.Data.SSDelayDO0 = delay2*60ul;
  LightCfg.Save();
}

void Cmd_SetSSDelays(CommandParameter &Parameters) {
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

void Cmd_SetSRDelays(CommandParameter &Parameters) {
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

void Cmd_SetNestSunsetOffset(CommandParameter &Parameters) {
    int32_t offset0 = Parameters.NextParameterAsInteger(5*60);
    int32_t offset1 = Parameters.NextParameterAsInteger(0);
    NestCfg.Data.NestOpenSunsetOffset = offset0*60l;
    NestCfg.Data.NestCloseSunsetOffset = offset1*60l;
    NestCfg.Save();
}

void UpdateLightAO(uint32_t srDelay, uint32_t ssDelay, int maxBrightness, int arduinoPin, bool digital_out) {
  #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
    Serial.print(F("Channel 0 - "));
  #endif
  if (timestamp >= (sunriseTime+srDelay)%one_day
    && timestamp < (sunriseTime+srDelay+LightCfg.Data.SunriseDuration)%one_day) // if(sunrise)
  {
    int brightness = maxBrightness*((timestamp-sunriseTime-srDelay)%one_day)/LightCfg.Data.SunriseDuration;
    if (digital_out) {
      digitalWrite(arduinoPin, LOW);
    } else {
      analogWrite(arduinoPin, brightness);
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
    } else {
      analogWrite(arduinoPin, maxBrightness);
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
    } else {
      analogWrite(arduinoPin, brightness);
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
    } else {
      analogWrite(arduinoPin, 0);
    }
    #if DEBUG_OUTPUT == 1 || DEBUG_OUTPUT == 2
      Serial.print(F("Night:    "));
      Serial.println(0);
    #endif
  }
}



void UpdateLightOutputs(){
  sunriseTime = (LightCfg.Data.SunsetTime-LightCfg.Data.LightDuration)%one_day;
  UpdateLightAO(LightCfg.Data.SRDelayA0, LightCfg.Data.SSDelayA0, LightCfg.Data.MaxBrightness0, LIGHT_ANALOG_OUT_0, false);
  UpdateLightAO(LightCfg.Data.SRDelayA1, LightCfg.Data.SSDelayA1, LightCfg.Data.MaxBrightness1, LIGHT_ANALOG_OUT_1, false);
  UpdateLightAO(LightCfg.Data.SRDelayDO0, LightCfg.Data.SSDelayDO0, LightCfg.Data.MaxBrightness2, LIGHT_ANALOG_OUT_2, true);
}

void moveNest(uint8_t state){
    digitalWrite(NEST_DIGITAL_OUT, state);
    #if DEBUG_OUTPUT == 3 || DEBUG_OUTPUT == 4
      if(state == HIGH)
        Serial.println(F("Nest is opening"));
      if(state == LOW)
        Serial.println(F("Nest is closing"));
    #endif
}

void UpdateNestOutputs() { //HIGH == Nest UP; LOW == Nest Down;
    uint32_t nestCloseTime = (LightCfg.Data.SunsetTime + NestCfg.Data.NestCloseSunsetOffset)%one_day;
    uint32_t nestOpenTime = (LightCfg.Data.SunsetTime + NestCfg.Data.NestOpenSunsetOffset)%one_day;
    if(nestCloseTime < nestOpenTime) {
      if(timestamp > nestCloseTime && timestamp < nestOpenTime){
        if(digitalRead(NEST_DIGITAL_OUT) == HIGH){
          moveNest(LOW);
        }
      }
      else {
        if (digitalRead(NEST_DIGITAL_OUT) == LOW) {
          moveNest(HIGH);
        }
      }
    } else {
      if(timestamp > nestOpenTime && timestamp < nestCloseTime) {
        if(digitalRead(NEST_DIGITAL_OUT) == LOW){
          moveNest(HIGH);
        }
      }
      else {
        if (digitalRead(NEST_DIGITAL_OUT) == HIGH){
        moveNest(LOW);
        }
      }
    }
}

void UpdateWaterOutputs() {
  int currentBtnInput = digitalRead(WATER_BTN);
  if(waterBtnRead != currentBtnInput) {
    if(currentBtnInput == HIGH) {
      #if DEBUG_OUTPUT == 3
        Serial.println("WaterFlush Btn pressed!");
      #endif
      waterBtnPressedTime = millis();
      if(digitalRead(WATER_VALVE) == HIGH) {
        digitalWrite(WATER_VALVE, LOW);
      } 
      else {
        digitalWrite(WATER_VALVE, HIGH);
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
  if( waterBtnPressedTime + WaterCfg.Data.WaterFlushDuration < millis() ) {
    #if DEBUG_OUTPUT == 3
      Serial.println(waterBtnPressedTime + WaterCfg.Data.WaterFlushDuration);
      Serial.println(millis());
    #endif
    digitalWrite(WATER_VALVE, LOW);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
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
  SerialCommandHandler.AddCommand(F("SetNestOffset"), Cmd_SetNestSunsetOffset);
  SerialCommandHandler.AddCommand(F("SetWaterFlushDuration"), Cmd_SetWaterFlushDuration);
  SerialCommandHandler.AddCommand(F("GetWaterFlushDuration"), Cmd_GetWaterFlushDuration);

  Controllino_RTC_init(0);
  LightCfg.Load();
  NestCfg.Load();
  WaterCfg.Load();

  pinMode(LIGHT_ANALOG_OUT_0, OUTPUT);
  pinMode(LIGHT_ANALOG_OUT_1, OUTPUT);
  pinMode(LIGHT_ANALOG_OUT_2, OUTPUT);
  pinMode(LIGHT_DIGITAL_OUT_0, OUTPUT);
  pinMode(NEST_DIGITAL_OUT, OUTPUT);
  pinMode(WATER_VALVE, OUTPUT);
  pinMode(WATER_BTN, INPUT);

  digitalWrite(NEST_DIGITAL_OUT, HIGH);
  digitalWrite(WATER_VALVE, LOW);

  Serial.println("chicken-coop-light-controller-v0.01");
}

void loop() {
  SerialCommandHandler.Process();
  #if MOCK_CLOCK == 1 // A Day runs 100 times as fast.
    if(UpdateAoTimer.TimePassed_Milliseconds(10)){
  #elif DEBUG_OUTPUT == 2 || DEBUG_OUTPUT == 1
    if(UpdateAoTimer.TimePassed_Milliseconds(3000)){ //slow down in debug mode 1 or 2
  #else
    if(UpdateAoTimer.TimePassed_Milliseconds(100)){ //faster in production
  #endif
      uint32_t controllino_sec = Controllino_GetSecond();
      uint32_t controllino_min = Controllino_GetMinute();
      uint32_t controllino_hour = Controllino_GetHour();
      #if MOCK_CLOCK == 0
        timestamp = controllino_sec;
        timestamp += controllino_min*60ul;
        timestamp += controllino_hour*60ul*60ul;
      #elif MOCK_CLOCK == 1
        timestamp = millis()/1ul%one_day;
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
    }
}
