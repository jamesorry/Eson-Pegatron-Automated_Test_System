#ifndef _HMI_CMD_BT_COMMAND_H_
#define _HMI_CMD_BT_COMMAND_H_

#include "Arduino.h"
#include "cppQueue.h"

#define HMI_CMD_DATA_MAX_LEN 128+1

typedef struct {
	uint8_t datatype;
	uint8_t retrycnt;
	uint8_t datalen;
	uint8_t data[32];
} HMICmdRec;

class HMI_Command
{
private:
	Queue	*cmdQueue;		//!< Queue implementation: FIFO LIFO
	HMICmdRec cmdRec;
	uint8_t RequestTagID;
	uint8_t ResponseTagID;

	uint8_t	ProcessIndex;
	uint16_t ProcessTimeCnt;
	
	HardwareSerial *hmicmd_port = NULL;
	
	uint8_t recdata[HMI_CMD_DATA_MAX_LEN];
	uint8_t reclen;
	uint8_t DataBuff[HMI_CMD_DATA_MAX_LEN];
	uint8_t DataBuffLen;
    uint32_t ReciveTime;
	bool Update;

	uint8_t SendIndication;
	uint16_t IndicationDataType;

	bool Connected;

	uint8_t ParameterResult;
	uint8_t NeedSendRFIDData;
	uint8_t NeedSendBarcodeData;

	uint16_t SendTCmdimeCnt = 0x0F00;
	uint16_t ResendTimeOutCnt = 0x0F00;

	bool skipHMI_ID;
	
public:
		bool MsgUpdate;
	HMI_Command(HardwareSerial* cmdport, const uint32_t baudrate=115200, const uint8_t reqid=0xFC, const uint8_t resid=0xFD, const bool skipid=false);
	~HMI_Command();
	
	void Process(void);
	void Recive_Message();
	void Response_Message();
	void Response_CRC_Fail(uint8_t crc);
    bool SplitRecvice(void);
	void Response_HMIID_Fail();
	bool Response_Ping(uint8_t *buff, uint8_t *reslen);
	bool Response_ParameterSetting(uint8_t *buff, uint8_t *reslen);
	bool Response_IO_Status(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_Servo_Position(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_Servo_Accel(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_StepMotor_Speed(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_HMI_ID(uint8_t *buff, uint8_t *reslen);
	bool Response_Run(uint8_t *buff, uint8_t *reslen);
	bool Indication_Data(uint8_t datatype);
	bool Response_Resetting(uint8_t *buff, uint8_t *reslen);
	bool Indication_Ping();
	bool Indication_Flatcar_in_position(uint8_t inposition);
	bool Response_Set_Platcar_direction(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_Platcar_to_go_station(uint8_t *buff, uint8_t *reslen);
	bool Response_Indication_Data(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_DO_State(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_Flatcar_Grip_State(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_Flatcar_go_to_position_state(uint8_t *buff, uint8_t *reslen);
	bool Response_Set_Layer_Of_Flatcar(uint8_t *buff, uint8_t *reslen);
	
	uint8_t CheckReciveData();
	void SendCommandQ();
	void Set_HMIID(uint8_t id);
	
	void CommandTest();
	void Timer();
};

#define HMI_CMD_LEN_BASE			5

#define QUEUE_DATA_TYPE_RESPONSE	0
#define QUEUE_DATA_TYPE_INDICATION	1

#define HMI_CMD_RETRY_MAX	10
#define TIME_RECIVE_DATA_DELAY		50
#define TIME_RECIVE_DATA_OVERDUE	1000


#define HMI_CMD_PING							0x00
#define HMI_CMD_PARAMETER_SETTING				0x01
#define HMI_CMD_IO_STATUS						0x02
#define HMI_CMD_SET_SERVO_POSITION				0x03
#define HMI_CMD_SET_SERVO_ACCEL					0x04
#define HMI_CMD_SET_STEP_MOTOR_SPEED			0x05
#define HMI_CMD_SET_HMI_ID						0x06
#define HMI_CMD_RUN								0x07
#define HMI_CMD_DATA_INDICATION					0x08
#define HMI_CMD_RESETTING						0x09
#define HMI_CMD_PING_INDICATION					0x0A
#define HMI_CMD_FLATCAR_IN_POSITION_INDICATION	0x0B
#define HMI_CMD_SET_FLATCAR_DIRECTION			0x0C
#define HMI_CMD_SET_FLATCAR_TO_GO_STATION		0x0D
#define HMI_CMD_SET_DO_STATE					0x0E
#define HMI_CMD_SET_FLATCAR_GRIP_STATE			0x0F
#define HMI_CMD_SET_FLATCAR_GO_TO_POSITION_STATE	0x10
#define HMI_CMD_SET_LAYER_OF_FLATCAR			0x11


#define HMI_CMD_BYTE_TAGID				0
#define HMI_CMD_BYTE_LENGTH				1
#define HMI_CMD_BYTE_CMDID				2
#define HMI_CMD_BYTE_HMIID				3
#define HMI_CMD_BYTE_DATA				4

#define HMI_CMD_DATA_INDICATION_RFID	0
#define HMI_CMD_DATA_INDICATION_ERROR	1

#define HMI_CMD_TIME_RESEND_INTERVAL	300
#define HMI_CMD_TIME_RESEND_TIMEOUT		10000
#define TIME_HMI_CMD_INTERVAL			300

#endif	//_HMI_CMD_BT_COMMAND_H_
