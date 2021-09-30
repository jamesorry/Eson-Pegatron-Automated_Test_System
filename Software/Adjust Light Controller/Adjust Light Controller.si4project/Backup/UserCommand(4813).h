#ifndef _USER_COMMAND_H_
#define	_USER_COMMAND_H_

#include "Arduino.h"

typedef struct __CMD {
  const char* cmd;
  void (*func)(void);
} CMD, *PCMD;

void resetArduino(void);
void getMicros(void);
void cmd_HMI_ID(void);
void getAdc(void);
void getGpio(void);
void setGpio(void);
void echoOn(void);
void echoOff(void);
void cmd_CodeVer(void);
void showHelp(void);
bool getNextArg(String &arg);
void cmd_UpdateEEPROM(void);
void cmd_ClearEEPROM(void);
void cmd_Output(void);
void cmd_SetADC(void);
void cmd_SetVoltage(void);
void cmd_Maindata(void);

void UserCommand_Task(void);
void UserCommand_Timer(void);
#endif //_USER_COMMAND_H_
