#include "HMI.h"
#include <SPI.h>
#include "Timer.h"
#include "MainProcess.h"
#include "UserCommand.h"
#include "HMI_Command.h"
#include "StepperMotor.h"
#include "EEPROM_Function.h"

extern StepperMotor *Motor[];
extern RuntimeStatus runtimedata;
extern MainDataStruct maindata;

#define INO_DEBUG 1

#if INO_DEBUG
HardwareSerial *cmd_port;
#endif

void setup()
{
	cmd_port = &CMD_PORT;
	cmd_port->begin(CMD_PORT_BR);

	READ_EEPROM();
	MainProcess_Init();	
	TimerInit(1, 10000);
	
#if INO_DEBUG	
	cmd_port->println("Start of setup().");
	cmd_port->print("Version: ");
	cmd_port->println(VERSTR);
	cmd_port->println("End of setup().");
#endif
	buzzerPlay(500);
}

void loop()
{
	MainProcess_Task();
	UserCommand_Task();
	if(runtimedata.UpdateEEPROM)
	{
		runtimedata.UpdateEEPROM = false;
		WRITE_EEPROM();
	}
}

ISR(TIMER1_COMPA_vect)
{
	UserCommand_Timer();
    MainProcess_Timer();
}


//for PWM 2
ISR(TIMER3_COMPB_vect)          // timer compare interrupt service routine
{
	Motor[0]->TimerProcess(TIMER3_COMPB_vect_num);
}

//for PWM 7
ISR(TIMER4_COMPB_vect)          // timer compare interrupt service routine
{
	Motor[1]->TimerProcess(TIMER4_COMPB_vect_num);
}

//for PWM 12
ISR(TIMER1_COMPB_vect)          // timer compare interrupt service routine
{
	Motor[2]->TimerProcess(TIMER1_COMPB_vect_num);
}

void buzzerPlay(int ms)
{
    pinMode(BUZZ, OUTPUT);
    digitalWrite(BUZZ, HIGH);
    delay(ms);
    digitalWrite(BUZZ, LOW);
}
