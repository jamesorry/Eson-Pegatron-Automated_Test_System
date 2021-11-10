#ifndef _USER_COMMAND_H_
#define	_USER_COMMAND_H_

#include "Arduino.h"

typedef struct __CMD {
  const char* cmd;
  void (*func)(void);
} CMD, *PCMD;

void resetArduino(void);
void getMicros(void);
void cmdHMIID(void);
void cmd_Maindata(void);
void cmd_UpdateEEPROM(void);
void cmd_ClearEEPROM(void);
void getOnDuration(void);
void resetMicrosTimerCount(void);
void getAdc(void);
void getGpio(void);
void setGpio(void);
void echoOn(void);
void echoOff(void);
void cmd_CodeVer(void);
void showHelp(void);
bool getNextArg(String &arg);

void cmdRunMode(void);
void CmdSendTest();
void cmdMotorStep(void);
void cmdMotorMoveTo(void);
void cmdMotorFreq(void);
void cmdMotorStop(void);
void cmdMotorAccelerate(void);
void cmdMotorSlowdown(void);
void cmdMotorSlowdownStop(void);
void cmdMotorGetFreq(void);
void cmdgetsetPosition(void);
void cmdSearchSensor();
void cmd_Offset(void);
void UserCommand_Task(void);
void UserCommand_Timer(void);

#endif //_USER_COMMAND_H_

