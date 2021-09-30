#include "Arduino.h"
#include "hmi.h"
#include <SoftwareSerial.h>
#include "hmi_command.h"
#include "timer.h"
#include "cppQueue.h"
#include "MainProcess.h"

extern "C" {
	#include <string.h>
	#include <stdlib.h>
}

extern MainDataStruct maindata;
extern DigitalIO digitalio;
extern RuntimeStatus runtimedata;

extern HardwareSerial *cmd_port;

#define HMI_CMD_DEBUG		1
#define HMI_CMD_QUEUE_DEBUG	1

bool IsDataSend=false;
uint8_t resopnsebuf[32], resopnselen = 0;

HMI_Command::HMI_Command(HardwareSerial* port, const uint32_t baudrate, const uint8_t reqid, const uint8_t resid, const bool skipid)
{
	hmicmd_port = port;
	hmicmd_port->begin(baudrate);

	RequestTagID = reqid;
	ResponseTagID = resid;
	skipHMI_ID = skipid;

#if HMI_CMD_DEBUG	
	cmd_port->println("HMI_Command()." );
#endif
	Update = false;
	reclen = 0;

	cmdQueue = new Queue(sizeof(HMICmdRec), 20, FIFO);
#if HMI_CMD_DEBUG	
		cmd_port->println("init Q cnt: " + String(cmdQueue->getCount()));
#endif
	cmdRec.datalen = 0;

	ProcessIndex = 0;
	ProcessTimeCnt = 0;
	ParameterResult = 0;	//Success;
}


void HMI_Command::Set_HMIID(uint8_t id)
{
	maindata.HMI_ID = id;
}

int qlen = 0xff;
void HMI_Command::SendCommandQ(void)
{
#if 0
	if(qlen != cmdQueue->getCount())
	{
		qlen = cmdQueue->getCount();
		cmd_port->println("Q Cnt:" + String(qlen, DEC) + " is empty: " + cmdQueue->isEmpty());
		if(qlen > 100)
			cmdQueue->clean();
	}
#endif
	if(SendTCmdimeCnt >= TIME_HMI_CMD_INTERVAL)
	{
		if(cmdQueue->isEmpty() == false)
		{
			if(!cmdQueue->pop(&cmdRec))
			{
				cmdRec.datalen = 0;
			}
		}
		if(cmdRec.datalen > 0)
		{
			if(hmicmd_port->availableForWrite())
			{
				hmicmd_port->write(cmdRec.data, cmdRec.datalen);
				if(cmdRec.datatype == QUEUE_DATA_TYPE_RESPONSE)
					cmdRec.datalen = 0;
				else
				{
					if(cmdRec.retrycnt < HMI_CMD_RETRY_MAX)
						cmdRec.retrycnt ++;
					else
						cmdRec.datalen = 0;
				}
				
				SendTCmdimeCnt = 0;
			}
		}
	}
}

void HMI_Command::Process(void)
{
	static uint8_t preProcessIndex = 0xff;
	int ret,i,inputCount=0;
	int retry=0;
#if HMI_CMD_DEBUG
	if(preProcessIndex != ProcessIndex)
	{
		preProcessIndex = ProcessIndex;
		cmd_port->println("ProcessIndex: " + String(ProcessIndex, DEC));
	}
#endif
	switch(ProcessIndex)
	{
		case 0:
		{
			if(hmicmd_port->available())
			{
#if HMI_CMD_DEBUG	
                cmd_port->println("hmicmd_port->available()");
#endif
                ProcessTimeCnt = millis();
                cmd_port->println("ProcessTimeCnt: " + String(ProcessTimeCnt));
                DataBuffLen = 0;
                reclen = 0;
                ProcessIndex ++;
			}
			break;
		}
		case 1:
		{
            if((millis() - ProcessTimeCnt) > TIME_RECIVE_DATA_DELAY)
            {
#if HMI_CMD_DEBUG	
                cmd_port->println("delay " + String(TIME_RECIVE_DATA_DELAY) +" ms");
#endif
                ProcessIndex ++;
            }
			break;
		}
		case 2:
		{
			while(hmicmd_port->available() && (DataBuffLen < HMI_CMD_DATA_MAX_LEN))
			{
				ret = hmicmd_port->read();
				DataBuff[DataBuffLen++] = (char)ret;
//				recdata[reclen++] = (char)ret;
			}
			Update = true;
#if HMI_CMD_DEBUG	
            cmd_port->print("HMI CMD recive (" + String(DataBuffLen) + String("): ") );
//              for(i=0; i<reclen; i++)
//                  cmd_port->print(String(recdata[i], HEX)+ String(","));
            for(i=0; i<DataBuffLen; i++)
                cmd_port->print(String(DataBuff[i], HEX)+ String(","));
            cmd_port->println();
#endif
            ReciveTime = millis();
            ProcessIndex = 0;
			break;
		}
	}
	while(SplitRecvice())
		CheckReciveData();
	SendCommandQ();
}

bool HMI_Command::SplitRecvice(void)
{
	int i, starti=-1, endi=-1;
	bool result = false;
	if(DataBuffLen >= 129)	
		DataBuffLen = 0;
	if(DataBuffLen >= HMI_CMD_LEN_BASE)
	{
		//for(i=0; i<DataBuffLen-2; i++)
		for(i=0; i<=DataBuffLen-HMI_CMD_LEN_BASE; i++)
			if((DataBuff[i] == RequestTagID) || (DataBuff[i] == ResponseTagID) || skipHMI_ID)
			{
				if(((DataBuff[i+1] < 32) && (DataBuff[i+1] >= HMI_CMD_LEN_BASE))
					&& (DataBuff[i+2] < 0x20)	//CMD ID < 0x20
					&& (DataBuff[i+3] < 0x50)	//HMI ID < 0x50
					)
				{
					bool match = true;
					if(((DataBuff[i+1] == 9) || (DataBuff[i+1] == 13))
						&&(DataBuff[i+2] == HMI_CMD_DATA_INDICATION))
					{
						if((DataBuff[i+4] == RequestTagID) || (DataBuff[i+4] == ResponseTagID)
							|| (DataBuff[i+5] == RequestTagID) || (DataBuff[i+5] == ResponseTagID)
							|| (DataBuff[i+6] == RequestTagID) || (DataBuff[i+6] == ResponseTagID)
							|| (DataBuff[i+7] == RequestTagID) || (DataBuff[i+7] == ResponseTagID)
							)
							match = false;
					}

					if(match)
					{
						starti = i;
						break;
					}
				}
			}
		if(starti > -1)
		{
			endi = DataBuff[HMI_CMD_BYTE_LENGTH+starti] + starti;
			if(DataBuffLen >= endi)
			{
#if HMI_CMD_DEBUG	
					cmd_port->println("SplitRecvice Datlen: " + String(DataBuffLen) +", Starti: " + String(starti));
					cmd_port->println("Len: " + String(DataBuff[HMI_CMD_BYTE_LENGTH]) + ", Endi: " + String(endi));
#endif
				memcpy(recdata, &DataBuff[starti], DataBuff[HMI_CMD_BYTE_LENGTH+starti]);
				reclen = DataBuff[HMI_CMD_BYTE_LENGTH+starti];
#if HMI_CMD_DEBUG	
                    cmd_port->println("reclen: " + String(reclen));
					cmd_port->println("SplitRecvice: ");
					for(i=0; i<reclen; i++)
						cmd_port->print(String(recdata[i], HEX) + " ");
					cmd_port->println();
                    cmd_port->println("DataBuffLen-(reclen+starti): " + String(DataBuffLen-(reclen+starti)));
#endif
				for(i=0; i<DataBuffLen-(reclen+starti); i++)
					DataBuff[i] = DataBuff[endi+i];
				DataBuffLen -= (reclen+starti);
				result = true;
#if HMI_CMD_DEBUG	
					cmd_port->println("SplitRecvice result: " + String(result));
					cmd_port->print("HIM Data Buff (" + String(DataBuffLen) + String("): ") );
					for(i=0; i<DataBuffLen; i++)
						cmd_port->print(String(DataBuff[i], HEX)+ String(","));
					cmd_port->println();
#endif
			}
		}
	}

	if(DataBuffLen > 0)
		if(!result && ((millis() - ReciveTime) > TIME_RECIVE_DATA_OVERDUE))
		{
#if HMI_CMD_DEBUG	
				cmd_port->println("millis: " + String(millis()) + ", ReciveTime: " + String(ReciveTime));
				cmd_port->println("Clear DataBuff(" + String(DataBuffLen) + ")");
#endif
			DataBuffLen = 0;
		}
	return result;
}


uint8_t HMI_CMD_ComputeCRC(uint8_t *buff)
{
	uint8_t cmp_crc = 0x00, i;
	for(i=0; i<buff[HMI_CMD_BYTE_LENGTH]-1; i++)
		cmp_crc -= buff[i];
	return (uint8_t)(cmp_crc & 0xFF);
}


bool HMI_Command::Response_Ping(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_PING;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
	cmd_port->println("Response_Ping()");
#endif
	
	return true;
}


#if 0
bool HMI_Command::Response_ParameterSetting(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;

	//
	maindata.Mode = recdata[HMI_CMD_BYTE_DATA];
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_PARAMETER_SETTING;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[HMI_CMD_BYTE_DATA] = 0;	//0:Success
	
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	
	return true;

}



bool HMI_Command::Response_IO_Status(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_IO_STATUS;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

	if(recdata[HMI_CMD_BYTE_DATA] == 0)	//DI
	{
		buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 + INPUT_8_NUMBER+EXTIO_NUM;
		buff[HMI_CMD_BYTE_DATA] = 0;	//0:Input
#if HMI_CMD_DEBUG
			cmd_port->print("Input: ");
#endif
		for(i=0; i<INPUT_8_NUMBER+EXTIO_NUM; i++)
		{
			buff[HMI_CMD_BYTE_DATA+1+i] = digitalio.Input[i];
#if HMI_CMD_DEBUG
			cmd_port->print(String(digitalio.Input[i], HEX) + " ");
#endif
		}
#if HMI_CMD_DEBUG
		cmd_port->println();
#endif
	}
	else
	{
		buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 + OUTPUT_8_NUMBER+EXTIO_NUM;
		buff[HMI_CMD_BYTE_DATA] = 1;	//1:Output
#if HMI_CMD_DEBUG
		cmd_port->print("Output: ");
#endif
		for(i=0; i<OUTPUT_8_NUMBER+EXTIO_NUM; i++)
		{
			buff[HMI_CMD_BYTE_DATA+1+i] = digitalio.Output[i];
#if HMI_CMD_DEBUG
			cmd_port->print(String(digitalio.Output[i], HEX) + " ");
#endif
		}
#if HMI_CMD_DEBUG
		cmd_port->println();
#endif
	}
		
	
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	
	return true;

}

bool HMI_Command::Response_Set_Servo_Position(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;

	runtimedata.ServoPosition = 0;
	for(i=0; i<4; i++)
	{
		runtimedata.ServoPosition <<= 8; 
		runtimedata.ServoPosition = recdata[HMI_CMD_BYTE_DATA + i];
	}
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 ;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_SERVO_POSITION;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

	buff[HMI_CMD_BYTE_DATA] = 0;	//0:Pass, 1:Fail
		
	
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Response_Set_Servo_Accel(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;

	runtimedata.ServoAcceleration = 0;
	for(i=0; i<4; i++)
	{
		runtimedata.ServoAcceleration <<= 8; 
		runtimedata.ServoAcceleration = recdata[HMI_CMD_BYTE_DATA + i];
	}
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 ;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_SERVO_ACCEL;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

	buff[HMI_CMD_BYTE_DATA] = 0;	//0:Pass, 1:Fail
		
	
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Response_Set_StepMotor_Speed(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;

	if(recdata[HMI_CMD_BYTE_DATA] < 2)
	{
		runtimedata.StepMotorSpeed[recdata[HMI_CMD_BYTE_DATA]] = 0; 
		for(i=0; i<2; i++)
		{
			runtimedata.StepMotorSpeed[recdata[HMI_CMD_BYTE_DATA]] <<= 8; 
			runtimedata.StepMotorSpeed[recdata[HMI_CMD_BYTE_DATA]] = recdata[HMI_CMD_BYTE_DATA + 1 + i];
		}
		buff[HMI_CMD_BYTE_DATA] = 0;	//0:Pass, 1:Fail
	}
	else 
		buff[HMI_CMD_BYTE_DATA] = 1;	//0:Pass, 1:Fail
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 ;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_STEP_MOTOR_SPEED;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

		
	
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	
	return true;
}


bool HMI_Command::Response_Set_HMI_ID(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	Set_HMIID(recdata[HMI_CMD_BYTE_HMIID]);
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_HMI_ID;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	


	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Response_Run(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;	
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_RUN;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];

	runtimedata.run = 1;
	
	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Indication_Data(uint8_t datatype)
{
	uint8_t i;
	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_INDICATION;
	rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_DATA_INDICATION;
	rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

	if(datatype == HMI_CMD_DATA_INDICATION_RFID)
	{
//		rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + rfiddata.Len;
//		for(i=0; i<rfiddata.Len; i++)
//			rec.data[HMI_CMD_BYTE_DATA+i] = rfiddata.Data[i];
	}
	else
	{
		rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 2;
		rec.data[HMI_CMD_BYTE_DATA] = (runtimedata.ErrorCode >> 8) & 0xFF;
		rec.data[HMI_CMD_BYTE_DATA+1] = runtimedata.ErrorCode & 0xFF;
	}
		
	
	rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
	
	rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	return true;
}

bool HMI_Command::Response_Indication_Data(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;	
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_DATA_INDICATION;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	cmdQueue->push(&rec);
	
	return true;
}


bool HMI_Command::Response_Resetting(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;	
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_RESETTING;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];

	runtimedata.debugmode = false;
	
	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	cmdQueue->push(&rec);
	
	return true;
}


bool HMI_Command::Indication_Ping()
{

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_INDICATION;
	rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
	rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_PING_INDICATION;
	rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
	rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Indication_Flatcar_in_position(uint8_t inposition)
{
	uint8_t i;
	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_INDICATION;
	rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_FLATCAR_IN_POSITION_INDICATION;
	rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

	rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1;
	rec.data[HMI_CMD_BYTE_DATA] = inposition;
	
	rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
	
	rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	return true;
}

bool HMI_Command::Response_Set_Platcar_direction(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;

	runtimedata.Flatcar_dir = recdata[HMI_CMD_BYTE_DATA];
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 ;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_FLATCAR_DIRECTION;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Response_Set_Platcar_to_go_station(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;

	runtimedata.Flatcar_to_go_station = recdata[HMI_CMD_BYTE_DATA];
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 ;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_FLATCAR_TO_GO_STATION;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	

	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Response_Set_DO_State(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	uint8_t datalen = recdata[HMI_CMD_BYTE_LENGTH] - HMI_CMD_LEN_BASE;
#if HMI_CMD_DEBUG
	cmd_port->println("Set DO: ");
#endif
	for(i=0; i<datalen; i++)
	{
#if HMI_CMD_DEBUG
		cmd_port->println(String(recdata[HMI_CMD_BYTE_DATA + i]) + " ");
#endif
		digitalio.Output[i] = recdata[HMI_CMD_BYTE_DATA + i];
		digitalio.PreOutput[i] = ~digitalio.Output[i];
	}
#if HMI_CMD_DEBUG
		cmd_port->println();
#endif
	
	runtimedata.debugmode = true;
	
	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_DO_STATE;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	


	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Response_Set_Flatcar_Grip_State(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	runtimedata.Flatcar_Grip_State = recdata[HMI_CMD_BYTE_DATA ];

	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_FLATCAR_GRIP_STATE;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	


	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Response_Set_Flatcar_go_to_position_state(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	runtimedata.Flatcar_to_go_station = recdata[HMI_CMD_BYTE_DATA];

	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_FLATCAR_GO_TO_POSITION_STATE;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	


	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	cmdQueue->push(&rec);
	
	return true;
}

bool HMI_Command::Response_Set_Layer_Of_Flatcar(uint8_t *buff, uint8_t *reslen)
{
	uint8_t i;
	runtimedata.Flatcar_layer = recdata[HMI_CMD_BYTE_DATA];

	buff[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	buff[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
	buff[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_LAYER_OF_FLATCAR;
	buff[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	buff[buff[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(buff);
	*reslen = buff[HMI_CMD_BYTE_LENGTH];	


	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	memcpy(rec.data, buff, buff[HMI_CMD_BYTE_LENGTH]);
	rec.datalen = buff[HMI_CMD_BYTE_LENGTH];
	cmdQueue->push(&rec);
	
	return true;
}



void HMI_Command::CommandTest()
{
	//for(int i=0; i<10; i++)
		Response_Ping(resopnsebuf, &resopnselen);
}
#endif


uint8_t HMI_Command::CheckReciveData()
{
	uint8_t rec_crc, cmp_crc = 0x00, i;
	bool issupportcmd = false;
	if(reclen < HMI_CMD_LEN_BASE)
	{
#if 0//HMI_CMD_DEBUG	
		cmd_port->println("CheckReciveData reclen: " + String(reclen) + " Exit.");
#endif
		return -1;
	}
#if HMI_CMD_DEBUG	
		cmd_port->println("CheckReciveData reclen: " + String(reclen));
#endif
		//&& (recdata[HMI_CMD_BYTE_HMIID] == maindata.HMI_ID)
	if(reclen == recdata[HMI_CMD_BYTE_LENGTH])
	{
#if HMI_CMD_DEBUG	
		cmd_port->println("CheckReciveData reclen: " + String(reclen));
#endif

		rec_crc = recdata[recdata[HMI_CMD_BYTE_LENGTH] - 1];
		//for(i=0; i<reclen-1; i++)
		//	cmp_crc -= recdata[i];
		cmp_crc = HMI_CMD_ComputeCRC(recdata);
		
		if(cmp_crc != rec_crc)
		{
#if HMI_CMD_DEBUG	
			cmd_port->println("CRC Fail " + String(cmp_crc, HEX));
#endif
			return -1;
		}

#if HMI_CMD_DEBUG	
	cmd_port->println("cmdRec.datatype: " + String(cmdRec.datatype, HEX) + ", cmd: " + String(recdata[HMI_CMD_BYTE_CMDID], HEX));
#endif
//		if((cmdRec.datalen > 0)
//			&& (cmdRec.datatype == QUEUE_DATA_TYPE_INDICATION)
//			&& (cmdRec.data[HMI_CMD_BYTE_CMDID] != recdata[HMI_CMD_BYTE_CMDID])
//			)
//		{
//			if(cmdRec.retrycnt < HMI_CMD_RETRY_MAX)
//			{
//				cmdRec.last_ms = millis();
//				cmdQueue->push(&cmdRec);
//			}
//			cmdRec.datalen = 0;
//		}
//		else
		{
			if(cmdRec.datalen > 0)
			{
				if(cmdRec.retrycnt < HMI_CMD_RETRY_MAX)
				{
					cmdQueue->push(&cmdRec);
				}
				cmdRec.datalen = 0;
			}
#if HMI_CMD_DEBUG	
	cmd_port->println("HMI_CMD: " + String(recdata[HMI_CMD_BYTE_CMDID], HEX));
#endif
			
			resopnsebuf[HMI_CMD_BYTE_TAGID] = ResponseTagID;

			switch(recdata[HMI_CMD_BYTE_CMDID])
			{
				case HMI_CMD_PING:
					cmd_port->println("HMI_CMD_PING.");
					issupportcmd = true;
					break;
			}
			if(issupportcmd)
			{
				cmdRec.datalen = 0;
			}	
		}
	}
	else
	{
#if HMI_CMD_DEBUG
		if(reclen != recdata[HMI_CMD_BYTE_LENGTH])
			cmd_port->println("Leng fail (" + String(reclen, DEC) + " != " + String(recdata[HMI_CMD_BYTE_LENGTH]) + String(")"));
#endif
		return -1;
	}
	
}



void HMI_Command::Timer()
{
	if(SendTCmdimeCnt < 0xFF00)
		SendTCmdimeCnt += TIMER_INTERVAL_MS;
	
	if(ResendTimeOutCnt < 0xFF00)
		ResendTimeOutCnt += TIMER_INTERVAL_MS;
	
	if(ProcessTimeCnt < 0xFF00)
		ProcessTimeCnt += TIMER_INTERVAL_MS;
}

