#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "HMI.h"
#include "Timer.h"
#include "StepperMotor.h"

uint16_t CheckTimeCnt = 0;
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
	if(CheckTimeCnt < 0xFF00)
		CheckTimeCnt += TIMER_INTERVAL_MS;
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

	Motor[2]->setLimitPin(InputPin[0], LOW, InputPin[1], LOW);
}

void MainProcess_Task()
{
    
}

bool LightBoxMoveToStation()
{
	bool isfinish = false;
		
#if MAIN_PROCESS_DEBUG
	if(runtimedata.preWorkindex[WORKINDEX_MOVE_TO_STATION] != runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION])
	{
		runtimedata.preWorkindex[WORKINDEX_MOVE_TO_STATION] = runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION];
		cmd_port->println("WORKINDEX_MOVE_TO_STATION: " + String(runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION]));
	}
#endif
	switch(runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION])
	{
		case 0:
#if MAIN_PROCESS_DEBUG
			cmd_port->println("Move to Station " + String(runtimedata.TargetStation) + " Position:" + String(maindata.StationPosition[runtimedata.TargetStation]));
#endif
			
			if((runtimedata.TargetStation >= STATION_TOTAL) || (runtimedata.TargetStation <= 0) 
				|| (runtimedata.Station >= STATION_TOTAL)  || (runtimedata.Station <= 0))
			{
				runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION] = 0xF0;
				isfinish = true;
			}
			else
			{
				if((runtimedata.TargetStation == 3) && (runtimedata.Station == 1))
					runtimedata.TargetStation = 3;
				else if(runtimedata.TargetStation > runtimedata.Station)
					runtimedata.TargetStation = runtimedata.Station + 1;
				else if((runtimedata.TargetStation == 1) && (runtimedata.Station == 3))
					runtimedata.TargetStation = 1;
				else if(runtimedata.TargetStation < runtimedata.Station)
					runtimedata.TargetStation = runtimedata.Station - 1;
				if(runtimedata.TargetStation < 1)
					runtimedata.TargetStation = 3;
				else if (runtimedata.TargetStation > 3)
					runtimedata.TargetStation = 1;
					
#if MAIN_PROCESS_DEBUG
				cmd_port->println("Target Station: " + String(runtimedata.TargetStation) + ", & 0x01:" + String(runtimedata.TargetStation & 0x01));
#endif
				runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION] += 10;
			}
				
			break;
		case 10:
			Motor[MOTOR_SERVO]->setFrequenceStartup(1000);
			Motor[MOTOR_SERVO]->setAccelerateTime(TIME_FLATCAR_ACCELERATE);
			Motor[MOTOR_SERVO]->setFrequence(maindata.MotorSpeed[MOTOR_SERVO]);
			Motor[MOTOR_SERVO]->setStopRangePin(InputPin[IN02_IN_POSITION], HIGH, 0, 50);	//Targetposition +/- 50mm
			Motor[MOTOR_SERVO]->MoveToStopPin(maindata.StationPosition[runtimedata.TargetStation]);
			runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION] += 10;
			break;
		case 20:
			if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
			{	//shift 3.5mm
				int steps = 0;
				if(Motor[MOTOR_SERVO]->getDirection() == MOTOR_CW)
					steps = maindata.OffsetDistanceOfStopPin;
				else
					steps = -maindata.OffsetDistanceOfStopPin;
				if(Motor[MOTOR_SERVO]->getAccelerateTime() == 0)
					Motor[MOTOR_SERVO]->setAccelerateTime(200);
				Motor[MOTOR_SERVO]->Steps(steps);
				runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION] = 0xE0;
			}
		case 0xE0:
			if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
			{
				if(Motor[MOTOR_SERVO]->getPosition() == maindata.StationPosition[runtimedata.TargetStation])
					runtimedata.Station = runtimedata.TargetStation;
				runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION] = 0xF0;
				isfinish = true;
#if MAIN_PROCESS_DEBUG
				cmd_port->println("Motor to station process stop, Position:" + String(Motor[MOTOR_SERVO]->getPosition()));
#endif
			}
			break;
		case 0xEF:
			if(Motor[MOTOR_SERVO]->getLimitPinState() == HIGH)
				runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION] = 0x00;
			break;
	}
	if(Motor[MOTOR_SERVO]->getLimitPinState() == LOW)
	{
		runtimedata.Workindex[WORKINDEX_MOVE_TO_STATION] = 0xEF;
		isfinish = false;
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
		
#if MAIN_PROCESS_DEBUG
			cmd_port->println("Start flatcar search station sensor.");
			cmd_port->println("Dir: " + String(runtimedata.SerarchDir));
#endif
			
			searchtoggletimes = 0;
			searchcnt = 0;

			runtimedata.ProcessTimer[WORKINDEX_SEARCH_SENSOR] = millis();
			runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
			break;
		case 10:
			SearchSensor(runtimedata.SerarchDir, searchtoggletimes);
			runtimedata.ProcessTimer[WORKINDEX_SEARCH_SENSOR] = millis();
			runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
			break;
		case 20:
			if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
			{
				if(getStationSensor() != 0)
					runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
				else
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
					if(Motor[MOTOR_SERVO]->getDirection() == MOTOR_CW)
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
				if((station > 0) && (station < 12))
				{
					if(runtimedata.Station == STATION_UNKNOW)
						Motor[MOTOR_SERVO]->setPosition(maindata.StationPosition[station]);
					runtimedata.Station = station;
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
	
	station = getInput(IN21_STATION_B1) << 1;
	station += getInput(IN20_STATION_B0);
	return station;	
}

void SearchSensor(int dir, int toggletimes)
{
	int pinindex = IN02_IN_POSITION;
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


