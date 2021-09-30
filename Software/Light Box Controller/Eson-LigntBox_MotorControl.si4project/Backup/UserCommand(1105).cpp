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
extern SoftwareSerial *wifi_port;
extern volatile unsigned long timer0_overflow_count;
extern volatile unsigned long timer0_millis;
extern RuntimeStatus	runtimedata;
extern MainDataStruct maindata;
extern StepperMotor *Motor[];


CMD g_cmdFunc[] = {
	{"adc", getAdc},
	{"getgpio", getGpio},
	{"setgpio", setGpio},
	{"reset", resetArduino},
	{"resetmicros", resetMicrosTimerCount},
	{"getmicros", getMicros},
	{"ver", cmd_CodeVer},
	{"echoon", echoOn},
	{"echooff", echoOff},
	
    {"MoveTo", cmdMotorMoveTo},
    {"MotorMoveToStopPin", cmdMotorMoveToStopPin},   
    {"Speed", cmdMotorSpeed},
    {"Info", cmdMotorInfo},
    {"Freq", cmdMotorFreq},   
    {"FreqStartUp", cmdMotorFreqStartUp},
    {"AccelerateTime", cmdMotorAccelerateTime},
    {"Step", cmdMotorStep},
    {"ClearEEPROM", cmdClearEEPROM},
    {"SearchSensor", cmdSearchSensor},
    {"position", cmdgetsetPosition},
    {"stationpos", cmdStationPosition},
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
void getMicros(void)
{
  cmd_port->println(String("micros:") + micros());
}


void resetMicrosTimerCount(void)
{
  timer0_overflow_count = 0;
  timer0_millis = 0;
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
			case 2: 
				if(Motor[motorNumber]->getAccelerateTime() == 0)
					Motor[motorNumber]->setAccelerateTime(200);
				Motor[motorNumber]->Steps(stepToMove) >> 1; 
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
			case 2: 
				if(Motor[motorNumber]->getAccelerateTime() == 0)
					Motor[motorNumber]->setAccelerateTime(200);
				Motor[motorNumber]->Steps(stepToMove, frequece) >> 1; 
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
			case 2: 
				Motor[motorNumber]->MoveTo(targetposition) >> 1;
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
			case 2: 
				Motor[motorNumber]->MoveTo(targetposition, frequece) >> 1;
				break;
		}
	}
}
void cmdMotorMoveToStopPin(void)
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
			case 2: 
				Motor[motorNumber]->MoveToStopPin(targetposition) >> 1; 
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
			case 2: 
				Motor[motorNumber]->MoveToStopPin(targetposition, frequece) >> 1; 
				break;
		}
	}
}

void cmdMotorSpeed()
{
	String arg1, arg2, arg3, arg4;
	int motorNumber;
	long freq, freqstartup;
	int acctime = 0;
	
	if(!getNextArg(arg1))
	{
	  cmd_port->println("No parameter 1");
	  return;
	}
	if(!getNextArg(arg2))
	{
	  cmd_port->println("No parameter 2");
	  return;
	}
	motorNumber = arg1.toInt();
	freq = arg2.toInt();
	getNextArg(arg3);
	if(arg3.length()>0)
	{
		freqstartup = arg3.toInt();
		getNextArg(arg4);
		if(arg4.length()>0)
			acctime = arg4.toInt();
	}
	
	switch(motorNumber)
	{
		case 0:
		case 1: 
		case 2:
            Motor[motorNumber]->Speed(freq, freqstartup, acctime) >> 1; break;
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
		
	runtimedata.UpdateEEPROM = true;
}

void cmdMotorFreqStartUp(void)
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
				Motor[motorNumber]->setFrequenceStartup((unsigned long)frequence); 
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
				frequence = Motor[motorNumber]->getFrequenceStartup(); 
				cmd_port->println("Motor " + String(motorNumber) + " Frequence: " + String(frequence));
				break;
			default: cmd_port->println("unknown motor number"); break;
		}
	}
	runtimedata.UpdateEEPROM = true;
		
}

void cmdMotorAccelerateTime(void)
{
	String arg1, arg2;
	unsigned long motorNumber, AccelerateTime;

	getNextArg(arg1);
	if(arg1.length() == 0)
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	motorNumber = arg1.toInt();
	if(getNextArg(arg2))
	{
		AccelerateTime = arg2.toInt();
		switch(motorNumber)
		{
			case 0: 
			case 1: 
			case 2: 
				maindata.MotorSpeed[motorNumber] = AccelerateTime;
				Motor[motorNumber]->setAccelerateTime((unsigned long)AccelerateTime); 
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
				AccelerateTime = Motor[motorNumber]->getAccelerateTime(); 
				cmd_port->println("Motor " + String(motorNumber) + " Frequence: " + String(AccelerateTime));
				break;
			default: cmd_port->println("unknown motor number"); break;
		}
	}
	runtimedata.UpdateEEPROM = true;
		
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

void cmdClearEEPROM(void)
{
    Clear_EEPROM();
}

void cmdSearchSensor()
{
	String arg1,arg2;
	int dir = -1;
    int toggletime = 0;
	if(getNextArg(arg1))
	    dir = arg1.toInt();
    if(getNextArg(arg2))
        toggletime = arg2.toInt();
    cmd_port->println("dir:" + String(dir));
    cmd_port->println("toggletime:" + String(toggletime));
    
//	runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0;
	runtimedata.SerarchDir = dir;

	SearchSensor(dir, toggletime);
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
}

void cmdStationPosition(void)
{
	String arg1, arg2;
	int station, i;
	
	getNextArg(arg1);
	if(arg1.length()==0)
	{
		for(i=0; i<STATION_TOTAL; i++)
		  cmd_port->println("Station " + String(i) + ": " + String(maindata.StationPosition[i]));
	  return;
	}
	station = arg1.toInt();

	if(station < STATION_TOTAL)
	{
		getNextArg(arg2);
		if(arg2.length()>0)
		{
			maindata.StationPosition[station] = arg2.toInt();
			runtimedata.UpdateEEPROM = true;
		}
		cmd_port->println("Station " + String(station) + ": " + String(maindata.StationPosition[station]));
	}
	else
		for(i=0; i<STATION_TOTAL; i++)
		  cmd_port->println("Station " + String(i) + ": " + String(maindata.StationPosition[i]));
}


uint8_t UserCommWorkindex = 0;
uint16_t UserCommandTimeCnt = 0;
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
				UserCommandTimeCnt = millis();
			}
			break;
		}
		case 1:
		{
			if(millis() - UserCommandTimeCnt > 50)
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
