#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "HMI.h"
#include "Timer.h"
#include "StepperMotor.h"
#include "HMI_Command.h"

uint16_t CheckTimeCnt = 0;
uint16_t EmergencyTimeCnt = 0;
extern HardwareSerial *cmd_port;
extern HardwareSerial *CNC_cmd_port;
extern HMI_Command *hmicmd;
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
	if(CheckTimeCnt < 0xFF00)
		CheckTimeCnt += TIMER_INTERVAL_MS;
    if(EmergencyTimeCnt < 0xFF00)
        EmergencyTimeCnt += TIMER_INTERVAL_MS;
}

void MainProcess_ReCheckEEPROMValue()
{
	if((maindata.HMI_ID < 0) || (maindata.HMI_ID > 128))
	{
		maindata.HMI_ID = 0;
		runtimedata.UpdateEEPROM = true;
	}
    if(maindata.OffsetDistanceOfStopPin < 0){
       maindata.OffsetDistanceOfStopPin = 0;
       runtimedata.UpdateEEPROM = true;
    }
    if(maindata.TargetStation < 0 || maindata.TargetStation > 3){
       maindata.TargetStation = 2;
       runtimedata.UpdateEEPROM = true;
    }
    if(maindata.CheckVersion != 100601){
        maindata.CheckVersion = 100601;
        for(uint8_t i=0; i<8; i++)
            maindata.Output_Last_HighLow[i] = 0;
        runtimedata.UpdateEEPROM = true;
    }
}


void MainProcess_Init()
{
	int i,j;
	runtimedata.UpdateEEPROM = false;

	MainProcess_ReCheckEEPROMValue();
	runtimedata.RunMode = RUN_MODE_STOP;
    runtimedata.VR_RunMode = VR_RUN_MODE_STOP;
	for(i=0; i<WORKINDEX_TOTAL; i++)
		runtimedata.Workindex[i]  = 0;
    for(i=0; i<WORKINDEX_TOTAL; i++)
            runtimedata.preWorkindex[i] = -1;
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
        
    
    Motor[MOTOR_SERVO] = new StepperMotor(2, A8, 10000, 1000);    
    Motor[MOTOR_VR] = new StepperMotor(7, A11, 10000, 1000);

	Motor[MOTOR_SERVO]->setLimitPin(InputPin[IN01_FrontLimitPin], LOW, InputPin[IN05_BackLimitPin], LOW);
	Motor[MOTOR_VR]->setLimitPin(InputPin[IN06_VR_FrontLimitPin], LOW, InputPin[IN07_VR_BackLimitPin], LOW);
}

void MainProcess_Task()
{
    VR_Contorl();
    LightBox();
}
void VR_Contorl()
{
    switch(runtimedata.VR_RunMode)
    {
        case VR_RUN_MODE_STOP:
            if(Motor[MOTOR_SERVO]->getState() != MOTOR_STATE_STOP){
                Motor[MOTOR_SERVO]->Stop();
                runtimedata.VR_RunMode = VR_RUN_MODE_NORMAL;
            }
            break;
        case VR_RUN_MODE_INIT:
            if(VR_Init()){
                runtimedata.VR_RunMode = VR_RUN_MODE_STOP;
                for(int i=0; i<8; i++)
                    digitalWrite(OutputPin[i],maindata.Output_Last_HighLow[i]);
            }
            break;
        case VR_RUN_MODE_NORMAL:
            
            break;
    }
}

bool VR_Init()
{
    bool isfinish = false;
#if MAIN_PROCESS_DEBUG
    if(runtimedata.preWorkindex[WORKINDEX_VR_INITIAL] != runtimedata.Workindex[WORKINDEX_VR_INITIAL])
    {
        runtimedata.preWorkindex[WORKINDEX_VR_INITIAL] = runtimedata.Workindex[WORKINDEX_VR_INITIAL];
        cmd_port->println("WORKINDEX_VR_INITIAL: " + String(runtimedata.Workindex[WORKINDEX_VR_INITIAL]));
    }
#endif
    switch(runtimedata.Workindex[WORKINDEX_VR_INITIAL])
    {
        case 0:
            Motor[MOTOR_VR]->setStopPin(InputPin[IN06_VR_FrontLimitPin], LOW, 0);
            Motor[MOTOR_VR]->Speed(-SPEED_SERVO_SEARCH);
            runtimedata.Workindex[WORKINDEX_VR_INITIAL] += 10;
            break;
        case 10:
            if(Motor[MOTOR_VR]->getState() == MOTOR_STATE_STOP){
                Motor[MOTOR_VR]->setPosition(0);
            }
            runtimedata.Workindex[WORKINDEX_VR_INITIAL] += 10;
            break;
        case 20:
            Motor[MOTOR_VR]->MoveTo(maindata.VR_HomeOffset);
            runtimedata.Workindex[WORKINDEX_VR_INITIAL] = 0xE0;
            break;
        case 0xE0:
            if(Motor[MOTOR_VR]->getState() == MOTOR_STATE_STOP){
                runtimedata.Workindex[WORKINDEX_VR_INITIAL] = 0xF0;
                isfinish = true;
            }
            break;
    }
    return isfinish;
}

void LightBox()
{
    switch(runtimedata.RunMode)
	{
        case RUN_MODE_STOP:
            if(Motor[MOTOR_SERVO]->getState() != MOTOR_STATE_STOP){
                Motor[MOTOR_SERVO]->Stop();
            }
            if(runtimedata.IndicationEmergency){
                runtimedata.RunMode = RUN_MODE_EMERGENCY;
            }
            break;
		case RUN_MODE_INIT: //重啟後，需要先回到上次狀態
			if(RestartInit())
				runtimedata.RunMode = RUN_MODE_STOP;
			break;
		case RUN_MODE_SEARCH_SENSOR:
			if(LightBoxSearchSensor())
				runtimedata.RunMode = RUN_MODE_STOP;
			break;
//		case RUN_MODE_DEBUG:
//            if(Demo_debug())
//                runtimedata.RunMode = RUN_MODE_STOP;
//			break;
        case RUN_MODE_EMERGENCY:
            if(EmergencyTimeCnt > 1000){
                EmergencyTimeCnt = 0;
                hmicmd->Indication_Emergency();
            }
            if(digitalRead(InputPin[IN00_EmergencyPin])){
                runtimedata.IndicationEmergency = false;
                runtimedata.RunMode = RUN_MODE_STOP;
            }
            break;
		default:
			runtimedata.RunMode = RUN_MODE_NORMAL;
			break;
	}
}

#if 0
bool Demo_debug()
{
	bool isfinish = false;
#if MAIN_PROCESS_DEBUG
    if(runtimedata.preWorkindex[WORKINDEX_DEBUG] != runtimedata.Workindex[WORKINDEX_DEBUG])
    {
        runtimedata.preWorkindex[WORKINDEX_DEBUG] = runtimedata.Workindex[WORKINDEX_DEBUG];
        cmd_port->println("WORKINDEX_DEBUG: " + String(runtimedata.Workindex[WORKINDEX_DEBUG]));
    }
#endif
    switch(runtimedata.Workindex[WORKINDEX_DEBUG])
    {
        case 0:
            switch (runtimedata.Station)
            {
                case 0x00:
                    Motor[MOTOR_SERVO]->MoveTo(0);
                    break;
                case 0x01:
                    Motor[MOTOR_SERVO]->MoveTo(533);
                    break;
                case 0x02:
                    Motor[MOTOR_SERVO]->MoveTo(1066);
                    break;
            }
            runtimedata.Workindex[WORKINDEX_DEBUG] += 10;
            break;
        case 10:
            if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
                runtimedata.Workindex[WORKINDEX_DEBUG] += 10;
            break;
        default:
            isfinish = true;
            break;
    }
    return isfinish;
}
#endif

bool RestartInit()
{
    bool isfinish = false;
#if MAIN_PROCESS_DEBUG
        if(runtimedata.preWorkindex[WORKINDEX_INITIAL] != runtimedata.Workindex[WORKINDEX_INITIAL])
        {
            runtimedata.preWorkindex[WORKINDEX_INITIAL] = runtimedata.Workindex[WORKINDEX_INITIAL];
            cmd_port->println("WORKINDEX_INITIAL: " + String(runtimedata.Workindex[WORKINDEX_INITIAL]));
        }
#endif
    switch(runtimedata.Workindex[WORKINDEX_INITIAL])
    {
        case 0:
            Motor[MOTOR_SERVO]->setStopPin(InputPin[IN01_FrontLimitPin], LOW, 0);
            Motor[MOTOR_SERVO]->Speed(-SPEED_SERVO_SEARCH);
            runtimedata.Workindex[WORKINDEX_INITIAL] += 10;
            break;
        case 10:
			if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
                runtimedata.Workindex[WORKINDEX_INITIAL] += 10;
            break;
        case 20:
            Motor[MOTOR_SERVO]->setStopPin(InputPin[maindata.TargetStation+1], HIGH, 0);
            Motor[MOTOR_SERVO]->Speed(SPEED_SERVO_SEARCH);
            runtimedata.Workindex[WORKINDEX_INITIAL] += 10;
            break;
        case 30:
			if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
                runtimedata.Workindex[WORKINDEX_INITIAL] += 10;
            break;
        case 40:
            if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
            {   //shift ?mm
                int steps = 0;
                if(Motor[MOTOR_SERVO]->getDirection() == MOTOR_CW)//正轉
                    steps = maindata.OffsetDistanceOfStopPin;
                else
                    steps = -maindata.OffsetDistanceOfStopPin;
                if(Motor[MOTOR_SERVO]->getAccelerateTime() == 0)
                    Motor[MOTOR_SERVO]->setAccelerateTime(200);
                Motor[MOTOR_SERVO]->Steps(steps, SPEED_SERVO_SEARCH);
                runtimedata.Workindex[WORKINDEX_INITIAL] = 0xE0;
            }
            break;
        case 0xE0:
            runtimedata.Workindex[WORKINDEX_INITIAL] = 0xF0;
            runtimedata.Station = 0;
            int station = getStationSensor();
            if((station > 0) && (station < 3))
            {
                runtimedata.Station = station;
                DEBUG(runtimedata.Station);
            }
            isfinish = true;
            break;
    }
	return isfinish;
}


int searchtoggletimes = 0;
int searchcnt = 0;
bool LightBoxSearchSensor()
{
	bool isfinish = false;
#if MAIN_PROCESS_DEBUG
		if(runtimedata.preWorkindex[WORKINDEX_SEARCH_SENSOR] != runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR])
		{
			runtimedata.preWorkindex[WORKINDEX_SEARCH_SENSOR] = runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR];
			cmd_port->println("WORKINDEX_SEARCH_SENSOR: " + String(runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR]));
		}
#endif
	switch(runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR])
	{
		case 0:
			searchtoggletimes = 0;
			searchcnt = 0;
            if(maindata.TargetStation - runtimedata.Station > 0){
                runtimedata.SerarchPin = maindata.TargetStation+1;
                runtimedata.SerarchDir = MOTOR_CW;
            }
            else{
                runtimedata.SerarchPin = maindata.TargetStation+1;
                runtimedata.SerarchDir = MOTOR_CCW;
            }
#if MAIN_PROCESS_DEBUG
            cmd_port->println("Start flatcar search station sensor.");
            cmd_port->println("Pin: " + String(runtimedata.SerarchPin));
            cmd_port->println("Dir: " + String(runtimedata.SerarchDir));
#endif
			runtimedata.ProcessTimer[WORKINDEX_SEARCH_SENSOR] = millis();
			runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
			break;
		case 10:
			SearchSensor(runtimedata.SerarchPin, runtimedata.SerarchDir, searchtoggletimes);
			runtimedata.ProcessTimer[WORKINDEX_SEARCH_SENSOR] = millis();
			runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
			break;
		case 20:
			if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
			{
				if(getStationSensor() != 0)
					runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
				else //沒有在站號上
				{
					searchtoggletimes = 2;
					searchcnt ++;
					if(searchcnt < 3)
						runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 10;
					else
					{
						runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
					}
				}
			}
			break;
		case 30:
			{
				if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
				{	//shift ?mm
					int steps = 0;
					if(Motor[MOTOR_SERVO]->getDirection() == MOTOR_CW)//正轉
						steps = maindata.OffsetDistanceOfStopPin;
					else
						steps = -maindata.OffsetDistanceOfStopPin;
					if(Motor[MOTOR_SERVO]->getAccelerateTime() == 0)
						Motor[MOTOR_SERVO]->setAccelerateTime(200);
					Motor[MOTOR_SERVO]->Steps(steps, SPEED_SERVO_SEARCH);
					runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0xE0;
				}
			}
			break;
		case 0xE0:
			{
				runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0xF0;
				runtimedata.Station = 0;
				int station = getStationSensor();
				if((station > 0) && (station < 3))
				{
//					if(runtimedata.Station == STATION_UNKNOW)
//						Motor[MOTOR_SERVO]->setPosition(maindata.StationPosition[station]);
					runtimedata.Station = station;
                    DEBUG(runtimedata.Station);
				}
				isfinish = true;
#if MAIN_PROCESS_DEBUG
				cmd_port->println("FlatCar search station sensor process stop, Station:" + String(getStationSensor()));
#endif
			}
			break;
	}
	return isfinish;
}


uint8_t getStationSensor()
{
	uint8_t station = 0;
	uint8_t i;
    if(getInput(IN02_Pos_1_Pin)) station = 1;
    if(getInput(IN03_Pos_2_Pin)) station = 2;
    if(getInput(IN04_Pos_3_Pin)) station = 3;
    
	return station;	
}

void SearchSensor(int pin, int dir, int toggletimes)
{
	int pinindex = pin;
	uint8_t HL = HIGH;

#if MAIN_PROCESS_DEBUG
		cmd_port->println("Search sensor(" + String(pinindex) + " GPIO:" + String(InputPin[pinindex]) + ").");
#endif
	Motor[MOTOR_SERVO]->setStopPin(InputPin[pinindex], HL, toggletimes);
	if(dir < 0)
		Motor[MOTOR_SERVO]->Speed(-SPEED_SERVO_SEARCH);
	else
		Motor[MOTOR_SERVO]->Speed(SPEED_SERVO_SEARCH);
}

