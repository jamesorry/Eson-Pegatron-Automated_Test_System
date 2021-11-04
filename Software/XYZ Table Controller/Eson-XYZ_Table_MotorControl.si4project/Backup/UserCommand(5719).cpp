#include "UserCommand.h"
#include "HMI.h"
#include <avr/wdt.h>
#include "SoftwareSerial.h"
#include "hmi_command.h"
#include "MainProcess.h"
#include "Timer.h"
#include "StepperMotor.h"
#include "EEPROM_Function.h"

#if INO_DEBUG
    #define USER_COMMAND_DEBUG	1
#endif

extern HardwareSerial *cmd_port;
extern SoftwareSerial Serial4;
extern volatile unsigned long timer0_overflow_count;
extern volatile unsigned long timer0_millis;
extern RuntimeStatus	runtimedata;
extern MainDataStruct maindata;
extern DigitalIO digitalio;
extern StepperMotor *Motor[];

CMD g_cmdFunc[] = {
    {"SD", cmd_UpdateEEPROM},
    {"CD", cmd_ClearEEPROM},
    {"RD", cmd_Maindata},
	{"adc", getAdc},
	{"getgpio", getGpio},
	{"setgpio", setGpio},
	{"reset", resetArduino},
	{"resetmicros", resetMicrosTimerCount},
	{"getmicros", getMicros},
	{"ver", cmd_CodeVer},
	{"echoon", echoOn},
	{"echooff", echoOff},
	{"HMI_ID", cmdHMIID},
	{"getdo", cmd_GetDO},
	{"setdo", cmd_SetDO},
	{"workindex", cmd_WorkIndex},
	
	{"step", cmdMotorStep},
	{"moveto", cmdMotorMoveTo},
	{"movetostoppin", cmdMotorMoveToStopPin},
	{"freq", cmdMotorFreq},
	{"rm", cmdMotorSetRotateMode},
	{"stop", cmdMotorStop},
	{"acc", cmdMotorAccelerate},
	{"sd", cmdMotorSlowdown},
	{"sstop", cmdMotorSlowdownStop},
	{"getfreq", cmdMotorGetFreq},
	{"speed", cmdMotorSpeed},
	{"rpm", cmdMotorSetRPM},
	{"slowdownpin", cmdSetSlowdownPin},
	{"slowdownStoppin", cmdSetSlowdownStopPin},
	{"stoppin", cmdSetStopPin},
	{"stopgpiopin", cmdSetStopGpioPin},
	{"setpwm", setPWM},
	{"info", cmdMotorInfo},
	{"position", cmdgetsetPosition},
	{"searchsensor", cmdSearchSensor},
	
	{"gohome", cmdgoHome},
	{"init", cmdInit},
	{"out", cmdOutput},
	{"in", cmdInput},
	{"runmode", cmdRunMode},
	{"hmitest", cmdHMITest},
    {"Acctime", cmd_SetAcctime},
	{"?", showHelp}
};


String g_inputBuffer0 = "";
String* g_inputBuffer = NULL;
String g_cmd = "";
String g_arg = "";

bool g_echoOn = true;

uint32_t targetcnt = 0;

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

void cmd_Maindata(void)
{
    READ_EEPROM();
    cmd_port->println("HMI_ID:" + String(maindata.HMI_ID));
    for(uint8_t i=0; i<MOTOR_TOTAL; i++){
        cmd_port->println("Speed " +String(i) + ":" + String(maindata.MotorSpeed[i]));
        cmd_port->println("FrequenceStart " +String(i) + ":" + String(maindata.MotorFrequenceStart[i]));
        cmd_port->println("AccelerateTime " +String(i) + ":" + String(maindata.MotorAccelerateTime[i]));        
    }
    cmd_port->println("TargetPosition: " + String(maindata.TargetPosition));
    cmd_port->println("CheckVersion: " + String(maindata.CheckVersion));
}
void cmd_UpdateEEPROM(void)
{
    runtimedata.UpdateEEPROM = true;
}
void cmd_ClearEEPROM(void)
{
    Clear_EEPROM();
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


void cmd_WriteToSerial(void)
{
	  String arg1, arg2;
	char atcmd[32], ch;
	uint8_t i;
	if(!getNextArg(arg1))
		
	{
		cmd_port->println("No parameter1");
		return;
	}
	if(!getNextArg(arg2))
		
	{
		cmd_port->println("No parameter2");
		return;
	}
	if(arg1 == "0")
	{
		Serial.write(arg2.c_str());
	}
	else if(arg1 == "1")
	{
		Serial1.write(arg2.c_str());
		cmd_port->println("send to Serial1.");
	}
	else if(arg1 == "2")
	{
		Serial2.write(arg2.c_str());
		cmd_port->println("send to Serial2.");
	}
	else if(arg1 == "3")
	{
		Serial3.write(arg2.c_str());
		cmd_port->println("send to Serial3.");
	}
	
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


void cmd_SetDO(void)
{
	String arg1, arg2;
	int i, bytei;
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
	cmd_GetDO();
	bytei = arg1.toInt();
	value = arg2.toInt();
	for(i=0; i<8; i++)
		setOutput(bytei*8+i, bitRead(value, i));

	cmd_port->print("set DO" + String(bytei) + ": ");
	cmd_port->println(String( value, HEX));

}

void cmdOutput(void)
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

	cmd_port->print("PIN index:");
	cmd_port->println(arg1);
	cmd_port->print("level:");
	cmd_port->println(arg2);

	setOutput((uint8_t)digitalPin, (uint8_t)(value ? HIGH : LOW));
	//cmd_port->println("");
}

void cmd_GetDO(void)
{
	int i, bytei;
	uint8_t outbuf;
	cmd_port->print("get DO: ");
	for(bytei=0; bytei<(OUTPUT_8_NUMBER + EXTIO_NUM); bytei++)
	{
		outbuf = 0;
		for(i=0; i<8; i++)
		{
			outbuf |= (digitalio.Output[bytei*8+i] & 0x01) << i;
			cmd_port->print(String(digitalio.Output[i], HEX) + " ");
		}
		cmd_port->println(": " + String(outbuf, HEX));
	}
}
void cmdInput(void)
{
	String arg1, arg2;
	unsigned long pinindex;

	getNextArg(arg1);
	if( (arg1.length()==0))
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	pinindex = arg1.toInt();
	cmd_port->println("Sensor: " + String(getInput(pinindex)));
}

void cmd_WorkIndex(void)
{
	String arg1;
	int index;
	if (!getNextArg(arg1))
	{
		cmd_port->println("RunMode: " + String(runtimedata.RunMode));
        return;
	}
    runtimedata.Workindex[runtimedata.RunMode] = arg1.toInt();
	cmd_port->print("Workindex: ");
	for(int i=0; i<WORKINDEX_TOTAL; i++)
		cmd_port->print(String(runtimedata.Workindex[i]) + " ");
	cmd_port->println();
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

void cmd_SetAcctime()
{    
    String arg1, arg2;
    long motorNumber, Acctime;

    getNextArg(arg1);
    getNextArg(arg2);
    if( (arg1.length()==0))
    {
        cmd_port->println("Please input enough parameters");
        return;
    }
    if( (arg2.length()==0))
    {
        cmd_port->println("Please input enough parameters 2");
        return;
    }
    motorNumber = arg1.toInt();
    Acctime = arg2.toInt();
    switch(motorNumber)
    {
        case 0: 
        case 1: 
        case 2: 
            maindata.MotorAccelerateTime[motorNumber] = Acctime;        
            Motor[motorNumber]->setAccelerateTime(maindata.MotorAccelerateTime[motorNumber]);
            cmd_port->println("ACC:" + String(Motor[motorNumber]->getAccelerateTime()));
            runtimedata.UpdateEEPROM = true;
            break;
        default: cmd_port->println("unknown Motor number"); break;
    }
}
void cmdRunMode(void)
{
	String arg1, arg2;
	int runmode = 0;
	
	if(getNextArg(arg1))
	{
		runtimedata.RunMode = arg1.toInt();
        for(uint8_t i=0; i<WORKINDEX_TOTAL; i++)
            runtimedata.Workindex[i] = 0;
//		runtimedata.Workindex[runtimedata.RunMode] = 0;
	}
	cmd_port->println("Run mode: " + String(runtimedata.RunMode));
    
}

void cmdHMITest(void)
{
	String arg1, arg2;
	if(!getNextArg(arg1))
	{
        
    }
    cmd_port->print("PositionInput: ");
    for(int i = 10; i >= 0; i--)
        cmd_port->print(getbit(runtimedata.PositionInput, i));
    cmd_port->println();
    
    if(Motor[MOTOR_X]->getState() == MOTOR_STATE_STOP)
        cmd_port->println("Motor stop"); //status Motor stop.
    else
        cmd_port->println("Motor is running"); //status Motor is running.
//    cmd_port->println("PositionInput: " + String(runtimedata.PositionInput, BIN));
    DEBUG("Pos:" + String(Motor[MOTOR_X]->getPosition()));
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
			case 2: 
				if(Motor[MOTOR_X]->getAccelerateTime() <= 200)
                    Motor[MOTOR_X]->setAccelerateTime(maindata.MotorAccelerateTime[MOTOR_X]);
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
			case 2: 
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
			case 2:
                if(Motor[MOTOR_X]->getAccelerateTime() <= 200)
                    Motor[MOTOR_X]->setAccelerateTime(maindata.MotorAccelerateTime[MOTOR_X]);
				Motor[motorNumber]->MoveTo(targetposition, frequece); 
				break;
		}
	}
}

void cmdgoHome(void)
{
	String arg1, arg2, arg3;
	long station, frequece;

	getNextArg(arg1);
	getNextArg(arg2);

//	runtimedata.Workindex[WORKINDEX_GO_HOME] = 0;
//	runtimedata.RunMode = RUN_MODE_GO_HOME;
}

void cmdInit(void)
{
	String arg1, arg2, arg3;
	long station, frequece;

	getNextArg(arg1);
	getNextArg(arg2);

//	runtimedata.Workindex[WORKINDEX_INITIAL] = 0;
//	runtimedata.RunMode = RUN_MODE_INIT;
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
				Motor[motorNumber]->MoveToStopPin(targetposition); 
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
				Motor[motorNumber]->MoveToStopPin(targetposition, frequece); 
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
			default: cmd_port->println("unknown Motor number"); break;
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
			default: cmd_port->println("unknown Motor number"); break;
		}
	}
		
}

void cmdMotorSetRotateMode(void)
{
    String arg1, arg2;
    unsigned long motorNumber;
    int	freqinc;

    getNextArg(arg1);
    getNextArg(arg2);
    if( (arg1.length()==0)||(arg2.length()==0) )
    {
        cmd_port->println("Please input enough parameters");
        return;
    }
    motorNumber = arg1.toInt();
    freqinc = arg2.toInt();
    switch(motorNumber)
    {
        case 0: 
        case 1: 
        case 2: 
            Motor[motorNumber]->setRotateMode(freqinc); 
            break;
        default: cmd_port->println("unknown Motor number"); break;
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
        case 1: 
        case 2: 
        	runtimedata.RunMode = RUN_MODE_STOP;
        	Motor[motorNumber]->Stop(); 
        	break;
        default: cmd_port->println("unknown Motor number"); break;
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
        default: cmd_port->println("unknown Motor number"); break;
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
        default: cmd_port->println("unknown Motor number"); break;
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
        default: cmd_port->println("unknown Motor number"); break;
    }
}


void cmdMotorGetFreq(void)
{
	cmd_port->println(String("motor0 freq:")+Motor[0]->getFrequence());
	cmd_port->println(String("motor1 freq:")+Motor[1]->getFrequence());
	cmd_port->println(String("motor2 freq:")+Motor[2]->getFrequence());
}


void cmdMotorSetRPM(void)
{
	String arg1, arg2;
	unsigned long motorNumber, rpm;

	getNextArg(arg1);
	getNextArg(arg2);
	if( (arg1.length()==0)||(arg2.length()==0) )
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	motorNumber = arg1.toInt();
	rpm = arg2.toInt();
	switch(motorNumber)
	{
		case 0: 
		case 1: 
		case 2: 
			Motor[motorNumber]->setRPM((unsigned long)rpm); 
			break;
		default: cmd_port->println("unknown Motor number"); break;
	}
}

void cmdSetStopPin(void)
{
	String arg1, arg2, arg3, arg4, arg5;
	unsigned long motorNumber;
	int pin, state, toggletimes = 0, range = 0;

	getNextArg(arg1);
	getNextArg(arg2);
	getNextArg(arg3);
	getNextArg(arg4);
	getNextArg(arg5);
	if( (arg1.length()==0)||(arg2.length()==0) )
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	motorNumber = arg1.toInt();
	pin = arg2.toInt();
	pinMode(InputPin[pin], INPUT_PULLUP);

	if(arg3.length()==0)
		state = HIGH;
	else
		state = arg3.toInt();

	if(arg4.length() > 0)
		toggletimes = arg4.toInt();

	if(arg5.length() > 0)
	{
		range = arg5.toInt();
		switch(motorNumber)
		{
			case 0: 
			case 1: 
			case 2: 
                Motor[motorNumber]->setStopRangePin(InputPin[pin], state, toggletimes, range); 
                break;
		}
	}
	else
	{
		switch(motorNumber)
		{
			case 0: 
			case 1: 
			case 2: 
                Motor[motorNumber]->setStopPin(InputPin[pin], state, toggletimes); 
                break;
		}
	}

	cmd_port->print("stop " + String(pin) + ": ");
	cmd_port->print(InputPin[pin]);
	cmd_port->print(", active:");
	cmd_port->print(state);
	cmd_port->print(", Times: ");
	cmd_port->print(toggletimes);
	cmd_port->print(", range: ");
	cmd_port->println(range);
}

void cmdSetStopGpioPin(void)
{
    String arg1, arg2, arg3;
    unsigned long motorNumber;
    int pin, state;

    getNextArg(arg1);
    getNextArg(arg2);
    getNextArg(arg3);
    if( (arg1.length()==0)||(arg2.length()==0) )
    {
        cmd_port->println("Please input enough parameters");
        return;
    }
    motorNumber = arg1.toInt();
    pin = arg2.toInt();
    pinMode(pin, INPUT_PULLUP);

    if(arg3.length()==0)
    {
        state = HIGH;
    }
    else
    {
        state = arg3.toInt();
    }
    switch(motorNumber)
    {
        case 0: 
        case 1: 
        case 2: 
            Motor[motorNumber]->setStopPin(pin, state, 0); 
            break;
    }

    cmd_port->print("stop:");
    cmd_port->print(pin);
    cmd_port->print(", active:");
    cmd_port->println(state);
}


void cmdSetSlowdownPin(void)
{
  String arg1, arg2, arg3, arg4;
  unsigned long motorNumber;
  int pin, state, toggletimes = 0;

  getNextArg(arg1);
  getNextArg(arg2);
  getNextArg(arg3);
  getNextArg(arg4);
  if( (arg1.length()==0)||(arg2.length()==0) )
  {
    cmd_port->println("Please input enough parameters");
    return;
  }
  motorNumber = arg1.toInt();
  pin = arg2.toInt();
  pinMode(InputPin[pin], INPUT_PULLUP);
  
  if(arg3.length()==0)
  {
	  state = HIGH;
  }
  else
  {
	  state = arg3.toInt();
  }
  if(arg4.length() > 0)
  {
	  toggletimes = arg4.toInt();
  }
  switch(motorNumber)
  {
	  case 0: 
	  case 1: 
	  case 2: Motor[motorNumber]->setSlowdownPin(InputPin[pin], state, toggletimes); break;
  }

  cmd_port->print("Slowdown " + String(pin) + ": ");
  cmd_port->print(InputPin[pin]);
  cmd_port->print(", active:");
  cmd_port->print(state);
  cmd_port->print(", Times: ");
  cmd_port->println(toggletimes);
}

void cmdSetSlowdownStopPin(void)
{
	String arg1, arg2, arg3, arg4;
	unsigned long motorNumber;
	int pin, state, toggletimes = 0;

	getNextArg(arg1);
	getNextArg(arg2);
	getNextArg(arg3);
	getNextArg(arg4);
	if((arg1.length() == 0) || (arg2.length() == 0))
	{
		cmd_port->println("Please input enough parameters");
		return;
	}
	motorNumber = arg1.toInt();
	pin = arg2.toInt();
	pinMode(InputPin[pin], INPUT_PULLUP);

	if(arg3.length() == 0)
		state = HIGH;
	else
		state = arg3.toInt();

	if(arg4.length() > 0)
		toggletimes = arg4.toInt();
	switch(motorNumber)
	{
		case 0: 
		case 1: 
		case 2: Motor[motorNumber]->setSlowdownStopPin(InputPin[pin], state, toggletimes); break;
	}

	cmd_port->print("Slowdown stop " + String(pin) + ": ");
  cmd_port->print(InputPin[pin]);
  cmd_port->print(", active:");
  cmd_port->print(state);
  cmd_port->print(", Times: ");
  cmd_port->println(toggletimes);
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
            Motor[motorNumber]->Speed(freq, freqstartup, acctime); 
            break;
	}
	
}

void cmdSearchSensor()
{
	String arg1;
	int dir = -1;
	if(getNextArg(arg1))
	  dir = arg1.toInt();

//#if BOARD_FLATCAR
//	runtimedata.Workindex[WORKINDEX_SEARCH_SENSOR] = 0;
//	runtimedata.RunMode = RUN_MODE_SEARCH_SENSOR;
//	runtimedata.SerarchDir = dir;
//#else
//	SearchSensor(dir);
//#endif
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



void setPWM(void)
{
  String arg1, arg2, arg3, arg4;
  int pwmPin;
  int value;
  long prescale;

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
  pwmPin = arg1.toInt();
  value = arg2.toInt();

  //reg.TCCRxB = _BV(WGM42) | _BV(CS42) | _BV(CS41) | _BV(CS40);
  //CS2  Divisor  Frequency
  //001    1        31372.55
  //010    8        3921.16
  //011    32       980.39
  //100    64       490.20   <--DEFAULT
  //101    128      245.10
  //110    256      122.55
  //111    1024     30.64



  if(getNextArg(arg3))
  {
	  prescale = arg3.toInt();
#if 0
	  switch(prescale)
	  {
		  case 1: TCCR4B = _BV(WGM42) | _BV(CS40); break;
		  case 8: TCCR4B = _BV(WGM42) | _BV(CS41); break;
		  case 32: TCCR4B = _BV(WGM42) | _BV(CS41) | _BV(CS40); break;
		  case 64: TCCR4B = _BV(WGM42) | _BV(CS42); break;
		  case 128: TCCR4B = _BV(WGM42) | _BV(CS42) | _BV(CS40); break;
		  case 256: TCCR4B = _BV(WGM42) | _BV(CS42) | _BV(CS41); break;
		  case 1024: TCCR4B = _BV(WGM42) | _BV(CS42) | _BV(CS41) | _BV(CS40); break;
	  }
#else	  
		switch(prescale)
		{
			case 1: TCCR4B =  _BV(CS40); break;
			case 8: TCCR4B =  _BV(CS41); break;
			case 32: TCCR4B =  _BV(CS41) | _BV(CS40); break;
			case 64: TCCR4B =  _BV(CS42); break;
			case 128: TCCR4B =  _BV(CS42) | _BV(CS40); break;
			case 256: TCCR4B =  _BV(CS42) | _BV(CS41); break;
			case 1024: TCCR4B =  _BV(CS42) | _BV(CS41) | _BV(CS40); break;
		}
#endif	  
  }
  
  if(getNextArg(arg4))
  {
	  OCR4C = (unsigned int)arg4.toInt();
  }

  analogWrite(pwmPin, value);
  cmd_port->println("PIN:" + String(pwmPin, DEC) + " pwm:" + String(value, DEC) + " div:" + String(prescale) + " TCCR4B:" + String(TCCR4B, HEX) + "(" + String(TCCR4B, BIN) + ")" + " OCR4C:" + String(OCR4C, HEX) + "(" + String(OCR4C, BIN) + ")");

/*	if((value == 0) || (value == 255))
  		TIMSK4 &= ~(1<<OCIE4C); //disable timer compare interrupt
  	else
  		TIMSK4 |= (1<<OCIE4C);  //enable timer compare interrupt*/
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




