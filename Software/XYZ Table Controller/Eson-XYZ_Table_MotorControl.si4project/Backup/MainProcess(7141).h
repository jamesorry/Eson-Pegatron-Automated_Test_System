#ifndef _MAIN_PROCESS_H_
#define _MAIN_PROCESS_H_

#include "Arduino.h"
#include "hmi.h"

#define MOTOR_X             0
#define SPEED_GO_HOME       1000

#define	EXTIO_NUM		0
#define	INPUT_8_NUMBER	1
#define OUTPUT_8_NUMBER	1

#define	OUTPUT_NONE_ACTIVE	0
#define	OUTPUT_ACTIVE		1

#define	INPUT_NONE_ACTIVE	0
#define	INPUT_ACTIVE		1

#define STATION_UNKNOW		-1
#define RUN_MODE_STOP		0
#define RUN_MODE_GO_HOME    1
#define RUN_MODE_EMERGENCY  2

#define WORKINDEX_TOTAL		4
#define WORKINDEX_GO_HOME   0
typedef struct _DigitalIO_
{
	uint8_t	Input[(INPUT_8_NUMBER+EXTIO_NUM)*8];
	uint8_t	Output[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];
	uint8_t PreOutput[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];
}DigitalIO;

typedef struct _MainDataStruct_
{
	char Vendor[10];
	uint8_t 	HMI_ID;
	char 		Mode;
	long		MotorSpeed[3];
	long		MotorFrequenceStart[3];
	long		MotorAccelerateTime[3];
    long		MotorResolution[3];
}MainDataStruct;


typedef struct _RuntimeStruct_
{
	int  	Workindex[WORKINDEX_TOTAL];
	int		preWorkindex[WORKINDEX_TOTAL];

	uint8_t sensor[INPUT_8_NUMBER*8 + EXTIO_NUM*8];
	uint8_t outbuf[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];

	bool 		UpdateEEPROM;
	int 		RunMode = 0;	
	bool	 	run;
	uint8_t		ErrorCode;
    
    uint16_t    ReadInput = 0x00;
    uint16_t    PositionInput = 0x00;
    bool        IndicationEmergency = false;
}RuntimeStatus;


#define TIME_SENSOR_CONFIRM			300
#define TIME_READ_RFID_FAIL			3000

#define MOTOR_SPEED_NORMAL		    1000

void MainProcess_Timer();
void MainProcess_Task();

void MainProcess_Init();
void setOutput(uint8_t index, uint8_t hl);
uint8_t getInput(uint8_t index);
void buzzerPlay(int ms);
void ReadPositionSensor();
bool Go_Home();

#endif	//_MAIN_PROCESS_H_
