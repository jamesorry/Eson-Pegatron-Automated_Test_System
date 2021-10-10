#ifndef _HMI_CMD_BT_COMMAND_H_
#define _HMI_CMD_BT_COMMAND_H_

#include "Arduino.h"
#include "cppQueue.h"

#define HMI_CMD_DATA_MAX_LEN 128 + 1

typedef struct {
  uint8_t datatype;
  uint8_t retrycnt;
  uint8_t datalen;
  uint8_t data[32];
} HMICmdRec;

class HMI_Command {
private:
  Queue *cmdQueue;  //!< Queue implementation: FIFO LIFO
  HMICmdRec cmdRec;
  uint8_t RequestTagID;
  uint8_t ResponseTagID;

  uint8_t ProcessIndex;
  uint16_t ProcessTimeCnt;

  HardwareSerial *hmicmd_port = NULL;

  uint8_t recdata[HMI_CMD_DATA_MAX_LEN];
  uint8_t reclen;
  uint8_t DataBuff[HMI_CMD_DATA_MAX_LEN];
  uint8_t DataBuffLen;
  uint32_t ReciveTime;
  bool Update;
  bool Connected;
  uint8_t ParameterResult;
  uint32_t SendCmdTimeCnt = 0;
  bool skipHMI_ID;

public:
  HMI_Command(HardwareSerial *cmdport, const uint32_t baudrate = 38400, const uint8_t reqid = 0xF1, const uint8_t resid = 0xF9, const bool skipid = false);
  ~HMI_Command();

  void Process(void);
  bool SplitRecvice(void);
  uint8_t CheckReciveData();
  void SendCommandQ();
  bool Response_Ping();
  bool Response_Set_Station();
  bool Response_ReadParameter();
  bool Response_WriteParameter();
  bool Response_Save_Data();
  bool Response_Control_Board_Status();
  bool Response_Restart();
  bool Response_IO_Status();
  bool Response_Set_DO_State();
  bool Response_Motor_Move();
  bool Indication_Data();
  bool Indication_Emergency();
  bool Response_VR_Home_Offset();
  bool Response_Motor_Search_Home();
  void Set_HMIID(uint8_t id);

  void CommandTest();
};

#define HMI_CMD_LEN_BASE 5

#define QUEUE_DATA_TYPE_RESPONSE 0
#define QUEUE_DATA_TYPE_INDICATION 1

#define HMI_CMD_RETRY_MAX 0
#define TIME_RECIVE_DATA_DELAY 50
#define TIME_RECIVE_DATA_OVERDUE 1000


#define HMI_CMD_PING                    0x00
#define HMI_CMD_SET_STATION             0x01
#define HMI_CMD_READ_PARAMETER          0x02
#define HMI_CMD_WRITE_PARAMETER         0x03
#define HMI_CMD_SAVE_DATA               0x04
#define HMI_CMD_CONTROL_BOARD_STATUS    0x05
#define HMI_CMD_RESTART                 0x06
#define HMI_CMD_IO_STATUS               0x07
#define HMI_CMD_SET_DO                  0x08
#define HMI_CMD_MOTOR_MOVE              0x09
#define HMI_CMD_EMERGENCY_INDICATION    0x0A
#define HMI_CMD_VR_HOME_OFFSET          0x0D
#define HMI_CMD_MOTOR_SEARCH_HOME       0x0E


#define HMI_CMD_BYTE_TAGID 0
#define HMI_CMD_BYTE_LENGTH 1
#define HMI_CMD_BYTE_CMDID 2
#define HMI_CMD_BYTE_HMIID 3
#define HMI_CMD_BYTE_DATA 4

#define TIME_HMI_CMD_INTERVAL 300

#endif  //_HMI_CMD_BT_COMMAND_H_
