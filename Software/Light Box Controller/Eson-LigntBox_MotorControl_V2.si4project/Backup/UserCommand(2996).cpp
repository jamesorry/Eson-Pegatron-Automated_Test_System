#include "UserCommand.h"
#include "HMI.h"
#include <avr/wdt.h>
#include "SoftwareSerial.h"
#include "hmi_command.h"
#include "MainProcess.h"
#include "Timer.h"
#include "StepperMotor.h"
#include "EEPROM_Function.h"

#define USER_COMMAND_DEBUG	1

extern HardwareSerial *cmd_port;
extern RuntimeStatus	runtimedata;
extern MainDataStruct maindata;
extern StepperMotor *Motor[];
extern HMI_Command *hmicmd;

CMD g_cmdFunc[] = {
   	{"HMI_ID", cmdHMIID},
    {"SD", cmd_UpdateEEPROM},
    {"CD", cmd_ClearEEPROM},
    {"RD", cmd_Maindata},
	{"adc", getAdc},
	{"getgpio", getGpio},
	{"setgpio", setGpio},
	{"reset", resetArduino},
	{"getmicros", getMicros},
	{"ver", cmd_CodeVer},
	{"echoon", echoOn},
	{"echooff", echoOff},
	{"RunMode",cmdRunMode},
    {"CmdSendTest",CmdSendTest},
    {"MoveTo", cmdMotorMoveTo},
    {"Step", cmdMotorStep},
    {"Freq", cmdMotorFreq},
    {"Stop", cmdMotorStop},
    {"Accelerate", cmdMotorAccelerate},
    {"Slowdown", cmdMotorSlowdown},
    {"SlowdownStop", cmdMotorSlowdownStop},
    {"GetFreq", cmdMotorGetFreq},
    {"Position", cmdgetsetPosition},
    {"search", cmdSearchSensor},
    {"offset", cmd_Offset},
	{"?", showHelp}
};


String g_inputBuffer0 = "";
String* g_inputBuffer = NULL;
String g_cmd = "";
String g_arg = "";

bool g_echoOn = true;


bool getNextArg(String &arg)
{
  if (g_arg.length() == 0)
    return false;
  if (g_arg.indexOf(" ") == -1)
  {
    arg = g_arg;
    g_arg.remove(0);
  }
  else
  {
    arg = g_arg.substring(0, g_arg.indexOf(" "));
    g_arg = g_arg.substring(g_arg.indexOf(" ") + 1);
  }
  return true;
}


void resetArduino(void)
{
  wdt_enable(WDTO_500MS);
  while (1);
}
void cmdHMIID(void)
{
	String arg1;
	int value;

	if (!getNextArg(arg1))
	{
		cmd_port->println("HMI ID: " + String(maindata.HMI_ID));
		return;
	}
	value = arg1.toInt();
    if(value >= 0){
        maindata.HMI_ID = value;
        cmd_port->println("NEW HMI ID: " + String(maindata.HMI_ID));
        runtimedata.UpdateEEPROM = true;
    }
}
#if 0
uint8_t     HMI_ID;
long        MotorSpeed[2];
long        MotorFrequenceStart[2];
long        MotorAccelerateTime[2];
long        StationPosition[3];
uint16_t    OffsetDistanceOfStopPin;
int         TargetStation;
bool        Output_Last_HighLow[8];
uint16_t    CheckVersion;
long        VR_HomeOffset;

#endif
void cmd_Maindata(void)
{
    READ_EEPROM();
    cmd_port->println("HMI_ID:" + String(maindata.HMI_ID));
    for(uint8_t i=0; i<MOTOR_TOTAL; i++){
        cmd_port->println("Speed " +String(i) + ":" + String(maindata.MotorSpeed[i]));
        cmd_port->println("FrequenceStart " +String(i) + ":" + String(maindata.MotorFrequenceStart[i]));
        cmd_port->println("AccelerateTime " +String(i) + ":" + String(maindata.MotorAccelerateTime[i]));        
    }
    for(uint8_t i=0; i<3; i++){
        cmd_port->println("StationPosition " +String(i) + ":" + String(maindata.StationPosition[i]));        
    }
    cmd_port->println("OffsetDistanceOfStopPin: " + String(maindata.OffsetDistanceOfStopPin));
    cmd_port->println("TargetStation: " + String(maindata.TargetStation));
    cmd_port->print("Output_Last_HighLow: ");
    for(uint8_t i=0; i<8; i++){
        cmd_port->print(maindata.Output_Last_HighLow[i]);
        cmd_port->print(", ");
    }cmd_port->println("");
    cmd_port->println("CheckVersion: " + String(maindata.CheckVersion));
    cmd_port->println("VR_HomeOffset: " + String(maindata.VR_HomeOffset));
}
void cmd_UpdateEEPROM(void)
{
    runtimedata.UpdateEEPROM = true;
}
void cmd_ClearEEPROM(void)
{
    Clear_EEPROM();
}
void getMicros(void)
{
  cmd_port->println(String("micros:") + micros());
}

void showHelp(void)
{
  int i;

  cmd_port->println("");
  for (i = 0; i < (sizeof(g_cmdFunc) / sizeof(CMD)); i++)
  {
    cmd_port->println(g_cmdFunc[i].cmd);
  }
}

void getAdc(void)
{
  String arg1;
  int analogPin;
  int value;

  if (!getNextArg(arg1))
  {
    cmd_port->println("No parameter");
    return;
  }
  analogPin = arg1.toInt();
  value = analogRead(analogPin);
  cmd_port->print("ADC_");
  cmd_port->print(analogPin);
  cmd_port->print(" : ");
  cmd_port->println(value);
}

void getGpio(void)
{
  String arg1, arg2;
  int digitalPin, pullUp;
  int value;

  if (!getNextArg(arg1))
  {
    cmd_port->println("No parameter");
    return;
  }
  if (!getNextArg(arg2))
  {
    pullUp = 0;
  }
  else
  {
    pullUp = arg2.toInt();
  }
  digitalPin = arg1.toInt();
  if (arg2.compareTo("na") == 0)
  {
    cmd_port->println("pin mode keep original");
  }
  else
  {
    if (pullUp)
    {
      cmd_port->println("pull-up");
      pinMode(digitalPin, INPUT_PULLUP);
    }
    else
    {
      cmd_port->println("no-pull");
      pinMode(digitalPin, INPUT);
    }
  }

  cmd_port->print("GPIO:");
  cmd_port->println(arg1);

  value = digitalRead(digitalPin);

  cmd_port->print("input value:");
  cmd_port->println(value);
}

void setGpio(void)
{
  String arg1, arg2;
  int digitalPin;
  int value;

  if (!getNextArg(arg1))
  {
    cmd_port->println("No parameter 1");
    return;
  }
  if (!getNextArg(arg2))
  {
    cmd_port->println("No parameter 2");
    return;
  }
  digitalPin = arg1.toInt();
  value = arg2.toInt();

  cmd_port->print("GPIO:");
  cmd_port->println(arg1);
  cmd_port->print("level:");
  cmd_port->println(arg2);

  digitalWrite(digitalPin, value ? HIGH : LOW);
  pinMode(digitalPin, OUTPUT);
  //cmd_port->println("");
}

void echoOn(void)
{
  g_echoOn = true;
}

void echoOff(void)
{
  g_echoOn = false;
}

void cmd_CodeVer(void)
{
	cmd_port->println(VERSTR);
}
#if 0
#define RUN_MODE_EMERGENCY		        -1
#define RUN_MODE_SERVO_STOP				0
#define RUN_MODE_SERVO_INIT				1
#define RUN_MODE_SERVO_NORMAL			2
#define RUN_MODE_SERVO_SEARCH_SENSOR	3

#define RUN_MODE_VR_STOP			    0
#define RUN_MODE_VR_INIT				1
#define RUN_MODE_VR_IO_INIT				2
#define RUN_MODE_VR_NORMAL				3
#endif
char *RunModeServoStrings[] = 
{ 
    "RUN_MODE_EMERGENCY: -1",
    "RUN_MODE_SERVO_STOP: 0",
    "RUN_MODE_SERVO_INIT: 1",
    "RUN_MODE_SERVO_NORMAL: 2",
    "RUN_MODE_SERVO_SEARCH_SENSOR: 3",
};
const size_t nb_Servo_strings = sizeof(RunModeServoStrings) / sizeof(RunModeServoStrings[0]);
char *RunModeVRStrings[] = 
{ 
    "RUN_MODE_EMERGENCY: -1",
    "RUN_MODE_VR_STOP: 0",
    "RUN_MODE_VR_INIT: 1",
    "RUN_MODE_VR_IO_INIT: 2",
    "RUN_MODE_VR_NORMAL: 3",
};
const size_t nb_VR_strings = sizeof(RunModeVRStrings) / sizeof(RunModeVRStrings[0]);

void cmdRunMode(void)
{
	String arg1,arg2;
	int motor,value;
	if(!getNextArg(arg1))
	{
		cmd_port->println("No parameter1");
		return;
	}
	motor = arg1.toInt();
    for(int i=0; i<nb_Servo_strings; i++){
        cmd_port->println(RunModeServoStrings[i]);
    }
    for(int i=0; i<nb_VR_strings; i++){
        cmd_port->println(RunModeVRStrings[i]);
    }
	if(!getNextArg(arg2))
	{
        if(motor == MOTOR_SERVO){
            cmd_port->println("Servo RunMode: " + String(runtimedata.RunMode[MOTOR_SERVO]));
        }
        else if(motor == MOTOR_VR){
            cmd_port->println("VR RunMode: " + String(runtimedata.RunMode[MOTOR_VR]));
        }
		return;
	}
    value = arg2.toInt();
    if(motor == MOTOR_SERVO){
        runtimedata.RunMode[MOTOR_SERVO] = value;
        if(value == RUN_MODE_SERVO_INIT){
            runtimedata.Workindex[WORKINDEX_SERVO_INITIAL] = 0;
            runtimedata.Workindex[WORKINDEX_GO_HOME] = 0;
            runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0;
        }
        if(value == RUN_MODE_SERVO_SEARCH_SENSOR)
        {
            runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0;
        }
        DEBUG("Servo RunMode: " + String(runtimedata.RunMode[MOTOR_SERVO]));
        }
    else if(motor == MOTOR_VR){
	    runtimedata.RunMode[MOTOR_VR] = value;
        if(value == RUN_MODE_VR_INIT){
            runtimedata.Workindex[WORKINDEX_VR_INITIAL] = 0;
            runtimedata.Workindex[WORKINDEX_VR_GO_HOME] = 0;
        }
        DEBUG("VR RunMode: " + String(runtimedata.RunMode[MOTOR_VR]));
    }
}

void CmdSendTest()
{
    hmicmd->Response_Ping();
}
void cmdMotorStep(void)
{
	String arg1, arg2, arg3;
	long motorNumber, stepToMove, frequece;

	getNextArg(arg1);
	getNextArg(arg2);
	getNextArg(arg3);
	if( (arg1.length()==0)||(arg2.length()==0) )
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	motorNumber = arg1.toInt();
	stepToMove = arg2.toInt();
	if(arg3.length()==0)
	{
		switch(motorNumber)
		{
			case 0: 
			case 1:
				if(Motor[motorNumber]->getAccelerateTime() == 0)
					Motor[motorNumber]->setAccelerateTime(200);
				Motor[motorNumber]->Steps(stepToMove);
				break;
		}
		
	}
	else
	{
		frequece = arg3.toInt();
		switch(motorNumber)
		{
			case 0:
			case 1:
				if(Motor[motorNumber]->getAccelerateTime() == 0)
					Motor[motorNumber]->setAccelerateTime(200);
				Motor[motorNumber]->Steps(stepToMove, frequece); 
				break;
		}
	}
	
}


void cmdMotorMoveTo(void)
{
	String arg1, arg2, arg3;
	long motorNumber, targetposition, frequece;

	getNextArg(arg1);
	getNextArg(arg2);
	getNextArg(arg3);
	if( (arg1.length()==0)||(arg2.length()==0))
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	motorNumber = arg1.toInt();
	targetposition = arg2.toInt();
	if(arg3.length()==0)
	{
		switch(motorNumber)
		{
			case 0:
			case 1:
				Motor[motorNumber]->MoveTo(targetposition);
				break;
		}
	}
	else
	{
		frequece = arg3.toInt();
		switch(motorNumber)
		{
			case 0:
			case 1:
				Motor[motorNumber]->MoveTo(targetposition, frequece);
				break;
		}
	}
}

void cmdMotorFreq(void)
{
	String arg1, arg2;
	unsigned long motorNumber, frequence;

	getNextArg(arg1);
	if(arg1.length() == 0)
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	motorNumber = arg1.toInt();
	if(getNextArg(arg2))
	{
		frequence = arg2.toInt();
		switch(motorNumber)
		{
			case 0: 
			case 1: 
			case 2: 
				maindata.MotorSpeed[motorNumber] = frequence;
				Motor[motorNumber]->setFrequence((unsigned long)frequence); 
				runtimedata.UpdateEEPROM = true;
				break;
			default: cmd_port->println("unknown motor number"); break;
		}
	}
	else
	{
		switch(motorNumber)
		{
			case 0: 
			case 1: 
			case 2: 
				frequence = maindata.MotorSpeed[motorNumber]; 
				cmd_port->println("Motor " + String(motorNumber) + " Frequence: " + String(frequence));
				break;
			default: cmd_port->println("unknown motor number"); break;
		}
	}
		
}

void cmdMotorStop(void)
{
    String arg1, arg2;
    unsigned long motorNumber, steps;

    getNextArg(arg1);
    if( (arg1.length()==0))
    {
        cmd_port->println("Please input enough parameters");
        return;
    }
    motorNumber = arg1.toInt();
    switch(motorNumber)
    {
        case 0:
            runtimedata.RunMode[motorNumber] = RUN_MODE_SERVO_STOP;
        	Motor[motorNumber]->Stop(); 
        	break;
        case 1:
            runtimedata.RunMode[motorNumber] = RUN_MODE_VR_STOP;
        	Motor[motorNumber]->Stop(); 
        	break;
        default: cmd_port->println("unknown motor number"); break;
    }
}

void cmdMotorAccelerate(void)
{
    String arg1, arg2;
    unsigned long motorNumber, steps;

    getNextArg(arg1);
    if( (arg1.length()==0))
    {
        cmd_port->println("Please input enough parameters");
        return;
    }
    motorNumber = arg1.toInt();
    switch(motorNumber)
    {
        case 0: 
        case 1: 
        case 2: 
        	Motor[motorNumber]->Accelerate(); 
        	break;
        default: cmd_port->println("unknown motor number"); break;
    }
}

void cmdMotorSlowdown(void)
{
    String arg1, arg2;
    unsigned long motorNumber, steps;

    getNextArg(arg1);
    if( (arg1.length()==0))
    {
        cmd_port->println("Please input enough parameters");
        return;
    }
    motorNumber = arg1.toInt();
    switch(motorNumber)
    {
        case 0: 
        case 1: 
        case 2: 
        	Motor[motorNumber]->Slowdown(); 
        	break;
        default: cmd_port->println("unknown motor number"); break;
    }
}

void cmdMotorSlowdownStop(void)
{
    String arg1, arg2;
    unsigned long motorNumber, steps;

    getNextArg(arg1);
    if( (arg1.length()==0))
    {
        cmd_port->println("Please input enough parameters");
        return;
    }
    motorNumber = arg1.toInt();
    switch(motorNumber)
    {
        case 0: 
        case 1: 
        case 2: 
        	Motor[motorNumber]->SlowdownStop(); 
        	break;
        default: cmd_port->println("unknown motor number"); break;
    }
}

void cmdMotorInfo()
{
	String arg1, arg2, arg3, arg4;
	int motorNumber;
	RotateInfo decinfo;
	RotateInfo accinfo;
	
	
	if(!getNextArg(arg1))
	{
	  cmd_port->println("No parameter 1");
	  return;
	}
	motorNumber = arg1.toInt();
    if(motorNumber > 2 && motorNumber < 0) return;
	cmd_port->println("Timer: " + String(Motor[motorNumber]->getTimer(), DEC) + ", VectNum: " + String(Motor[motorNumber]->getVectNum(), DEC));
	cmd_port->println("State: " + String(Motor[motorNumber]->getState(), DEC));
	cmd_port->println("Pin: " + String(Motor[motorNumber]->getPulsePin(), DEC) + ", " + String(Motor[motorNumber]->getDirPin(), DEC));
	cmd_port->println("Direction: " + String(Motor[motorNumber]->getDirection(), DEC));
	cmd_port->println("Resolution: " + String(Motor[motorNumber]->getResolution(), DEC));
	cmd_port->println("RPM: " + String(Motor[motorNumber]->getRPM()));
	cmd_port->println("Freq Target: " + String(Motor[motorNumber]->getFrequence(), DEC) + ", Startup: " + String(Motor[motorNumber]->getFrequenceStartup(), DEC) + ", Now: " + String(Motor[motorNumber]->getFrequenceNow(), DEC) + ", Rotate Mode: " + String(Motor[motorNumber]->getRotateMode(), DEC));
	cmd_port->println("Accelerate time: " + String(Motor[motorNumber]->getAccelerateTime(), DEC));
	cmd_port->println("TargetPosition: " + String(Motor[motorNumber]->getTargetPosition(), DEC));
	cmd_port->println("Position: " + String(Motor[motorNumber]->getPosition(), DEC) + ", PWMOnOff: " + String(Motor[motorNumber]->getPWMOnOff(Motor[motorNumber]->getTimer())));
	cmd_port->println("ACC: " + String(Motor[motorNumber]->AccelerateInfo.stepscnt, DEC) + " T: " + String(Motor[motorNumber]->AccelerateInfo.endms-Motor[motorNumber]->AccelerateInfo.startms) + ", DEC: " + String(Motor[motorNumber]->DecelerateInfo.stepscnt) + " T: " + String(Motor[motorNumber]->DecelerateInfo.endms-Motor[motorNumber]->DecelerateInfo.startms));
	cmd_port->println("INC: " + String(Motor[motorNumber]->FrequenceIncIndex, DEC) + " / " + String(sizeof(AccelerateTable) / 2, DEC) + " T: " + String(AccelerateTable[Motor[motorNumber]->FrequenceIncCnt]));
		
}

void cmdMotorGetFreq(void)
{
	cmd_port->println(String("motor1 freq:")+Motor[0]->getFrequence());
	cmd_port->println(String("motor2 freq:")+Motor[1]->getFrequence());
	cmd_port->println(String("motor3 freq:")+Motor[2]->getFrequence());
}

void cmdgetsetPosition(void)
{
	String arg1, arg2;
	unsigned long motorNumber, steps;
	
	getNextArg(arg1);
	if( (arg1.length()==0))
	{
	  cmd_port->println("Please input enough parameters");
	  return;
	}
	motorNumber = arg1.toInt();

	getNextArg(arg2);
	if(arg2.length()>0)
		Motor[motorNumber]->setPosition(arg2.toInt());
	cmd_port->println("Position: " + String(Motor[motorNumber]->getPosition()));
    DEBUG("Station: " + String(runtimedata.Station));
    DEBUG("PositionInput: " + String(runtimedata.PositionInput, BIN));
}

void cmdSearchSensor()
{
	String arg1, arg2;
	int targetposition;
	
	getNextArg(arg1);
	if( (arg1.length()==0))
	{
	  cmd_port->println("Please input targetposition.");      
      DEBUG("Now station: " + String(runtimedata.Station));    
	  return;
	}
	targetposition = arg1.toInt();
	maindata.TargetStation = targetposition;
    cmd_port->println("targetposition: " + String(maindata.TargetStation));	
    
    runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_SEARCH_SENSOR;
    runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0;
}

void cmd_Offset(void)
{
    String arg1, arg2;
	int motor,offset;
	
	getNextArg(arg1);
	if( (arg1.length()==0))
	{
	  cmd_port->println("Please input motor.");    
	  return;
	}
    motor = arg1.toInt();
    getNextArg(arg2);
	if( (arg2.length()==0))
	{
	  cmd_port->println("Please input offset.");
      if(motor == MOTOR_SERVO){
        DEBUG("OffsetDistanceOfStopPin: " + String(maindata.OffsetDistanceOfStopPin));
      }
      else if(motor == MOTOR_VR){
        DEBUG("VR_HomeOffset: " + String(maindata.VR_HomeOffset));     
      }
	  return;
	}
	offset = arg2.toInt();
    if(motor == MOTOR_SERVO){
        maindata.OffsetDistanceOfStopPin = offset;
        DEBUG("OffsetDistanceOfStopPin: " + String(maindata.OffsetDistanceOfStopPin));
    }
    else if(motor == MOTOR_VR){
        maindata.VR_HomeOffset = offset;
        DEBUG("VR_HomeOffset: " + String(maindata.VR_HomeOffset));     
    }
}


uint8_t UserCommWorkindex = 0;

uint16_t UserCommandTimeCnt = 0;
void UserCommand_Timer(void)
{
	if(UserCommandTimeCnt < 0xFF00)
		UserCommandTimeCnt += TIMER_INTERVAL_MS;
}

void UserCommand_Task(void)
{
  int i, incomingBytes, ret, cmdPortIndex;
  char data[2] = {0};

	switch(UserCommWorkindex)
	{
		case 0:
		{
			
			if(cmd_port->available())
			{
				g_inputBuffer = &g_inputBuffer0;
				UserCommWorkindex ++;
				UserCommandTimeCnt = 0;
			}
			break;
		}
		case 1:
		{
			if(UserCommandTimeCnt > 50)
				UserCommWorkindex ++;
			break;
		}
		case 2:
		{
		  if ( incomingBytes = cmd_port->available() )
		  {
#if USER_COMMAND_DEBUG  
			cmd_port->println("cmd_port datalen: " + String(incomingBytes));
#endif  	
			for ( i = 0; i < incomingBytes; i++ )
			{
			  ret = cmd_port->read();
			  if ( (ret >= 0x20) && (ret <= 0x7E) || (ret == 0x0D) || (ret == 0x0A) )
			  {
				data[0] = (char)ret;
				(*g_inputBuffer) += data;
				if (g_echoOn)
				{
				  if ( (data[0] != 0x0D) && (data[0] != 0x0A) )
					cmd_port->write(data);
				}
			  }
			  else if (ret == 0x08)
			  {
				if (g_inputBuffer->length())
				{
				  g_inputBuffer->remove(g_inputBuffer->length() - 1);
				  if (g_echoOn)
				  {
					data[0] = 0x08;
					cmd_port->write(data);
				  }
				}
			  }
			}
			if (g_inputBuffer->indexOf('\r') == -1)
			{
			  if (g_inputBuffer->indexOf('\n') == -1)
				return;
			}
			g_inputBuffer->trim();
			while (g_inputBuffer->indexOf('\r') != -1)
			  g_inputBuffer->remove(g_inputBuffer->indexOf('\r'), 1);
			while (g_inputBuffer->indexOf('\n') != -1)
			  g_inputBuffer->remove(g_inputBuffer->indexOf('\n'), 1);
			while (g_inputBuffer->indexOf("  ") != -1)
			  g_inputBuffer->remove(g_inputBuffer->indexOf("  "), 1);
		
			cmd_port->println();
		
			if (g_inputBuffer->length())
			{
			  g_arg.remove(0);
			  if (g_inputBuffer->indexOf(" ") == -1)
				g_cmd = (*g_inputBuffer);
			  else
			  {
				g_cmd = g_inputBuffer->substring(0, g_inputBuffer->indexOf(" "));
				g_arg = g_inputBuffer->substring(g_inputBuffer->indexOf(" ") + 1);
			  }
			  for (i = 0; i < (sizeof(g_cmdFunc) / sizeof(CMD)); i++)
			  {
				//if(g_cmd==g_cmdFunc[i].cmd)
				if (g_cmd.equalsIgnoreCase(g_cmdFunc[i].cmd))
				{
				  g_cmdFunc[i].func();
				  cmd_port->println();
//				  cmd_port->print("ARDUINO>");
				  break;
				}
				else if (i == (sizeof(g_cmdFunc) / sizeof(CMD) - 1))
				{
				  cmd_port->println("bad command !!");
				  cmd_port->println();
//				  cmd_port->print("ARDUINO>");
				}
			  }
			  *g_inputBuffer = "";
			}
			else
			{
			  cmd_port->println();
//			  cmd_port->print("ARDUINO>");
			}
			UserCommWorkindex = 0;
			break;
		}
	}

  }
}




