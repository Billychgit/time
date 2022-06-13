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
    //rtc.setDate(22, 6, 13);   //設定 RTC　年月日
    //rtc.setTime(9,40, 00);   //設定 RTC　時分秒 
    rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
    rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);
      

    pinMode(BUZZ, OUTPUT);
    TimerInit(1, 10000);
    DEBUG_PRINT_DATA_LOG("FW_Version: ", VERSTR);
    //buzzerPlay(500);
}

void loop()
{   
    /*if(reflash_timer > 1000){
        reflash_timer = 0;
    }*/
    rtc.getDate(runtimedata.year, runtimedata.month, runtimedata.day, runtimedata.weekday);
    rtc.getTime(runtimedata.hour, runtimedata.minute, runtimedata.second, runtimedata.period);

    UserCommand_Task();
    MainProcess_Task();
     sprintf(runtimedata.DS1307_DateTime, "%04d/%02d/%02d %02d:%02d:%02d", 
        runtimedata.year+2000, runtimedata.month, runtimedata.day, 
        runtimedata.hour, runtimedata.minute, runtimedata.second);
    cmd_port->println(runtimedata.DS1307_DateTime);
     Display(0,0,0,runtimedata.DS1307_DateTime);
     
     /*Display(0,0,1,String(runtimedata.hour));
     
     Display(2,0,1,":");
    
     Display(3,0,1,String(runtimedata.minute));
     
     Display(5,0,1,String(runtimedata.second));
     */
    if(runtimedata.UpdateEEPROM)
    {
        runtimedata.UpdateEEPROM = false;
        WRITE_EEPROM();
    }
}

ISR(TIMER1_COMPA_vect)
{
    if(reflash_timer < 0xFFFF)
        reflash_timer += TIMER_INTERVAL_MS;
    MainPorcess_Timer();
}
