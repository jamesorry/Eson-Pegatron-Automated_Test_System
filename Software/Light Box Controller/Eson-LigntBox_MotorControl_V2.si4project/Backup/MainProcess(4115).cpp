#include "Arduino.h"
#include "MainProcess.h"
#include <Adafruit_MCP23017.h>
#include "HMI.h"
#include "Timer.h"
#include "StepperMotor.h"
#include "HMI_Command.h"

uint16_t EmergencyTimeCnt = 0;
extern HardwareSerial *cmd_port;
extern HMI_Command *hmicmd;
DigitalIO digitalio;
Adafruit_MCP23017 extio[EXTIO_NUM];
MainDataStruct maindata;
RuntimeStatus runtimedata;
StepperMotor *Motor[MOTOR_TOTAL];


void MainProcess_Timer()
{
	if(EmergencyTimeCnt < 0xFF00)
		EmergencyTimeCnt += TIMER_INTERVAL_MS;
}

void MainProcess_ReCheckEEPROMValue()
{
	if((maindata.HMI_ID < 0) || (maindata.HMI_ID > 128))
	{
		maindata.HMI_ID = 0;
	}
	if((maindata.OffsetDistanceOfStopPin < 0))
	{
		maindata.OffsetDistanceOfStopPin = 0;
	}
	if((maindata.TargetStation < 1) || maindata.TargetStation > 3)
	{
		maindata.TargetStation = 2;
	}
    if(maindata.CheckVersion != 101002){
        for(uint8_t i=0; i<8; i++){
            maindata.Output_Last_HighLow[i] = 0;    
        }
        maindata.OffsetDistanceOfStopPin = 0;
        maindata.VR_HomeOffset = 0;
        maindata.TargetStation = 2;
        for(uint8_t i=0; i<2; i++){
            maindata.MotorSpeed[i] = 1500;
            maindata.MotorFrequenceStart[i] = 1000;
            maindata.MotorAccelerateTime[i] = 300;
        }
        for(uint8_t i=0; i<3; i++)
            maindata.StationPosition[i] = 0;
        maindata.CheckVersion = 101002;
    }
    runtimedata.UpdateEEPROM = true;
}


void MainProcess_Init()
{
	int i,j;
	runtimedata.UpdateEEPROM = false;

	MainProcess_ReCheckEEPROMValue();
    
	runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_INIT;
    runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_INIT;
//	runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_STOP;
//    runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_STOP;
    
	for(i=0; i<WORKINDEX_TOTAL; i++)
		runtimedata.Workindex[i] = 0;
    for(i=0; i<WORKINDEX_TOTAL; i++)
        runtimedata.preWorkindex[i] = -1;

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

	
	Motor[MOTOR_SERVO] = new StepperMotor(2, A8, 10000, maindata.MotorSpeed[0]);
	Motor[MOTOR_VR] = new StepperMotor(7, A11, 10000, maindata.MotorSpeed[1]);
	
	Motor[MOTOR_SERVO]->setLimitPin(InputPin[IN01_FrontLimitPin], LOW, InputPin[IN05_BackLimitPin], LOW);
	Motor[MOTOR_VR]->setLimitPin(InputPin[IN06_VR_FrontLimitPin], LOW, InputPin[IN07_VR_BackLimitPin], LOW);
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

void OutputBuf2Byte()
{
	uint8_t i, bi, byteindex;
	for(i=0; i< (OUTPUT_8_NUMBER+EXTIO_NUM)*8; i++)
	{
		byteindex = i / 8;
		bi = i % 8;
		if(runtimedata.outbuf[i] == 1)
			setbit(digitalio.Output[byteindex], bi);
		else
			clrbit(digitalio.Output[byteindex], bi);
	}
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

void MainProcess_Task()  // This is a task.
{
    ReadPositionSensor();
    MotorVRStatus();
    MotorServoStatus();
}

void MotorServoStatus()
{
	if(runtimedata.preRunMode[MOTOR_SERVO] != runtimedata.RunMode[MOTOR_SERVO])
	{
		runtimedata.preRunMode[MOTOR_SERVO] = runtimedata.RunMode[MOTOR_SERVO];
		cmd_port->println("Servo RunMode: " + String(runtimedata.RunMode[MOTOR_SERVO]));
	}
	switch(runtimedata.RunMode[MOTOR_SERVO])
	{
        case RUN_MODE_EMERGENCY:
            if(EmergencyTimeCnt > 3000){
                EmergencyTimeCnt = 0;
//                hmicmd->Indication_Emergency();
                DEBUG("Indication_Emergency()");
            }
            if(!digitalRead(InputPin[IN00_EmergencyPin])){
                runtimedata.IndicationEmergency = false;
//                runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_STOP;
                runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_INIT;
                runtimedata.Workindex[WORKINDEX_SERVO_INITIAL] = 0;
                runtimedata.Workindex[WORKINDEX_GO_HOME] = 0;
                runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0;
            }
            break;
	    case RUN_MODE_SERVO_STOP:
            if(Motor[MOTOR_SERVO]->getState() != MOTOR_STATE_STOP){
                Motor[MOTOR_SERVO]->Stop();
            }
            if(runtimedata.IndicationEmergency){
                runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_EMERGENCY;
            }
            else
                runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_NORMAL;
            break;
		case RUN_MODE_SERVO_INIT:
			if(MotorServoInit())
            {
				runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_SEARCH_SENSOR;
				Motor[MOTOR_SERVO]->setFrequence(maindata.MotorSpeed[MOTOR_SERVO]);
		    }
			break;
        case RUN_MODE_SERVO_NORMAL:
            
            break;
        case RUN_MODE_SERVO_SEARCH_SENSOR:
            if(LightBoxSearchSensor()){
                runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_STOP;
            }
            break;
    }
}

void MotorVRStatus()
{
	if(runtimedata.preRunMode[MOTOR_VR] != runtimedata.RunMode[MOTOR_VR])
	{
		runtimedata.preRunMode[MOTOR_VR] = runtimedata.RunMode[MOTOR_VR];
		cmd_port->println("VR RunMode: " + String(runtimedata.RunMode[MOTOR_VR]));
	}
	switch(runtimedata.RunMode[MOTOR_VR])
    {
        case RUN_MODE_EMERGENCY:
            if(EmergencyTimeCnt > 3000){
                EmergencyTimeCnt = 0;
//                hmicmd->Indication_Emergency();
                DEBUG("Indication_Emergency()");
            }
            if(!digitalRead(InputPin[IN00_EmergencyPin])){
                runtimedata.IndicationEmergency = false;
//                runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_STOP;
                runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_INIT;
                runtimedata.Workindex[WORKINDEX_VR_INITIAL] = 0;
                runtimedata.Workindex[WORKINDEX_VR_GO_HOME] = 0;
            }
            break;
        case RUN_MODE_VR_STOP:
            if(Motor[MOTOR_VR]->getState() != MOTOR_STATE_STOP){
                Motor[MOTOR_VR]->Stop();
            }
            if(runtimedata.IndicationEmergency){
                runtimedata.RunMode[MOTOR_VR] = RUN_MODE_EMERGENCY;
            }
            else
                runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_NORMAL;
            break;
        case RUN_MODE_VR_INIT:
            if(MotorVRInit()) 
            {
                runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_IO_INIT;
                Motor[MOTOR_VR]->setFrequence(maindata.MotorSpeed[MOTOR_VR]);
            }
            break;
        case RUN_MODE_VR_IO_INIT:
            for(uint8_t i=0; i<8; i++)
                digitalWrite(OutputPin[i], maindata.Output_Last_HighLow[i]);
            runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_STOP;
            break;
        case RUN_MODE_VR_NORMAL:
            
            break;
    }
}

bool MotorServoInit()
{
	bool isfinish = false;
	if(runtimedata.preWorkindex[WORKINDEX_SERVO_INITIAL] != runtimedata.Workindex[WORKINDEX_SERVO_INITIAL])
	{
		runtimedata.preWorkindex[WORKINDEX_SERVO_INITIAL] = runtimedata.Workindex[WORKINDEX_SERVO_INITIAL];
		cmd_port->println("WORKINDEX_SERVO_INITIAL: " + String(runtimedata.Workindex[WORKINDEX_SERVO_INITIAL]));
	}
	switch(runtimedata.Workindex[WORKINDEX_SERVO_INITIAL])
	{
		case 0:
            runtimedata.Station = 0;//若開始尋找原點，站號改為0
			runtimedata.Workindex[WORKINDEX_SERVO_INITIAL] += 10;
			break;
		case 10:
			if(MotorGoHome(0))
			{
			    DEBUG("MotorGoHome finish.");
                runtimedata.Workindex[WORKINDEX_SERVO_INITIAL] += 10;
                if(Motor[MOTOR_SERVO]->getAccelerateTime() == 0)
                    Motor[MOTOR_SERVO]->setAccelerateTime(200);
				if(Motor[0]->getDirection() == MOTOR_CW) //正轉
                {
                    Motor[0]->Steps(maindata.OffsetDistanceOfStopPin, maindata.MotorSpeed[0]);
                }
                else{
                    Motor[0]->Steps((-1)*maindata.OffsetDistanceOfStopPin, maindata.MotorSpeed[0]);
                }
			}
			break;
		case 20:
			if(Motor[0]->getState() == MOTOR_STATE_STOP)
			{
				runtimedata.Workindex[WORKINDEX_SERVO_INITIAL] = 0xF0;
                runtimedata.Station = getStationSensor();
                DEBUG("Now station: " + String(runtimedata.Station));   
				isfinish = true;
				cmd_port->println("WORKINDEX_SERVO_INITIAL isfinish: " + String(runtimedata.Workindex[WORKINDEX_SERVO_INITIAL]));
			}
			break;
	}
	return isfinish;
}
bool MotorVRInit()
{
	bool isfinish = false;
	if(runtimedata.preWorkindex[WORKINDEX_VR_INITIAL] != runtimedata.Workindex[WORKINDEX_VR_INITIAL])
	{
		runtimedata.preWorkindex[WORKINDEX_VR_INITIAL] = runtimedata.Workindex[WORKINDEX_VR_INITIAL];
		cmd_port->println("WORKINDEX_VR_INITIAL: " + String(runtimedata.Workindex[WORKINDEX_VR_INITIAL]));
	}
	switch(runtimedata.Workindex[WORKINDEX_VR_INITIAL])
	{
		case 0:
			runtimedata.Workindex[WORKINDEX_VR_INITIAL] += 10;
			break;
		case 10:
			if(VRGoHome(1))
			{
				if(Motor[1]->getPosition() == maindata.VR_HomeOffset)
					runtimedata.Workindex[WORKINDEX_VR_INITIAL] += 10;
				else
				{
					runtimedata.Workindex[WORKINDEX_VR_INITIAL] += 0xF0;
				}
			}
			break;
		case 20:
			if(Motor[1]->getState() == MOTOR_STATE_STOP)
			{
				runtimedata.Workindex[WORKINDEX_VR_INITIAL] = 0xF0;
				isfinish = true;
			}
			break;
	}
	return isfinish;
}

bool MotorGoHome(int motornum)
{
	bool isfinish = false;
	if(runtimedata.preWorkindex[WORKINDEX_GO_HOME] != runtimedata.Workindex[WORKINDEX_GO_HOME])
	{
		runtimedata.preWorkindex[WORKINDEX_GO_HOME] = runtimedata.Workindex[WORKINDEX_GO_HOME];
		cmd_port->println("WORKINDEX_GO_HOME: " + String(runtimedata.Workindex[WORKINDEX_GO_HOME]));
	}
	switch(runtimedata.Workindex[WORKINDEX_GO_HOME])
	{
		case 0:
			if(motornum == 0) MotorServoSearchSensor();//回原點找極限
			runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
			break;
		case 10:
			if(Motor[motornum]->getState() == MOTOR_STATE_STOP){
				runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
                if(motornum == 1)
                    Motor[motornum]->setPosition(0);    
            }
			break;
		case 20:
			goHome(motornum);
			runtimedata.Workindex[WORKINDEX_GO_HOME] += 10;
			break;
		case 30:
			if(motornum == 0)
            {
				if((Motor[motornum]->getState() == MOTOR_STATE_STOP) 
                    && (getInput(IN03_Pos_2_Pin) == HIGH))
				{
				    DEBUG("Servo finish go home.");
                    Motor[motornum]->setPosition(0);
					runtimedata.Workindex[WORKINDEX_GO_HOME] = 0xE0;
				}
			}
			break;
		case 0xE0:
			isfinish = true;
			runtimedata.Workindex[WORKINDEX_GO_HOME] = 0xF0;
			break;
	}
	return isfinish;
}
bool VRGoHome(int motornum)
{
    bool isfinish = false;
    if(runtimedata.preWorkindex[WORKINDEX_VR_GO_HOME] != runtimedata.Workindex[WORKINDEX_VR_GO_HOME])
    {
        runtimedata.preWorkindex[WORKINDEX_VR_GO_HOME] = runtimedata.Workindex[WORKINDEX_VR_GO_HOME];
        cmd_port->println("WORKINDEX_VR_GO_HOME: " + String(runtimedata.Workindex[WORKINDEX_VR_GO_HOME]));
    }
    switch(runtimedata.Workindex[WORKINDEX_VR_GO_HOME])
    {
        case 0:
            if(motornum == 1) MotorVRSearchSensor();
            runtimedata.Workindex[WORKINDEX_VR_GO_HOME] += 10;
            break;
        case 10:
            if(Motor[motornum]->getState() == MOTOR_STATE_STOP){
                runtimedata.Workindex[WORKINDEX_VR_GO_HOME] += 10;
                if(motornum == 1)
                    Motor[motornum]->setPosition(0);
            }
            break;
        case 20:
            goHome(motornum);
            runtimedata.Workindex[WORKINDEX_VR_GO_HOME] += 10;
            break;
        case 30:
            if(motornum == 1)
            {
                if(Motor[motornum]->getState() == MOTOR_STATE_STOP)
                {
                    DEBUG("Now VR postion: " + String(Motor[MOTOR_VR]->getPosition()));
                    runtimedata.Workindex[WORKINDEX_VR_GO_HOME] = 0xE0;
                }
            }
            break;
        case 0xE0:
            isfinish = true;
            runtimedata.Workindex[WORKINDEX_VR_GO_HOME] = 0xF0;
            break;
    }
    return isfinish;
}
void MotorServoSearchSensor(int pin, int dir, int toggletimes)
{
	int pinindex = pin;
	uint8_t HL = HIGH;

	Motor[0]->setStopPin(InputPin[pinindex], HL, toggletimes);
	if(dir == MOTOR_CCW)
		Motor[0]->Speed(-maindata.MotorSpeed[MOTOR_SERVO]);
	else if(dir == MOTOR_CW)
		Motor[0]->Speed(maindata.MotorSpeed[MOTOR_SERVO]);
}

void MotorServoSearchSensor(int dir, int toggletimes)
{
	int pinindex = IN05_BackLimitPin;
	uint8_t HL = LOW;

	Motor[0]->setStopPin(InputPin[pinindex], HL, toggletimes);
	if(dir < 0)
		Motor[0]->Speed(-SPEED_SERVO_GOHOME);
	else
		Motor[0]->Speed(SPEED_SERVO_GOHOME);
}

void MotorVRSearchSensor(int dir, int toggletimes)
{
	int pinindex = IN07_VR_BackLimitPin;
	uint8_t HL = LOW;

	Motor[1]->setStopPin(InputPin[pinindex], HL, toggletimes);
	if(dir < 0)
		Motor[1]->Speed(-SPEED_SERVO_GOHOME);
	else
		Motor[1]->Speed(SPEED_SERVO_GOHOME);
}


void goHome(int motornum)
{
	if(motornum == 0)
	{
		Motor[motornum]->setStopPin(InputPin[IN03_Pos_2_Pin], HIGH, 0);
		Motor[motornum]->Speed(maindata.MotorSpeed[motornum]);
	}
    else if(motornum == 1)
    {   
        DEBUG(maindata.VR_HomeOffset);
        Motor[motornum]->MoveTo(maindata.VR_HomeOffset, maindata.MotorSpeed[motornum]);
    }
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
            DEBUG("Start SearchSensor.");
            if(maindata.TargetStation == 3 && runtimedata.Station == 1){
                runtimedata.SerarchPin = maindata.TargetStation + 1;
                runtimedata.SerarchDir = MOTOR_CCW;
    			runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
            }
            else{
                DEBUG("Result: " + String(maindata.TargetStation - runtimedata.Station));
                if(maindata.TargetStation - runtimedata.Station > 0){
                    runtimedata.SerarchPin = maindata.TargetStation + 1;
                    runtimedata.SerarchDir = MOTOR_CCW;
                    runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
                }
                else if(maindata.TargetStation - runtimedata.Station < 0){
                    runtimedata.SerarchPin = maindata.TargetStation + 1;
                    runtimedata.SerarchDir = MOTOR_CW;                    
                    runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
                }
                else{
                    runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0xE0;
                }
            }
#if MAIN_PROCESS_DEBUG
            cmd_port->println("Start flatcar search station sensor.");
            cmd_port->println("Pin: " + String(runtimedata.SerarchPin));
            cmd_port->println("Dir: " + String(runtimedata.SerarchDir));
#endif
			break;
		case 10:
			MotorServoSearchSensor(runtimedata.SerarchPin, runtimedata.SerarchDir, searchtoggletimes);
			runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
			break;
		case 20:
			if(Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
			{
				if(getStationSensor() != 0)
					runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] += 10;
				else //沒有在站號上(基本上不會遇到此情況)
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
					Motor[MOTOR_SERVO]->Steps(steps, maindata.MotorSpeed[MOTOR_SERVO]);
					runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0xE0;
				}
			}
			break;
		case 0xE0:
			{
				runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0xF0;
				runtimedata.Station = 0;
				int station = getStationSensor();
				if((station > 0) && (station <= 3))
				{
					runtimedata.Station = station;
                    DEBUG(runtimedata.Station);
				}
				isfinish = true;
#if MAIN_PROCESS_DEBUG
				cmd_port->println("Search station sensor process stop, Station:" + String(getStationSensor()));
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

void ReadPositionSensor()
{
    for(int i = 2; i<5; i++){
        if(digitalRead(InputPin[i]))
            {setbit(runtimedata.ReadInput, i-2);  }
        else
            {clrbit(runtimedata.ReadInput, i-2);  }
    }
    if(runtimedata.ReadInput != 0){
        runtimedata.PositionInput = runtimedata.ReadInput;
    }
}


void buzzerPlay(int ms)
{
    pinMode(BUZZ, OUTPUT);
    digitalWrite(BUZZ, HIGH);
    delay(ms);
    digitalWrite(BUZZ, LOW);
}

