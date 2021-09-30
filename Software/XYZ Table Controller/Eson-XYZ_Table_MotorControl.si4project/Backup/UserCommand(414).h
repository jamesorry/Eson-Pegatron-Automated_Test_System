#ifndef _USER_COMMAND_H_
#define	_USER_COMMAND_H_

#include "Arduino.h"

typedef struct __CMD {
  const char* cmd;
  void (*func)(void);
} CMD, *PCMD;

void resetArduino(void);
void getMicros(void);
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

void cmdMotorStep(void);
void cmdMotorMoveTo(void);
void cmdMotorSpeed();
void cmdMotorInfo();
void cmdMotorFreq(void);
void cmdMotorFreqStartUp(void);
void cmdMotorAccelerateTime(void);
void cmdClearEEPROM(void);
void cmd_DemoGoHome(void);
void cmd_DemoMoveCentimeter(void);

void cmd_DemoFOWARD1Centimeter(void);
void cmd_DemoFOWARD5Centimeter(void);
void cmd_DemoFOWARD10Centimeter(void);

void cmd_DemoBACK1Centimeter(void);
void cmd_DemoBACK5Centimeter(void);
void cmd_DemoBACK10Centimeter(void);


void cmdDemoMode(void);
void DemoBackForth();

void UserCommand_Task(void);
void UserCommand_Timer(void);
#endif //_USER_COMMAND_H_
