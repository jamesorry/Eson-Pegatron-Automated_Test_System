#ifndef _MAIN_PROCESS_H_
#define _MAIN_PROCESS_H_

#include "Arduino.h"
#include "hmi.h"

#define MAIN_PROCESS_DEBUG              1

#define MOTOR_SERVO			            2
#define SPEED_SERVO_SEARCH              500
#define TIME_FLATCAR_ACCELERATE		    300

#define	EXTIO_NUM		                0
#define	INPUT_8_NUMBER	                1
#define OUTPUT_8_NUMBER	                1

#define	OUTPUT_NONE_ACTIVE	            0
#define	OUTPUT_ACTIVE		            1

#define	INPUT_NONE_ACTIVE	            0
#define	INPUT_ACTIVE		            1

#define IN02_IN_POSITION                2
#define IN21_STATION_B1                 1
#define IN20_STATION_B0                 0




#define STATION_UNKNOW					-1
#define STATION_TOTAL                   3

#define RUN_MODE_STOP					0

#define WORKINDEX_MAIN					0
#define WORKINDEX_INITIAL				1
#define WORKINDEX_GO_HOME				2
#define WORKINDEX_MOVE_TO_STATION		3
#define WORKINDEX_SEARCH_SENSOR			4
#define WORKINDEX_NORMAL_PROCESS		5
#define WORKINDEX_TOTAL					8

typedef struct _DigitalIO_
{
	uint8_t	Input[4];
	uint8_t	Output[4];
	uint8_t PreOutput[4];
}DigitalIO;

typedef struct _MainDataStruct_
{
	char Vendor[10];
	uint8_t 	HMI_ID;
	char 		Mode;
	long		MotorSpeed[3];
	long		MotorFrequenceStart[3];
	long		MotorAccelerateTime[3];
    long        StationPosition[3];
    uint16_t    OffsetDistanceOfStopPin;
}MainDataStruct;


typedef struct _RuntimeStruct_
{
	int  	Workindex[WORKINDEX_TOTAL];
	int		preWorkindex[WORKINDEX_TOTAL];
    uint32_t ProcessTimer[WORKINDEX_TOTAL];
    
	uint8_t sensor[INPUT_8_NUMBER*8 + EXTIO_NUM*8];
	uint8_t outbuf[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];
	bool	debugmode;
	bool	predebugmode;

	bool 		UpdateEEPROM;

	uint16_t	inPosition;
	uint32_t	ServoPosition;
	uint32_t	ServoAcceleration;
	uint16_t	StepMotorSpeed[2];
	bool	 	run;
	bool        IndicationEmergency = false;
	uint8_t		ErrorCode;

	int 		RunMode = 0;
    int         SerarchDir;
    int         TargetStation;
    int         Station;
}RuntimeStatus;


#define TIME_SENSOR_CONFIRM			300
#define TIME_READ_RFID_FAIL			3000

void MainProcess_Timer();
void MainProcess_Task();
void Demo_ControlMode();

void MainProcess_Init();
void setOutput(uint8_t index, uint8_t hl);
uint8_t getInput(uint8_t index);
bool LightBoxMoveToStation();
bool LightBoxSearchSensor();
uint8_t getStationSensor();
void SearchSensor(int dir=-1, int toggletimes = 0);


#endif	//_MAIN_PROCESS_H_
