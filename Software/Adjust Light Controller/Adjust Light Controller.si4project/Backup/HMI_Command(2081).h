#ifndef _HMI_CMD_BT_COMMAND_H_
#define _HMI_CMD_BT_COMMAND_H_

#include "Arduino.h"
#include "cppQueue.h"

#define HMI_CMD_DATA_MAX_LEN 128+1

#define LCD_SHOW_DEBUG_MSG	0
#define LCD_SHOW_DEBUG_MSG_LINE	7

#define TIME_RECIVE_DATA_DELAY		50
#define TIME_RECIVE_DATA_OVERDUE	1000

typedef struct {
	uint8_t datatype;
	uint8_t retrycnt;
	uint8_t datalen;
	uint8_t data[40];

	uint32_t last_ms;
} HMICmdRec;



class HMI_Command
{
private:
	Queue	*cmdQueue;		//!< Queue implementation: FIFO LIFO
	HMICmdRec cmdRec;

	uint8_t	ProcessIndex;
	uint16_t ProcessTimeCnt;
	
	Stream *hmicmd_port = NULL;

	bool SWSerial = false;
	bool Update;

	uint32_t SendCmdTimeCnt = 0;
	uint32_t	ReciveTime;

	bool skipHMI_ID;
	uint8_t recdata[HMI_CMD_DATA_MAX_LEN];
	uint8_t reclen;
	uint8_t DataBuff[HMI_CMD_DATA_MAX_LEN];
	uint8_t DataBuffLen;

public:
	
		uint8_t 	ControllerTagID;
		uint8_t 	ResponseTagID;
		uint8_t 	Receive_HMI_CMD;
		bool 		MsgUpdate;
	HMI_Command(HardwareSerial* cmdport, const uint32_t baudrate=115200, const uint8_t reqid=0xF1, const uint8_t resid=0xF9, const bool skipid=false);
	~HMI_Command();
	
	void Process(void);
	void Recive_Message();
	void Response_Message();
	void Response_CRC_Fail(uint8_t crc);

	bool Response_Ping();
	bool Request_Ping();
    bool Response_OHM_Reader(uint32_t ohm_A, uint32_t ohm_B, uint32_t ohm_C, uint32_t ohm_D);
    bool Response_OHM_Scale();
    bool Response_OHM_Tare();
    bool Response_HMIID();

    bool Response_ScaleADC(uint32_t ScaleADC_A, uint32_t ScaleADC_B, uint32_t ScaleADC_C, uint32_t ScaleADC_D);
    bool Response_TareADC(uint32_t TareADC_A, uint32_t TareADC_B, uint32_t TareADC_C, uint32_t TareADC_D);
	

	bool Send_Command(uint8_t cmdid, uint8_t tagid, uint8_t hmiid);
	bool Send_Command(uint8_t cmdid, uint8_t tagid, uint8_t hmiid, uint8_t data);

	bool SplitRecvice();
	uint8_t CheckReciveData();
	void SendCommandQ();
	void Set_HMIID(uint8_t id);
	void ReciveDataTest(uint8_t *data, uint8_t datalen);
	
	void CommandTest();
};

#define HMI_CMD_LEN_BASE			5

#define QUEUE_DATA_TYPE_RESPONSE	0
#define QUEUE_DATA_TYPE_INDICATION	1

#define HMI_CMD_RETRY_MAX	        0


#define HMI_CMD_PING				    0x00
#define HMI_CMD_RECORD_OHM_RESELT       0x01
#define HMI_CMD_OHM_TARE                0x02
#define HMI_CMD_OHM_SCALE               0x03
#define HMI_CMD_SETID                   0x04
#define HMI_CMD_TARE_ADC                0x05
#define HMI_CMD_SCALE_ADC               0x06



#define HMI_CMD_DATA_INDICATION			0x08

#define HMI_CMD_BYTE_TAGID				0
#define HMI_CMD_BYTE_LENGTH				1
#define HMI_CMD_BYTE_CMDID				2
#define HMI_CMD_BYTE_HMIID				3
#define HMI_CMD_BYTE_DATA				4


#define HMI_CMD_TIME_RESEND_TIMEOUT		10000
#define TIME_HMI_CMD_INTERVAL			400

#define ERROR_CODE_NONE					0x0000
#define ERROR_CODE_EMO					0xE001
#define ERROR_CODE_RFID					0xE002

uint8_t HMI_CMD_ComputeCRC(uint8_t *buff);

#endif	//_HMI_CMD_BT_COMMAND_H_
