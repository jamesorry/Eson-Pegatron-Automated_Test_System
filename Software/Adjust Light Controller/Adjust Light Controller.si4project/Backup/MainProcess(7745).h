#ifndef _MAIN_PROCESS_H_
#define _MAIN_PROCESS_H_

#include "Arduino.h"

#define	VERSTR	"2021083001"
#define VENDOR	"LienHong"
//調光控制板*8

#define setbit(value,x) (value |=(1<<x))
#define getbit(value,x) ((value>>x)&1)
#define clrbit(value,x) (value &=~(1<<x))
#define revbit(value,x) (value ^=(1<<x))

#define	INPUT_NUMBER		4
#define OUTPUT_NUMBER		4

#define WORKINDEX_TOTAL 	4
#define BUZZ				9
#define LAN_CFG             8
#define out0     A0
#define out1     A1
#define out2     A2
#define out3     A3
#define in0      A4
#define in1      A5
#define in2      A6
#define in3      A7
static const uint8_t ADC_OutputPin[] = {A0, A1, A2, A3};
static const uint8_t ADC_InputPin[] = {A4, A5, A6, A7};

#define OUT01_ONOFF 0

#define LUX_10      0
#define LUX_20      1
#define LUX_50      2
#define LUX_80      3
#define LUX_150     4
#define LUX_320     5
#define LUX_450     6
#define LUX_500     7
#define LUX_800     8
#define LUX_1000    9

typedef struct _DigitalIO_
{
	uint8_t	Input;
	uint8_t	Output;
	uint8_t PreOutput;
}DigitalIO;
/*
ID 0,1 --> Lux_Table[0][]
ID 2,3 --> Lux_Table[1][]
ID 4,5 --> Lux_Table[2][]
ID 6,7 --> Lux_Table[3][]
*/

typedef struct _MainDataStruct_
{
//	此處的變數值會寫到EEPROM
	char   Vendor[10];
    uint8_t HMI_ID;
    uint8_t Voltage_Last;  // 上次的Voltage輸出值(0~100%)
    bool    Output_Light_Last = false;
}MainDataStruct;


typedef struct _RuntimeStruct_
{
//	此處為啟動後才會使用的變數
	int  	Workindex[WORKINDEX_TOTAL];
	int		preWorkindex[WORKINDEX_TOTAL];
	
	uint8_t sensor[INPUT_NUMBER];
	uint8_t outbuf[OUTPUT_NUMBER];

	bool 		UpdateEEPROM;
    uint8_t CheckReceive = -1;
}RuntimeStatus;

void CheckID();
void MainProcess_Task();
void MainProcess_Init();
void buzzerPlay(int playMS);
void SetVoltage(uint8_t value);

#endif	//_MAIN_PROCESS_H_

