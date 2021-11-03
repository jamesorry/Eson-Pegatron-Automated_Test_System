#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "HMI.h"
#include "Timer.h"
#include "StepperMotor.h"
#include "HMI_Command.h"

uint16_t EmergencyTimeCnt = 0;
uint16_t sensorTimeCnt = 0;
extern HardwareSerial *cmd_port;
extern HMI_Command *hmicmd;
DigitalIO digitalio;
Adafruit_MCP23017 extio[EXTIO_NUM];

MainDataStruct maindata;
RuntimeStatus runtimedata;

StepperMotor *Motor[MOTOR_TOTAL];

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
	if((maindata.HMI_ID != 0))
	{
		maindata.HMI_ID = 0;
	}
	if(maindata.CheckVersion != 110301){
        maindata.TargetPosition = 0;
        for(uint8_t i=0; i<MOTOR_TOTAL; i++){
            maindata.MotorSpeed[i] = 3000;
            maindata.MotorFrequenceStart[i] = 1000;
            maindata.MotorAccelerateTime[i] = 500;
        }
        maindata.CheckVersion = 110301;
    }
	runtimedata.UpdateEEPROM = true;
}

void MainProcess_Init()
{

	int i,j;
	runtimedata.UpdateEEPROM = false;
	MainProcess_ReCheckEEPROMValue();
//	runtimedata.RunMode = RUN_MODE_NORMAL;
    runtimedata.RunMode = RUN_MODE_GO_HOME;

	for(i=0; i<WORKINDEX_TOTAL; i++)
		runtimedata.Workindex[i] = 0;
	for(i=0; i<WORKINDEX_TOTAL; i++)
		runtimedata.preWorkindex[i] = -1;
		
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
	Motor[MOTOR_X] = new StepperMotor(2, A8, 10000, 1000);
    Motor[MOTOR_X]->setLimitPin(InputPin[IN01_FrontLimitPin], LOW, InputPin[IN13_BackLimitPin], LOW); //LOW時停下來
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
    ReadPositionSensor();
    if(runtimedata.preRunMode != runtimedata.RunMode)
    {
        runtimedata.preRunMode = runtimedata.RunMode;
        cmd_port->println("RunMode: " + String(runtimedata.preRunMode));
    }
	switch(runtimedata.RunMode)
	{
        case RUN_MODE_EMERGENCY:
            if(EmergencyTimeCnt > 1500 && runtimedata.IndicationEmergency){
                EmergencyTimeCnt = 0;
                hmicmd->Indication_Emergency();
            }
            if(!digitalRead(InputPin[IN00_EmergencyPin]))
            {//解除緊急停止開關
                runtimedata.IndicationEmergency = false;
                runtimedata.RunMode = RUN_MODE_GO_HOME;                
                for(uint8_t i=0; i<WORKINDEX_TOTAL; i++)
                    runtimedata.Workindex[i] = 0;
            }
            break;
	    case RUN_MODE_STOP:
            if(Motor[MOTOR_X]->getState() != MOTOR_STATE_STOP){
                Motor[MOTOR_X]->Stop();
                runtimedata.RunMode = RUN_MODE_NORMAL;
            }
            if(runtimedata.IndicationEmergency){
                runtimedata.RunMode = RUN_MODE_EMERGENCY;
            }
            break;
		case RUN_MODE_GO_HOME://尋找原點
            if(Go_Home()){
                runtimedata.Workindex[WORKINDEX_INIT] = 0;
                runtimedata.RunMode = RUN_MODE_INIT;
            }
			break;
        case RUN_MODE_INIT://移動到上次記憶位置
            if(MotorInit()){
                runtimedata.RunMode = RUN_MODE_NORMAL;
            }
            break;
        case RUN_MODE_NORMAL:
            break;
	}
    
}
bool MotorInit()
{
    bool result = false;
    if(runtimedata.preWorkindex[WORKINDEX_INIT] != runtimedata.Workindex[WORKINDEX_INIT])
    {
        runtimedata.preWorkindex[WORKINDEX_INIT] = runtimedata.Workindex[WORKINDEX_INIT];
        cmd_port->println("WORKINDEX_INIT: " + String(runtimedata.preWorkindex[WORKINDEX_INIT]));
    }
    switch(runtimedata.Workindex[WORKINDEX_INIT]){
        case 0:
            if(Motor[MOTOR_X]->getState() != MOTOR_STATE_STOP){
                Motor[MOTOR_X]->Stop();
            }
            runtimedata.Workindex[WORKINDEX_INIT] += 10;
            break;
        case 10:
            Motor[MOTOR_X]->MoveTo(maindata.TargetPosition, maindata.MotorSpeed[MOTOR_X]);
            runtimedata.Workindex[WORKINDEX_INIT] += 10;
            break;
        case 20:
            if(Motor[MOTOR_X]->getState() == MOTOR_STATE_STOP)
                runtimedata.Workindex[WORKINDEX_INIT] += 10;
            break;
        default:
            result = true;
            break;
    }
    return result;
}
bool Go_Home()
{
    bool result = false;
    if(runtimedata.preWorkindex[WORKINDEX_GO_HOME] != runtimedata.Workindex[WORKINDEX_GO_HOME])
    {
        runtimedata.preWorkindex[WORKINDEX_GO_HOME] = runtimedata.Workindex[WORKINDEX_GO_HOME];
        cmd_port->println("WORKINDEX_GO_HOME: " + String(runtimedata.preWorkindex[WORKINDEX_GO_HOME]));
    }
    switch(runtimedata.Workindex[WORKINDEX_GO_HOME]){
        case 0:
            if(Motor[MOTOR_X]->getState() != MOTOR_STATE_STOP){
                Motor[MOTOR_X]->Stop();
            }
            runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
            break;
        case 10:
            Motor[MOTOR_X]->Speed(-SPEED_GO_HOME);
            runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
            break;
        case 20:
            if(Motor[MOTOR_X]->getState() == MOTOR_STATE_STOP){
                runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
            }
            break;
        case 30:
            MotorServoSearchSensor(IN02_HomePin, HIGH ,MOTOR_CW);
            runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
            break;
        case 40:
            if(Motor[MOTOR_X]->getState() == MOTOR_STATE_STOP){
                Motor[MOTOR_X]->setPosition(0); 
                runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
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
    for(int i = 2; i<13; i++){
        if(digitalRead(InputPin[i]))
            {setbit(runtimedata.ReadInput, i-2);  }
        else
            {clrbit(runtimedata.ReadInput, i-2);  }
    }
    if(runtimedata.ReadInput != 0){
        runtimedata.PositionInput = runtimedata.ReadInput;
    }
}

void MotorServoSearchSensor(int pin, uint8_t HL, int dir,  int toggletimes)
{
	Motor[MOTOR_X]->setStopPin(InputPin[pin], HL, toggletimes);
	if(dir == MOTOR_CCW)
		Motor[MOTOR_X]->Speed(-SPEED_SEARCH_SENSOR, maindata.MotorFrequenceStart[MOTOR_X], maindata.MotorAccelerateTime[MOTOR_X]);
	else if(dir == MOTOR_CW)
		Motor[MOTOR_X]->Speed(SPEED_SEARCH_SENSOR, maindata.MotorFrequenceStart[MOTOR_X], maindata.MotorAccelerateTime[MOTOR_X]);
}
