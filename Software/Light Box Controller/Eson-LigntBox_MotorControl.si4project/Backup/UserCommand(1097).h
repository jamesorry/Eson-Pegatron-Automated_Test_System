#ifndef _USER_COMMAND_H_
#define	_USER_COMMAND_H_

#include "Arduino.h"

typedef struct __CMD {
  const char* cmd;
  void (*func)(void);
} CMD, *PCMD;
void cmdHMIID(void);
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
void cmdRunMode(void);
void showHelp(void);
bool getNextArg(String &arg);

void cmdMotorStep(void);
void cmdMotorMoveTo(void);
void cmdMotorMoveToStopPin(void);
void cmdMotorSpeed();
void cmdMotorInfo();
void cmdMotorFreq(void);
void cmdMotorFreqStartUp(void);
void cmdMotorAccelerateTime(void);
void cmdClearEEPROM(void);
void cmdSearchSensor();
void cmdgetsetPosition(void);
void cmdStationPosition(void);

void UserCommand_Task(void);
void UserCommand_Timer(void);
#endif //_USER_COMMAND_H_
