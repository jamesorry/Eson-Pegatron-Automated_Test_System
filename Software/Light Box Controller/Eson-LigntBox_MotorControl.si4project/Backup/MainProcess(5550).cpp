#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "HMI.h"
#include "Timer.h"
#include "StepperMotor.h"


uint16_t ChechTimeCnt = 0;
int backforthCM = 0;
extern HardwareSerial *cmd_port;
extern HardwareSerial *CNC_cmd_port;

DigitalIO digitalio;
Adafruit_MCP23017 extio[EXTIO_NUM];

MainDataStruct maindata;
RuntimeStatus runtimedata;

StepperMotor *Motor[3];

void setOutput(uint8_t index, uint8_t hl)
{
	if(index < (OUTPUT_8_NUMBER*8))
	{
		digitalWrite(OutputPin[index], hl);
	}
	else
	{
		uint8_t extindex = index-(OUTPUT_8_NUMBER*8);
		uint8_t exi = extindex >> 3;
		uint8_t bi = extindex & 0x07;
		extio[exi].digitalWrite(bi, hl);
	}
	digitalio.Output[index] = hl;
}

uint8_t getInput(uint8_t index)
{
	uint8_t hl;
	if(index < (INPUT_8_NUMBER*8))
	{
		hl = digitalRead(InputPin[index]);
	}
	else
	{
		uint8_t extindex = index-(INPUT_8_NUMBER*8);
		uint8_t exi = extindex >> 3;
		uint8_t bi = extindex & 0x07;
		hl = extio[exi].digitalRead(bi+8);
	}

	digitalio.Input[index] = hl;
	return hl;
}


void MainProcess_Timer()
{
	if(ChechTimeCnt < 0xFF00)
		ChechTimeCnt += TIMER_INTERVAL_MS;
}

void MainProcess_ReCheckEEPROMValue()
{
	if((maindata.HMI_ID < 0) || (maindata.HMI_ID > 128))
	{
		maindata.HMI_ID = 0;
		runtimedata.UpdateEEPROM = true;
	}
}


void MainProcess_Init()
{
	int i,j;
	runtimedata.UpdateEEPROM = false;

	MainProcess_ReCheckEEPROMValue();
	runtimedata.RunMode = RUN_MODE_STOP;

	for(i=0; i<WORKINDEX_TOTAL; i++)
		runtimedata.Workindex[i]  = 0xF0;

	runtimedata.inPosition = 0xFF;
	
	runtimedata.ServoPosition = 0x00;
	runtimedata.ServoAcceleration = 100;

	for(i=0;i<2;i++)
		runtimedata.StepMotorSpeed[i] = 100;

	runtimedata.run = false;
	runtimedata.ErrorCode = 0x0000;

		
	for(i=0;i<INPUT_8_NUMBER+EXTIO_NUM;i++)
		digitalio.Input[i] = 0;
	
	for(i=0;i<OUTPUT_8_NUMBER+EXTIO_NUM;i++)
	{
		if(OUTPUT_NONE_ACTIVE == 0)
			digitalio.Output[i]	= 0;
		else
			digitalio.Output[i]	= 0xFF;
	}
		
	for(i=0; i<INPUT_8_NUMBER*8; i++)
	{
		pinMode(InputPin[i], INPUT);
	}
	for(i=0; i<OUTPUT_8_NUMBER*8; i++)
	{
		pinMode(OutputPin[i], OUTPUT);	
	}
	
	for(j=0; j<EXTIO_NUM; j++)
	{
		extio[j].begin(j);	  	// Default device address 0x20+j

		for(i=0; i<8; i++)
		{
			extio[j].pinMode(i, OUTPUT);  // Toggle LED 1
			extio[j].digitalWrite(i, OUTPUT_NONE_ACTIVE);
		}
	}
	for(i=0; i<OUTPUT_8_NUMBER*8; i++)
		digitalWrite(OutputPin[i], OUTPUT_NONE_ACTIVE);

	for(j=0; j<EXTIO_NUM; j++)
		for(i=0; i<8; i++)
		{
			extio[j].pinMode(i+8,INPUT);	 // Button i/p to GND
			extio[j].pullUp(i+8,HIGH);	 // Puled high to ~100k
		}

	Motor[0] = new StepperMotor(2, A8, 10000, 5000);
	Motor[1] = new StepperMotor(7, A11, 10000, 5000);
	Motor[2] = new StepperMotor(12, A13, 10000, 5000);

//	Motor[0]->setLimitPin(InputPin[1], LOW, InputPin[0], LOW);
}

void MainProcess_Task()
{
}
