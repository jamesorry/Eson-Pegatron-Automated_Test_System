#include "MainProcess.h"
#include <Adafruit_MCP4725.h>
#include <Wire.h>
#include "HMI_Command.h"
#include "EEPROM_Function.h"

extern HMI_Command *hmi_cmd;
extern Adafruit_MCP4725 MCP4725;
extern HardwareSerial *cmd_port;
MainDataStruct maindata;
RuntimeStatus runtimedata;

void MainProcess_ReCheckEEPROMValue()
{
	if((maindata.HMI_ID < 0) || (maindata.HMI_ID > 128))
	{
		maindata.HMI_ID = 0;
		runtimedata.UpdateEEPROM = true;
	}
    if((maindata.Voltage_Last < 0) || (maindata.Voltage_Last > 100))
    {
        maindata.Voltage_Last = 0;
        runtimedata.UpdateEEPROM = true;
    }
}

void MainProcess_Init()
{
	runtimedata.UpdateEEPROM = false;
	MainProcess_ReCheckEEPROMValue();
}


void MainProcess_Task()  // This is a task.
{
    if(runtimedata.CheckReceive != -1)
    {
        switch (runtimedata.CheckReceive)
        {
            case 0x00:
                break;
            case 0x01:
                break;
            case 0x02:
                break;
            case 0x03:
                break;
            case 0x04:
                break;
            case 0x05: //Tare
                break;
            case 0x06: //Scale
                break;
        }
        runtimedata.CheckReceive = -1;
    }
}

void buzzerPlay(int playMS)
{
  digitalWrite(BUZZ, HIGH);
  delay(playMS);
  digitalWrite(BUZZ, LOW);
}

