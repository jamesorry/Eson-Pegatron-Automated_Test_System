#include "Arduino.h"
#include "hmi.h"
#include <SoftwareSerial.h>
#include "hmi_command.h"
#include "timer.h"
#include "cppQueue.h"
#include "MainProcess.h"
#include "StepperMotor.h"

extern "C" {
	#include <string.h>
	#include <stdlib.h>
}
extern StepperMotor *Motor[];
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
#if 0//HMI_CMD_QUEUE_DEBUG
	if(qlen != cmdQueue->getCount())
	{
		qlen = cmdQueue->getCount();
		cmd_port->println("Q Cnt:" + String(qlen, DEC) + " is empty: " + cmdQueue->isEmpty());
		if(qlen > 100)
			cmdQueue->clean();
	}
#endif

	if((millis() - SendCmdTimeCnt) >= TIME_HMI_CMD_INTERVAL)
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
			if(((HardwareSerial *)hmicmd_port)->availableForWrite())
			{
				hmicmd_port->write(cmdRec.data, cmdRec.datalen);
#if HMI_CMD_DEBUG
			cmd_port->print("HMI_Command::SendCommandQ: ");
			for(int i=0; i<cmdRec.datalen; i++)
				cmd_port->print(String(cmdRec.data[i], HEX)+ ",");
			cmd_port->println();
#endif
				if(cmdRec.datatype == QUEUE_DATA_TYPE_RESPONSE)
					cmdRec.datalen = 0;
				else
				{
					if(cmdRec.retrycnt < HMI_CMD_RETRY_MAX)
						cmdRec.retrycnt ++;
					else
						cmdRec.datalen = 0;
				}
				
				SendCmdTimeCnt = millis();
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
//					if(((DataBuff[i+1] == 9) || (DataBuff[i+1] == 13))
//						&&(DataBuff[i+2] == HMI_CMD_DATA_INDICATION))
//					{
//						if((DataBuff[i+4] == RequestTagID) || (DataBuff[i+4] == ResponseTagID)
//							|| (DataBuff[i+5] == RequestTagID) || (DataBuff[i+5] == ResponseTagID)
//							|| (DataBuff[i+6] == RequestTagID) || (DataBuff[i+6] == ResponseTagID)
//							|| (DataBuff[i+7] == RequestTagID) || (DataBuff[i+7] == ResponseTagID)
//							)
//							match = false;
//					}

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


bool HMI_Command::Response_Ping()
{
	uint8_t i;
	HMICmdRec rec;
	rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
	rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
	rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
	rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_PING;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
	rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
	rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
	rec.retrycnt = 0;
	cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
	cmd_port->println("Response_Ping()");
#endif
	
	return true;
}

bool HMI_Command::Response_Set_Station()
{
    if(recdata[HMI_CMD_BYTE_DATA] < 0x03 && recdata[HMI_CMD_BYTE_DATA] >= 0x00){
        maindata.TargetStation = recdata[HMI_CMD_BYTE_DATA];
        cmd_port->println("Target Station: " + String(maindata.TargetStation));
        runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_SEARCH_SENSOR;
        runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0;
    }
    else {
        cmd_port->println("Wrong Station: " + String(recdata[HMI_CMD_BYTE_DATA]));
        return false;
    }
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_STATION;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_Set_Station()");
#endif
    runtimedata.UpdateEEPROM = true;
    return true;
}
bool HMI_Command::Response_ReadParameter()
{
    uint8_t motornum = recdata[HMI_CMD_BYTE_DATA];
    
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE+5;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_READ_PARAMETER;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[HMI_CMD_BYTE_DATA] = recdata[HMI_CMD_BYTE_DATA];
    cmd_port->println("Motor num: " + String(motornum));
    cmd_port->println("Frequence: " + String(Motor[motornum]->getFrequence()));
    cmd_port->println("AccelerateTime: " + String(Motor[motornum]->getAccelerateTime()));
	for(uint8_t i=1; i<3; i++)
		rec.data[HMI_CMD_BYTE_DATA+i] = (Motor[motornum]->getFrequence() >> (2-i)*8)& 0xff;
	for(uint8_t i=3; i<5; i++)
		rec.data[HMI_CMD_BYTE_DATA+i] = (Motor[motornum]->getAccelerateTime() >> (4-i)*8)& 0xff;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_ReadParameter()");
#endif
    
    return true;
}
bool HMI_Command::Response_WriteParameter()
{
    uint8_t motornum = recdata[HMI_CMD_BYTE_DATA];
    long freq = 0, acceleration = 0;
    
    for(uint8_t i=1; i<3; i++)
    {
        freq <<= 8;
        freq += recdata[HMI_CMD_BYTE_DATA+i];
    }
    
    for(uint8_t i=3; i<5; i++)
    {
        acceleration <<= 8;
        acceleration += recdata[HMI_CMD_BYTE_DATA+i];
    }
    cmd_port->println("Motor num: " + String(motornum));
    cmd_port->println("freq: " + String(freq));
    cmd_port->println("acceleration: " + String(acceleration));
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_WRITE_PARAMETER;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[HMI_CMD_BYTE_DATA] = 0x00; //success.
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_WriteParameter()");
#endif
    
    return true;
}

bool HMI_Command::Response_Save_Data()
{
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_SAVE_DATA;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_Save_Data()");
#endif
    runtimedata.UpdateEEPROM = true;
    return true;
}

bool HMI_Command::Response_Control_Board_Status()
{
    uint8_t motornum = recdata[HMI_CMD_BYTE_DATA];
    
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 7;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_CONTROL_BOARD_STATUS;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[HMI_CMD_BYTE_DATA] = motornum;
    switch(motornum){
        case MOTOR_SERVO:
            if(Motor[motornum]->getState() == MOTOR_STATE_STOP)
                rec.data[HMI_CMD_BYTE_DATA+1] = 0x00; //status Motor stop.
            else
                rec.data[HMI_CMD_BYTE_DATA+1] = 0x01; //status Motor is running.
            rec.data[HMI_CMD_BYTE_DATA+2] = runtimedata.Station;
            for(uint8_t i=3; i<6; i++)
                rec.data[HMI_CMD_BYTE_DATA+i] = (Motor[motornum]->getPosition() >> (5-i)*8)& 0xff;
            break;
        case MOTOR_VR:
            if(Motor[motornum]->getState() == MOTOR_STATE_STOP)
                rec.data[HMI_CMD_BYTE_DATA+1] = 0x00; //status Motor stop.
            else
                rec.data[HMI_CMD_BYTE_DATA+1] = 0x01; //status Motor is running.
            rec.data[HMI_CMD_BYTE_DATA+2] = 0x00;
            for(uint8_t i=3; i<6; i++)
                rec.data[HMI_CMD_BYTE_DATA+i] = (Motor[motornum]->getPosition() >> (5-i)*8)& 0xff;
            break;
    }
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_Control_Board_Status()");
#endif
    
    return true;
}

bool HMI_Command::Response_Restart()
{
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_RESTART;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_Restart()");
#endif
    
    return true;
}

bool HMI_Command::Response_IO_Status()
{
    int i;
    uint8_t hl = 0;
    uint8_t bytei = 0;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_IO_STATUS;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;

    if(recdata[HMI_CMD_BYTE_DATA] == 0) //DI
    {
        rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 + INPUT_8_NUMBER+EXTIO_NUM;
        rec.data[HMI_CMD_BYTE_DATA] = 0;    //0:Input
#if HMI_CMD_DEBUG
            cmd_port->println("Response_IO_Status()");
            cmd_port->print("Input: ");
#endif
        for(bytei=0; bytei<(INPUT_8_NUMBER + EXTIO_NUM); bytei++)
        {
            rec.data[HMI_CMD_BYTE_DATA+1+bytei] = 0;
            for(i=7; i>=0; i--)
            {
                hl = (getInput(bytei*8+i) & 0x01);
                rec.data[HMI_CMD_BYTE_DATA+1+bytei] |=  (hl << i);
			#if HMI_CMD_DEBUG
                    cmd_port->print(String(hl) + " ");
			#endif
            }
#if HMI_CMD_DEBUG
            cmd_port->println(": " + String(rec.data[HMI_CMD_BYTE_DATA+1+bytei], HEX));
#endif
        }
    }
    else
    {
        rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE + 1 + OUTPUT_8_NUMBER+EXTIO_NUM;
        rec.data[HMI_CMD_BYTE_DATA] = 1;    //1:Output
#if HMI_CMD_DEBUG
        cmd_port->print("Output: ");
#endif
        for(bytei=0; bytei<(OUTPUT_8_NUMBER + EXTIO_NUM); bytei++)
        {
            rec.data[HMI_CMD_BYTE_DATA+1+bytei] = 0;
            for(i=7; i>=0; i--)
            {
                rec.data[HMI_CMD_BYTE_DATA+1+bytei] |= digitalio.Output[bytei*8+i] << i;
                cmd_port->print(String(digitalio.Output[bytei*8+i]) + " ");
            }
#if HMI_CMD_DEBUG
            cmd_port->println(": " + String(rec.data[HMI_CMD_BYTE_DATA+1+bytei], HEX));
#endif
        }
    }
        
    
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
        
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);
    
    return true;
}

bool HMI_Command::Response_Set_DO_State()
{
    uint8_t i, bytei;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
#if HMI_CMD_DEBUG
    cmd_port->print("Set DO: ");
    cmd_port->print(recdata[HMI_CMD_BYTE_DATA]);
    cmd_port->print("-->");    
#endif
    if(recdata[HMI_CMD_BYTE_DATA + 1] > 0){
        cmd_port->println("HIGH");
        digitalWrite(OutputPin[recdata[HMI_CMD_BYTE_DATA]], HIGH);
        if(recdata[HMI_CMD_BYTE_DATA] >= 0 && recdata[HMI_CMD_BYTE_DATA] < 8){
            runtimedata.UpdateEEPROM = true;
            maindata.Output_Last_HighLow[recdata[HMI_CMD_BYTE_DATA]] = true;
        }
    }
    else{
        cmd_port->println("LOW");
        digitalWrite(OutputPin[recdata[HMI_CMD_BYTE_DATA]], LOW);
        if(recdata[HMI_CMD_BYTE_DATA] >= 0 && recdata[HMI_CMD_BYTE_DATA] < 8){
            runtimedata.UpdateEEPROM = true;
            maindata.Output_Last_HighLow[recdata[HMI_CMD_BYTE_DATA]] = false;
        }
    }

    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_SET_DO;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    cmdQueue->push(&rec);
    
    return true;
}

bool HMI_Command::Response_Motor_Move()
{
    uint8_t movetype = recdata[HMI_CMD_BYTE_DATA];
    uint8_t motornum = recdata[HMI_CMD_BYTE_DATA+1];
    long step = 0;
    
    for(uint8_t i=2; i<6; i++)
    {
        step <<= 8;
        step += recdata[HMI_CMD_BYTE_DATA+i];
    }
    cmd_port->println("Move type: " + String(movetype));
    cmd_port->println("Motor num: " + String(motornum));
    cmd_port->println("step: " + String(step));
    
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_MOTOR_MOVE;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_Motor_Move()");
#endif
    if(Motor[motornum]->getState() == MOTOR_STATE_STOP){
        if(movetype == 0x00)
            Motor[motornum]->Steps(step, Motor[motornum]->getFrequence());
        else if(movetype == 0x01)
            Motor[motornum]->MoveTo(step, Motor[motornum]->getFrequence());
    }
    return true;
}

bool HMI_Command::Indication_Data()
{
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_INDICATION;
    rec.data[HMI_CMD_BYTE_TAGID] = RequestTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_EMERGENCY_INDICATION;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Indication_Data()");
#endif
    return true;
}

bool HMI_Command::Indication_Emergency()
{
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_INDICATION;
    rec.data[HMI_CMD_BYTE_TAGID] = RequestTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_EMERGENCY_INDICATION;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Indication_Emergency()");
#endif
    return true;
}

bool HMI_Command::Response_VR_Home_Offset()
{
    uint8_t motornum = recdata[HMI_CMD_BYTE_DATA];
    long step = 0;
    
    for(uint8_t i=1; i<5; i++)
    {
        step <<= 8;
        step += recdata[HMI_CMD_BYTE_DATA+i];
    }
    cmd_port->println("Motor num: " + String(motornum));
    cmd_port->println("step: " + String(step));
    maindata.VR_HomeOffset = step;
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_VR_HOME_OFFSET;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_VR_Home_Offset()");
#endif
    if(motornum == MOTOR_VR){
        if(Motor[motornum]->getState() == MOTOR_STATE_STOP){
            runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_INIT;
            runtimedata.Workindex[WORKINDEX_VR_INITIAL] = 0;
        }
    }
    return true;
}

bool HMI_Command::Response_Motor_Search_Home()
{
    uint8_t motornum = recdata[HMI_CMD_BYTE_DATA];

    if(runtimedata.RunMode[MOTOR_SERVO] == RUN_MODE_SERVO_NORMAL
        && Motor[MOTOR_SERVO]->getState() == MOTOR_STATE_STOP)
    {
        if(motornum == MOTOR_SERVO)
        {
            runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_INIT;
            runtimedata.Workindex[WORKINDEX_SERVO_INITIAL] = 0;
            runtimedata.Workindex[WORKINDEX_GO_HOME] = 0;
        }
    }
    uint8_t i;
    HMICmdRec rec;
    rec.datatype = QUEUE_DATA_TYPE_RESPONSE;
    rec.data[HMI_CMD_BYTE_TAGID] = ResponseTagID;
    rec.data[HMI_CMD_BYTE_LENGTH] = HMI_CMD_LEN_BASE;
    rec.data[HMI_CMD_BYTE_CMDID] = HMI_CMD_MOTOR_SEARCH_HOME;
    rec.data[HMI_CMD_BYTE_HMIID] = maindata.HMI_ID;
    rec.data[rec.data[HMI_CMD_BYTE_LENGTH]-1] = HMI_CMD_ComputeCRC(rec.data);
    rec.datalen = rec.data[HMI_CMD_BYTE_LENGTH];
    rec.retrycnt = 0;
    cmdQueue->push(&rec);

#if HMI_CMD_DEBUG
    cmd_port->println("Response_Save_Data()");
#endif
    runtimedata.UpdateEEPROM = true;
    return true;
}


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
	if(reclen == recdata[HMI_CMD_BYTE_LENGTH] && (recdata[HMI_CMD_BYTE_HMIID] == maindata.HMI_ID))
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
		if((cmdRec.datalen > 0)
			&& (cmdRec.datatype == QUEUE_DATA_TYPE_INDICATION)
			&& (cmdRec.data[HMI_CMD_BYTE_CMDID] != recdata[HMI_CMD_BYTE_CMDID])
			)
		{
			if(cmdRec.retrycnt < HMI_CMD_RETRY_MAX)
			{
				cmdQueue->push(&cmdRec);
			}
			cmdRec.datalen = 0;
		}
		else
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
					issupportcmd = Response_Ping();
					break;
                case HMI_CMD_SET_STATION:
                    cmd_port->println("HMI_CMD_SET_STATION.");
                    issupportcmd = Response_Set_Station();
                    break;
                case HMI_CMD_READ_PARAMETER:
                    cmd_port->println("HMI_CMD_READ_PARAMETER.");
                    issupportcmd = Response_ReadParameter();
                    break;
                case HMI_CMD_WRITE_PARAMETER:
                    cmd_port->println("HMI_CMD_WRITE_PARAMETER.");
                    issupportcmd = Response_WriteParameter();
                    break;
                case HMI_CMD_SAVE_DATA:
                    cmd_port->println("HMI_CMD_SAVE_DATA.");
                    issupportcmd = Response_Save_Data();
                    break;
                case HMI_CMD_CONTROL_BOARD_STATUS:
                    cmd_port->println("HMI_CMD_CONTROL_BOARD_STATUS.");
                    issupportcmd = Response_Control_Board_Status();
                    break;
                case HMI_CMD_RESTART:
                    cmd_port->println("HMI_CMD_RESTART.");
                    issupportcmd = Response_Restart();
                    break;
                case HMI_CMD_IO_STATUS:
                    cmd_port->println("HMI_CMD_IO_STATUS.");
                    issupportcmd = Response_IO_Status();
                    break;
                case HMI_CMD_SET_DO:
                    cmd_port->println("HMI_CMD_SET_DO.");
                    issupportcmd = Response_Set_DO_State();
                    break;
                case HMI_CMD_MOTOR_MOVE:
                    cmd_port->println("HMI_CMD_MOTOR_MOVE.");
                    issupportcmd = Response_Motor_Move();
                    break;
                case HMI_CMD_EMERGENCY_INDICATION:
                    cmd_port->println("HMI_CMD_EMERGENCY_INDICATION.");
                    runtimedata.IndicationEmergency = false;
                    break;
                case HMI_CMD_VR_HOME_OFFSET:
                    cmd_port->println("HMI_CMD_VR_HOME_OFFSET.");
                    issupportcmd = Response_VR_Home_Offset();
                    break;
                case HMI_CMD_MOTOR_SEARCH_HOME:                    
                    cmd_port->println("HMI_CMD_MOTOR_SEARCH_HOME.");
                    issupportcmd = Response_Motor_Search_Home();
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
	    if(recdata[HMI_CMD_BYTE_HMIID] != maindata.HMI_ID)
			cmd_port->println("HMI_ID fail.");
		if(reclen != recdata[HMI_CMD_BYTE_LENGTH])
			cmd_port->println("Leng fail (" + String(reclen, DEC) + " != " + String(recdata[HMI_CMD_BYTE_LENGTH]) + String(")"));
#endif
		return -1;
	}
	
}

