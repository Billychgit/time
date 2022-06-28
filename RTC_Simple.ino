#include <Arduino.h>
#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "hmi.h"
#include "Timer.h"
#include <EEPROM.h>
#include "RTCDS1307.h"
#include "EEPROM_Function.h"
#include "UserCommand.h"
#include "Display.h"
#include <Wire.h>


extern MainDataStruct maindata;
extern RuntimeStatus runtimedata;
HardwareSerial *cmd_port;


RTCDS1307 rtc(0x68);

int StrToHex(char str[])
{
  return (int) strtol(str, 0, 16);
}
double map(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
long reflash_timer = 0;
long test_timer=0;
//========================================================
void setup() 
{
    cmd_port = &CMD_PORT;
    cmd_port->begin(CMD_PORT_BR);
    READ_EEPROM();
    MainProcess_Init();
    Display_Init();
//  Read DS1307 RTC =====================================
    rtc.begin();
    //rtc.setDate(22, 6, 14);   //設定 RTC　年月日
    //rtc.setTime(9,32, 00);   //設定 RTC　時分秒 
    rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
    rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);
     

    pinMode(BUZZ, OUTPUT);
    TimerInit(1, 10000);
    DEBUG_PRINT_DATA_LOG("FW_Version: ", VERSTR);
    //buzzerPlay(500);
}

void loop()
{   
    if(reflash_timer > 1000){
        reflash_timer = 0;
        Display(0,8,1,"  ");
    }
  /*if(test_timer> 10000){
        test_timer = 0;
        runtimedata.UpdateEEPROM = true;
        Serial.print("Savedhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh");
    }*/
    
  
    UserCommand_Task();
    MainProcess_Task();/*%02d:%02d:%02d*/
    
    //circal();
    DisplaySetHour();
    DisplaySetMinute();
    DisplaySetYear();
    DisplaySetMonth();
    
    coop();
    
    
    if(runtimedata.UpdateEEPROM)
    {
        runtimedata.UpdateEEPROM = false;
        WRITE_EEPROM();
    }
}
void coop(){
     sprintf(runtimedata.DS1307_DateTime, "%04d/%02d/%02d ", 
        runtimedata.ny+2000, runtimedata.nm, runtimedata.nd);
    cmd_port->println(runtimedata.DS1307_DateTime);
     Display(0,0,0,runtimedata.DS1307_DateTime);
     
     Display(0,0,1,String(runtimedata.nh));
     
     Display(0,2,1,":");
    
     Display(0,4,1,String(runtimedata.nin));
     Display(0,6,1,":");
     
     Display(0,8,1,String(runtimedata.second));
    }
void DisplaySetHour()
{
// 
   
  if(digitalRead(22)==HIGH){
    if(runtimedata.hour==23)
    { 
      Display(0,0,1,"  ");
      runtimedata.hour=0;
    }
    else
    { 
      Display(0,0,1,"  ");
      runtimedata.hour++;
    }
  }

  
   if(digitalRead(23)==HIGH)
  {
    if(runtimedata.hour==0)
    {Display(0,0,1,"  ");
     runtimedata.hour=23;
    }
    else
    {
      Display(0,0,1,"  ");
      runtimedata.hour--;
    }
  }
    runtimedata.nh=runtimedata.hour;
  delay(200);
}



void DisplaySetMinute()
{
// Setting the minutes
  
  if(digitalRead(24)==HIGH)
  {
    if (runtimedata.minute==59)
    {
      runtimedata.minute=0;
      runtimedata.nin=runtimedata.minute;
    }
    else
    {
      runtimedata.minute=runtimedata.minute++;
      runtimedata.nin=runtimedata.minute;
    }
  }
   if(digitalRead(25)==HIGH)
  {
    if (runtimedata.minute==0)
    {
      runtimedata.minute=59;
      runtimedata.nin=runtimedata.minute;
    }
    else
    {
      runtimedata.minute=runtimedata.minute--;
      runtimedata.nin=runtimedata.minute;
    }
  }
  
  
}

void DisplaySetYear()
{
// setting the year
  //lcd.clear();
  if(digitalRead(26)==HIGH)
  {    
    runtimedata.year++;
    runtimedata.ny=runtimedata.year;
    Serial.print("++");
  }
   if(digitalRead(27)==HIGH)
  {
    runtimedata.year--;
    Serial.print("--");
    runtimedata.ny=runtimedata.year;
  }
  
  delay(200);
}

void DisplaySetMonth()
{
// Setting the month
  
  if(digitalRead(28)==HIGH)
  {
    if (runtimedata.month==12)
    {
      runtimedata.month=1;
      runtimedata.nm=runtimedata.month;
    }
    else
    {
      runtimedata.month=runtimedata.month+1;
      runtimedata.nm=runtimedata.month;
    }
  }
   if(digitalRead(29)==HIGH)
  {
    if (runtimedata.month==1)
    {
      runtimedata.month=12;
      runtimedata.nm=runtimedata.month;
    }
    else
    {
      runtimedata.month=runtimedata.month-1;
      runtimedata.nm=runtimedata.month;
    }
  }
  
  delay(200);
}

void DisplaySetDay()
{
// Setting the day
  //lcd.clear();
  if(digitalRead(30)==HIGH)
  {
    if (runtimedata.day==31)
    { 
      runtimedata.day=1;
      runtimedata.nd=runtimedata.day;
    }
    else
    {
      runtimedata.day=runtimedata.day+1;
      runtimedata.nd=runtimedata.day;
    }
  }
   if(digitalRead(31)==HIGH)
  {
    if (runtimedata.day==1)
    {
      runtimedata.day=31;
      runtimedata.nd=runtimedata.day;
    }
    else
    {
      runtimedata.day=runtimedata.day-1;
      runtimedata.nd=runtimedata.day;
    }
  }
 
  delay(200);
}

ISR(TIMER1_COMPA_vect)
{
    if(reflash_timer < 0xFFFF)
        reflash_timer += TIMER_INTERVAL_MS;
        if(test_timer< 0xFFFF)
        test_timer+= TIMER_INTERVAL_MS;
    MainPorcess_Timer();
}
