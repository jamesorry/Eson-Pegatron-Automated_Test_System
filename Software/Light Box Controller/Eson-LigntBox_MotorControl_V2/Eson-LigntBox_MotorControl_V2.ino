#include "Arduino.h"
#include "HMI.h"
#include "Timer.h"
#include "MainProcess.h"
#include "UserCommand.h"
#include "StepperMotor.h"
#include "EEPROM_Function.h"
#include "HMI_Command.h"

extern StepperMotor *Motor[];
extern RuntimeStatus runtimedata;
extern MainDataStruct maindata;
HardwareSerial *cmd_port;

HMI_Command *hmicmd;

void setup()
{
	cmd_port = &CMD_PORT;
	cmd_port->begin(CMD_PORT_BR);
	
	hmicmd = new HMI_Command(&RS485_PORT, RS485_PORT_BR);

	READ_EEPROM();
	MainProcess_Init();	
	TimerInit(1, 10000);
	
	cmd_port->print("Version: ");
	cmd_port->println(VERSTR);
	cmd_port->println("End of setup().");
	buzzerPlay(500);
}

void loop()
{
	MainProcess_Task();
	UserCommand_Task();
    hmicmd->Process();
	if(runtimedata.UpdateEEPROM)
	{
		runtimedata.UpdateEEPROM = false;
		WRITE_EEPROM();
	}
}

ISR(TIMER1_COMPA_vect)
{
	UserCommand_Timer();
    if(!digitalRead(InputPin[IN00_EmergencyPin]) 
        && (runtimedata.RunMode[MOTOR_SERVO] != RUN_MODE_EMERGENCY || runtimedata.RunMode[MOTOR_VR] != RUN_MODE_EMERGENCY)){
        runtimedata.RunMode[MOTOR_SERVO] = RUN_MODE_SERVO_STOP;
        runtimedata.RunMode[MOTOR_VR] = RUN_MODE_VR_STOP;
        runtimedata.IndicationEmergency = true;
    }
}

//for PWM 2
ISR(TIMER3_COMPB_vect)          // timer compare interrupt service routine
{
	Motor[MOTOR_SERVO]->TimerProcess(TIMER3_COMPB_vect_num);
}

//for PWM 7
ISR(TIMER4_COMPB_vect)          // timer compare interrupt service routine
{
	Motor[MOTOR_VR]->TimerProcess(TIMER4_COMPB_vect_num);
}

