#include <avr/wdt.h>
#include "SoftwareSerial.h"
#include "UserCommand.h"
#include "EEPROM_Function.h"
#include "MainProcess.h"
#include <Adafruit_MCP4725.h>
#include "HMI_Command.h"

extern HMI_Command *hmi_cmd;

extern Adafruit_MCP4725 MCP4725;
extern HardwareSerial *cmd_port;
extern MainDataStruct maindata;
extern RuntimeStatus runtimedata;

CMD g_cmdFunc[] = {
//在這新增function name 以及所呼叫的function
	{"adc", getAdc},
	{"getgpio", getGpio},
	{"setgpio", setGpio},
	{"reset", resetArduino},
	{"getmicros", getMicros},
	{"ver", cmd_CodeVer},
	{"echoon", echoOn},
	{"echooff", echoOff},
    {"SD", cmd_UpdateEEPROM},
    {"CD", cmd_ClearEEPROM},
    {"SD", cmd_Maindata},
    {"Output", cmd_Output},
    {"MCP4725ADC", cmd_SetMCP4725_ADC},
    {"SetVoltage", cmd_SetVoltage},
    {"HMI_ID", cmd_HMI_ID},
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

void resetArduino(void)
{
	wdt_enable(WDTO_500MS);
	while (1);
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

void cmd_HMI_ID(void)
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
void cmd_UpdateEEPROM(void)
{
    runtimedata.UpdateEEPROM = true;
}
void cmd_ClearEEPROM(void)
{
    Clear_EEPROM();
}

void cmd_Output(void)
{
    String arg1, arg2;
    int value, hl;
    
    if (!getNextArg(arg1))
    {
      cmd_port->println("No parameter 1");
      return;
    }
    if (!getNextArg(arg2))
    {
      cmd_port->println("No parameter 2.");
      return;
    }
    value = arg1.toInt();
    if(value > 4 || value < 0) return;
    hl = arg2.toInt();
    if(hl == 0) hl = 0;
    else hl = 1;
    digitalWrite(ADC_OutputPin[value], hl);
}
void cmd_SetMCP4725_ADC(void)
{
    String arg1;
    int value;
    
    if (!getNextArg(arg1))
    {
      cmd_port->println("No parameter");
      return;
    }
    value = arg1.toInt();
    if (value > 4095)
        value = 4095;
    
    MCP4725.setVoltage(value, false);
    Serial.print("MCP4725 ADC= " + String(value));
}
void cmd_SetVoltage(void)
{
    String arg1;
    int value;
    float val_kk, OUT_K;
    if (!getNextArg(arg1))
    {
      cmd_port->println("No parameter");
      return;
    }
    value = arg1.toInt();
    if(value > 100)
        value = 100;
    maindata.Voltage_Last = value;
    val_kk=value/10;
    Serial.print("OUT V= " + String(val_kk));  
    OUT_K = value*40.96;
    if (OUT_K > 4095)
    {
        OUT_K=4095;
    }
    Serial.print("OUT ADC= " + String(OUT_K));
    MCP4725.setVoltage(OUT_K, false);
    WRITE_EEPROM();  // 記憶 KEEP_DATA.ADC_Last
    Serial.println("DONE ");
}


void cmd_Maindata(void)
{
    cmd_port->println("HMI_ID:" + String(maindata.HMI_ID));
    cmd_port->println("Voltage_Last:" + String(maindata.Voltage_Last));
}

uint8_t UserCommWorkindex = 0;

uint32_t UserCommandTimeCnt = 0;

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
      if((millis() - UserCommandTimeCnt) > 50)
        UserCommWorkindex ++;
      break;
    }
    case 2:
    {
      if ( incomingBytes = cmd_port->available() )
      {

      cmd_port->println("cmd_port datalen: " + String(incomingBytes));

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
          cmd_port->print("ARDUINO>");
          break;
        }
        else if (i == (sizeof(g_cmdFunc) / sizeof(CMD) - 1))
        {
          cmd_port->println("bad command !!");
          cmd_port->print("ARDUINO>");
        }
        }
        *g_inputBuffer = "";
      }
      else
      {
        cmd_port->print("ARDUINO>");
      }
      UserCommWorkindex = 0;
      break;
    }
  }

  }
}

