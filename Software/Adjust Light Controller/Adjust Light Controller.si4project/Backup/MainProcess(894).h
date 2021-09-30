#ifndef _MAIN_PROCESS_H_
#define _MAIN_PROCESS_H_

#include "Arduino.h"

#define	VERSTR	"2021062201"

#define VENDOR	"LienHong"

#define	EXTIO_NUM			0
#define	INPUT_8_NUMBER		0
#define OUTPUT_8_NUMBER		0

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
typedef struct _DigitalIO_
{
	uint8_t	Input[4];
	uint8_t	Output[4];
	uint8_t PreOutput[4];
}DigitalIO;

typedef struct _MainDataStruct_
{
//	此處的變數值會寫到EEPROM
	char   Vendor[10];
    uint16_t ADC_Last;  // 上次的輸出值
    float ADC_ave;      // 每 g 所需的 ADC 值

    uint8_t HMI_ID;
}MainDataStruct;


typedef struct _RuntimeStruct_
{
//	此處為啟動後才會使用的變數
	int  	Workindex[WORKINDEX_TOTAL];
	int		preWorkindex[WORKINDEX_TOTAL];
	
	uint8_t sensor[INPUT_8_NUMBER*8 + EXTIO_NUM*8];
	uint8_t outbuf[(OUTPUT_8_NUMBER+EXTIO_NUM)*8];

	bool 		UpdateEEPROM;
    uint8_t CheckReceive = -1;
}RuntimeStatus;


void MainProcess_Task();
void MainProcess_Init();
bool OHM_TARE(uint8_t num);
bool OHM_SCALE(uint8_t num);
void buzzerPlay(int playMS);
void Read_OHM(int    kk);
void draw_logo(void);
void draw(void);


#endif	//_MAIN_PROCESS_H_

