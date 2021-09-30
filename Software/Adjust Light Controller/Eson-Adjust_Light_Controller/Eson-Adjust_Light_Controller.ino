#include "Arduino.h"
#include "UserCommand.h"
#include "EEPROM_Function.h"
#include "MainProcess.h"
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include "Timer.h"
#include "HMI_Command.h"
#include "SoftwareSerial.h"

Adafruit_MCP4725 MCP4725;

SoftwareSerial mySerial(10, 9);	//38400

HMI_Command *hmi_cmd;

#define CMD_PORT 		Serial
#define CMD_PORT_BR 	115200

#define HMI_CMD_PORT    Serial1
#define HMI_CMD_PORT_BR 38400

#define	WIFI_PORT		mySerial
#define	WIFI_PORT_BR	38400

HardwareSerial *cmd_port;

extern MainDataStruct maindata;
extern RuntimeStatus runtimedata;

void setup() {
	cmd_port = &CMD_PORT;
	cmd_port->begin(CMD_PORT_BR);
    pinMode(BUZZ, OUTPUT);
    pinMode(LAN_CFG, OUTPUT);
    pinMode(out0, OUTPUT);
    pinMode(out1, OUTPUT);
    pinMode(out2, OUTPUT);
    pinMode(out3, OUTPUT);
    pinMode(in0, INPUT);
    pinMode(in1, INPUT);  
    pinMode(in2, INPUT);
    pinMode(in3, INPUT);   
    pinMode(BUZZ, OUTPUT);

    digitalWrite(BUZZ, LOW);
    digitalWrite(out0, LOW);  
    digitalWrite(out1, LOW);
    digitalWrite(out2, LOW);  
    digitalWrite(out3, LOW);
    
    MCP4725.begin(0x60);
    MCP4725.setVoltage(0, false);
    hmi_cmd = new HMI_Command(&HMI_CMD_PORT, HMI_CMD_PORT_BR);
//    hmi_cmd = new HMI_Command(&WIFI_PORT, WIFI_PORT_BR);

    TimerInit(1, 10000);
    
	READ_EEPROM();
	MainProcess_Init();
	buzzerPlay(500);
}

void loop() {
	UserCommand_Task();
    MainProcess_Task();
    hmi_cmd->Process();
    
	if(runtimedata.UpdateEEPROM)
	{
		runtimedata.UpdateEEPROM = false;
		WRITE_EEPROM(); //maindata內的值都會寫到EEPROM
	}
}


ISR(TIMER1_COMPA_vect)
{
	
}

