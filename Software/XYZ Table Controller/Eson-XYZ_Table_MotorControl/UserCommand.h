#ifndef _USER_COMMAND_H_
#define	_USER_COMMAND_H_

#include "Arduino.h"

typedef struct __CMD {
  const char* cmd;
  void (*func)(void);
} CMD, *PCMD;

void cmd_Maindata(void);
void cmd_UpdateEEPROM(void);
void cmd_ClearEEPROM(void);
void resetArduino(void);
void getMicros(void);
void resetMicrosTimerCount(void);
void getAdc(void);
void getGpio(void);
void setGpio(void);
void echoOn(void);
void echoOff(void);
void cmd_CodeVer(void);
void showHelp(void);
bool getNextArg(String &arg);
void UserCommand_Timer(void);
void UserCommand_Task(void);

void cmd_WriteToSerial(void);
void cmd_WorkIndex(void);
void cmdHMIID(void);
void cmd_SetDO(void);
void cmd_GetDO(void);

void cmdMotorStep();
void cmdMotorMoveTo(void);
void cmdMotorMoveToStopPin(void);
void cmdMotorFreq(void);
void cmdMotorSetRotateMode(void);
void cmdMotorStop(void);
void cmdMotorAccelerate(void);
void cmdMotorSlowdown(void);
void cmdMotorSlowdownStop(void);
void cmdMotorGetSetFreq(void);
void cmdMotorSetRPM(void);
void cmdSetStopPin(void);
void cmdSetStopGpioPin(void);
void cmdSetSlowdownPin(void);
void cmdSetSlowdownStopPin(void);
void cmdMotorSpeed();
void cmdMotorInfo();
void cmdRunMode();
void cmdgetsetPosition();
void setPWM(void);
void cmdInput();
void cmdOutput(void);
void cmdSearchSensor(void);
void cmdgoHome();
void cmdInit(void);
void cmdHMITest(void);
void cmd_SetAcctime();

#endif //_USER_COMMAND_H_
