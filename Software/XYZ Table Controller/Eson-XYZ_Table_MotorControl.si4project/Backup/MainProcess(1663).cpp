#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "HMI.h"
#include "Timer.h"
#include "StepperMotor.h"
#include "HMI_Command.h"

uint16_t EmergencyTimeCnt = 0;
uint16_t sensorTimeCnt = 0;
int backforthCM = 0;
extern HardwareSerial *cmd_port;
extern HardwareSerial *CNC_cmd_port;
extern HMI_Command *hmicmd;
DigitalIO digitalio;
Adafruit_MCP23017 extio[EXTIO_NUM];

MainDataStruct maindata;
RuntimeStatus runtimedata;

StepperMotor *Motor[3];

void buzzerPlay(int ms)
{
    pinMode(BUZZ, OUTPUT);
    digitalWrite(BUZZ, HIGH);
    delay(ms);
    digitalWrite(BUZZ, LOW);
}

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
	if(EmergencyTimeCnt < 0xFF00)
		EmergencyTimeCnt += TIMER_INTERVAL_MS;
	if(sensorTimeCnt < 0xFF00)
		sensorTimeCnt += TIMER_INTERVAL_MS;
}

void MainProcess_ReCheckEEPROMValue()
{
	uint8_t i; 
	
	if((maindata.HMI_ID < 0) || (maindata.HMI_ID > 128))
	{
		maindata.HMI_ID = 0;
		runtimedata.UpdateEEPROM = true;
	}
    
	for(i=0; i<3; i++)
		if((maindata.MotorSpeed[i] > MOTOR_VELOCITY_MAX) || (maindata.MotorSpeed[i] < 256))
		{
			maindata.MotorSpeed[i] = MOTOR_SPEED_NORMAL;
			runtimedata.UpdateEEPROM = true;
		}
    
    for(i=0; i<3; i++)
        if((maindata.MotorResolution[i] < 0) || (maindata.MotorResolution[i] > 200000))
        {
            maindata.MotorResolution[i] = 400;
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
    Motor[0] = new StepperMotor(12, A13, 10000, 5000);
//	Motor[0] = new StepperMotor(2, A8, maindata.MotorResolution[0], maindata.MotorSpeed[0]);
//	Motor[1] = new StepperMotor(7, A11, maindata.MotorResolution[1], maindata.MotorSpeed[1]);
//	Motor[2] = new StepperMotor(12, A13, maindata.MotorResolution[2], maindata.MotorSpeed[2]);

	Motor[0]->setLimitPin(InputPin[1], LOW, InputPin[0], LOW); //LOW時停下來
    
}

void ReadDigitalInput()
{
	uint8_t i,bi, j, value;
	String outstr = "";
	bool inputupdate = false;
	uint8_t input8 = 1;
	
	for(i=0; i<8; i++)
	{
		runtimedata.sensor[i] = digitalRead(InputPin[i]);
		if(runtimedata.sensor[i])
			{setbit(digitalio.Input[0], i);	}
		else
			{clrbit(digitalio.Input[0], i);	}
	}

	if(INPUT_8_NUMBER == 2)
	{
		for(i=0; i<8; i++)
		{
			runtimedata.sensor[i+8] = digitalRead(InputPin[i+8]);
			
			if(runtimedata.sensor[i+8])
				{setbit(digitalio.Input[1], i); }
			else
				{clrbit(digitalio.Input[1], i); }
		}
		input8 += 1;
	}

	if(EXTIO_NUM > 0)
	{
		for(i=0; i<8; i++)
		{
			runtimedata.sensor[i+8] = extio[0].digitalRead(i+8);
				
			if(runtimedata.sensor[i+input8*8])
				{setbit(digitalio.Input[input8], i);	}
			else
				{clrbit(digitalio.Input[input8], i);	}
		}
		input8 += 1;
	}
	if(EXTIO_NUM > 1)
	{
		for(i=0; i<8; i++)
		{
			runtimedata.sensor[i+input8*8] = extio[1].digitalRead(i+8);
			if(runtimedata.sensor[i+input8*8])
				{setbit(digitalio.Input[input8], i);	}
			else
				{clrbit(digitalio.Input[input8], i);	}
		}
	}

}

void WriteDigitalOutput()
{
	uint8_t i,bi, j, value;

	for(i=0; i<OUTPUT_8_NUMBER+EXTIO_NUM; i++)
	{
		if(digitalio.PreOutput[i] != digitalio.Output[i])
		{
			digitalio.PreOutput[i] = digitalio.Output[i];
			
			switch(i)
			{
				case 0: //onboard
					for(bi=0; bi<8; bi++)
					{
						value = getbit(digitalio.Output[i], bi);
						digitalWrite(OutputPin[bi], value);
					}
					break;

				case 1: //extern board 0
					for(bi=0; bi<8; bi++)
					{
						value = getbit(digitalio.Output[i], bi);
						if(OUTPUT_8_NUMBER == 2)
							digitalWrite(OutputPin[bi+8], value);
						else
							extio[0].digitalWrite(bi, value);
					}
					break;
				case 2: //extern board 1
					for(bi=0; bi<8; bi++)
					{
						value = getbit(digitalio.Output[i], bi);
						if(OUTPUT_8_NUMBER == 2)
							extio[0].digitalWrite(bi, value);
						else
							extio[1].digitalWrite(bi, value);
					}
					break;
				case 3: //extern board 1
					for(bi=0; bi<8; bi++)
					{
						value = getbit(digitalio.Output[i], bi);
						extio[1].digitalWrite(bi, value);
					}
					break;

			}	
		}
	}
}

void MainProcess_Task()  
{
    //ReadPositionSensor();
	switch(runtimedata.RunMode)
	{
	    case RUN_MODE_STOP:
            if(Motor[MOTOR_X]->getState() != MOTOR_STATE_STOP){
                Motor[MOTOR_X]->Stop();
            }
            if(runtimedata.IndicationEmergency){
                runtimedata.RunMode = RUN_MODE_EMERGENCY;
            }
            break;
		case RUN_MODE_GO_HOME:
            if(runtimedata.Workindex[WORKINDEX_GO_HOME] == -1){
                runtimedata.Workindex[WORKINDEX_GO_HOME] = 0;
            }
            if(Go_Home()){
                runtimedata.RunMode = RUN_MODE_STOP;
            }
			break;
        case RUN_MODE_EMERGENCY:
            if(EmergencyTimeCnt > 1000){
                EmergencyTimeCnt = 0;
                hmicmd->Indication_Emergency();
            }
            break;
	}
    
}

bool Go_Home()
{
    bool result = false;
    switch(runtimedata.Workindex[WORKINDEX_GO_HOME]){
        case 0:
            Motor[MOTOR_X]->Speed(SPEED_GO_HOME);
            runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
            break;
        case 10:
            if(Motor[MOTOR_X]->getState() == MOTOR_STATE_STOP){
                runtimedata.Workindex[WORKINDEX_GO_HOME] = -1;
            }
            break;
        default:
            result = true;
            break;
    }
    return result;
}
void ReadPositionSensor()
{
    for(int i = 0; i<13; i++){
        if(digitalRead(InputPin[i]))
            {setbit(runtimedata.ReadInput, i);  }
        else
            {clrbit(runtimedata.ReadInput, i);  }
    }
    if(runtimedata.ReadInput != 0){
        runtimedata.PositionInput = runtimedata.ReadInput;
    }
}



