#ifndef _MAIN_PROCESS_H_
#define _MAIN_PROCESS_H_

#include "Arduino.h"
#include "hmi.h"

#define MOTOR_X             0
#define MOTOR_TOTAL         1
#define SPEED_GO_HOME       6000
#define SPEED_SEARCH_SENSOR 1500

#define	EXTIO_NUM		0
#define	INPUT_8_NUMBER	1
#define OUTPUT_8_NUMBER	1

#define	OUTPUT_NONE_ACTIVE	0
#define	OUTPUT_ACTIVE		1

#define	INPUT_NONE_ACTIVE	0
#define	INPUT_ACTIVE		1

//總共有14個Sensor，前後極限各1個，急停1個，原點1個，位置10個
#define IN00_EmergencyPin       0
#define IN01_FrontLimitPin      13
#define IN02_HomePin            2
#define IN13_BackLimitPin       1

#define RUN_MODE_EMERGENCY		-1
#define RUN_MODE_STOP		    0
#define RUN_MODE_GO_HOME        1
#define RUN_MODE_INIT           2
#define RUN_MODE_NORMAL         3

#define WORKINDEX_TOTAL		4
#define WORKINDEX_GO_HOME   0
#define WORKINDEX_INIT      1

typedef struct _DigitalIO_
{
	uint8_t	Input[(INPUT_8_NUMBER+EXTIO_NUM)*8];
	uint8_t	Output[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];
	uint8_t PreOutput[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];
}DigitalIO;

typedef struct _MainDataStruct_
{
	char        Vendor[10];
	uint8_t 	HMI_ID;
	long		MotorSpeed[MOTOR_TOTAL];
	long		MotorFrequenceStart[MOTOR_TOTAL];
	long		MotorAccelerateTime[MOTOR_TOTAL];
    long        CheckVersion;
    long        TargetPosition;
}MainDataStruct;


typedef struct _RuntimeStruct_
{
	int  	Workindex[WORKINDEX_TOTAL];
	int		preWorkindex[WORKINDEX_TOTAL];

	uint8_t sensor[INPUT_8_NUMBER*8 + EXTIO_NUM*8];
	uint8_t outbuf[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];

	bool 		UpdateEEPROM;
	int 		RunMode = 0;
    int 		preRunMode = -1;
    uint16_t    ReadInput = 0x00;
    uint16_t    PositionInput = 0x00;
    bool        IndicationEmergency = false;
    bool        NeedRestart = false;
}RuntimeStatus;

#define MOTOR_SPEED_NORMAL		    1500

void MainProcess_Timer();
void MainProcess_Task();

void MainProcess_Init();
void setOutput(uint8_t index, uint8_t hl);
uint8_t getInput(uint8_t index);
void buzzerPlay(int ms);
void ReadPositionSensor();
bool Go_Home();
bool MotorInit();
void MotorServoSearchSensor(int pin, uint8_t HL, int dir=0, int toggletimes=0);

#endif	//_MAIN_PROCESS_H_
