#ifndef _MAIN_PROCESS_H_
#define _MAIN_PROCESS_H_

#include "Arduino.h"
#include "hmi.h"

#define MAIN_PROCESS_DEBUG              1

#define MOTOR_SERVO			            0
#define MOTOR_VR			            1
#define MOTOR_TOTAL			            2

#define SPEED_SERVO_SEARCH              4000
#define SPEED_SERVO_GOHOME              3000
#define TIME_FLATCAR_ACCELERATE		    300

#define	EXTIO_NUM		                0
#define	INPUT_8_NUMBER	                2
#define OUTPUT_8_NUMBER	                1

#define	OUTPUT_NONE_ACTIVE	            0
#define	OUTPUT_ACTIVE		            1

#define	INPUT_NONE_ACTIVE	            0
#define	INPUT_ACTIVE		            1

#define IN00_EmergencyPin               0
#define IN01_FrontLimitPin              1
#define IN02_Pos_1_Pin                  2
#define IN03_Pos_2_Pin                  3
#define IN04_Pos_3_Pin                  4
#define IN05_BackLimitPin               5
#define IN06_VR_FrontLimitPin           6
#define IN07_VR_BackLimitPin            7


#define STATION_UNKNOW					-1
#define STATION_TOTAL                   3

#define RUN_MODE_EMERGENCY		        -1
#define RUN_MODE_SERVO_STOP				0
#define RUN_MODE_SERVO_INIT				1
#define RUN_MODE_SERVO_NORMAL			2
#define RUN_MODE_SERVO_SEARCH_SENSOR	3

#define RUN_MODE_VR_STOP			    0
#define RUN_MODE_VR_INIT				1
#define RUN_MODE_VR_IO_INIT				2
#define RUN_MODE_VR_NORMAL				3

#define WORKINDEX_SERVO_INITIAL			0
#define WORKINDEX_VR_INITIAL			1
#define WORKINDEX_GO_HOME				2
#define WORKINDEX_SEARCH_SENSOR			3
#define WORKINDEX_VR_GO_HOME		    4
#define WORKINDEX_TOTAL					8

typedef struct _DigitalIO_
{
	uint8_t	Input[4];
	uint8_t	Output[4];
	uint8_t PreOutput[4];
}DigitalIO;

typedef struct _MainDataStruct_
{
	char        Vendor[10];
	uint8_t 	HMI_ID;
	long		MotorSpeed[2];
	long		MotorFrequenceStart[2];
	long		MotorAccelerateTime[2];
    long        StationPosition[3];
    uint16_t    OffsetDistanceOfStopPin;
    int         TargetStation;
    bool        Output_Last_HighLow[8];
    uint32_t    CheckVersion;
    long        VR_HomeOffset;
}MainDataStruct;


typedef struct _RuntimeStruct_
{
	int  	Workindex[WORKINDEX_TOTAL];
	int		preWorkindex[WORKINDEX_TOTAL];
	int     RunMode[MOTOR_TOTAL];
    int     preRunMode[MOTOR_TOTAL];
	uint8_t sensor[INPUT_8_NUMBER*8 + EXTIO_NUM*8];
	uint8_t outbuf[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];

	bool 		UpdateEEPROM;
	bool        IndicationEmergency = false;
	uint8_t		ErrorCode;
    int         SerarchPin;
    int         SerarchDir;
    int         Station;    
    uint8_t     ReadInput = 0x00;
    uint8_t     PositionInput = 0x00;
    bool        NeedRestart = false;
}RuntimeStatus;

void MainProcess_Timer();
void MainProcess_Task();
void MainProcess_Init();
void setOutput(uint8_t index, uint8_t hl);
uint8_t getInput(uint8_t index);
void MotorVRStatus();
void MotorServoStatus();
bool MotorServoInit();
bool MotorVRInit();
bool MotorGoHome(int motornum);
bool VRGoHome(int motornum);
void MotorServoSearchSensor(int pin, int dir=-1, int toggletimes = 0);
void MotorServoSearchSensor(int dir=-1, int toggletimes = 0);
void MotorVRSearchSensor(int dir=-1, int toggletimes = 0);
void goHome(int motornum);
uint8_t getStationSensor();
bool LightBoxSearchSensor();
void ReadPositionSensor();
void buzzerPlay(int ms);


#endif	//_MAIN_PROCESS_H_

