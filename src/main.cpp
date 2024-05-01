#include <Arduino.h>
#include <Streaming.h>
#include <MCP23017.h>

MCP23017 MCP(0x27);

// Definitions

//Sensors Pins: Arduino
#define reed_Nord 3 
#define rlt_Nord 4
#define reed_Sud 5
#define rlt_Sud 6

// Fahrstrom - Out / Pins: Arduino
#define relais_Nord 9
#define relais_Sud 10

//Belegtmeldungen 
boolean belegt_Nord = false;
boolean belegt_Sud = false;

//Array's
boolean inputs[6];
boolean inputs_old[6];

// Block Kommunikation (RX - Empfangen (4-7) / TX - Senden (0-3)) Pins: MCP23017 / PCF8574 zum Testen
#define FF_Sig_Nord_RX 4
#define FF_Sig_Sud_RX 5
#define FM_Sig_Nord_RX 6
#define FM_Sig_Sud_RX 7

#define FF_Sig_Nord_TX 0
#define FF_Sig_Sud_TX 1
#define FM_Sig_Nord_TX 2
#define FM_Sig_Sud_TX 3

// put function declarations here:
void Init_Pins_Communications(void)
{
  MCP.pinMode1(FF_Sig_Nord_RX,INPUT);
  MCP.pinMode1(FF_Sig_Sud_RX,INPUT);
  MCP.pinMode1(FM_Sig_Nord_RX,INPUT);
  MCP.pinMode1(FM_Sig_Sud_RX,INPUT);
  MCP.pinMode1(FF_Sig_Nord_TX,OUTPUT);
  MCP.pinMode1(FF_Sig_Sud_TX,OUTPUT);
  MCP.pinMode1(FM_Sig_Nord_TX,OUTPUT);
  MCP.pinMode1(FM_Sig_Sud_TX,OUTPUT);
}

void Init_Pins_Outputs(void)
{
  pinMode(relais_Nord,OUTPUT);
  pinMode(relais_Sud,OUTPUT);
}

void Init_Pins_Inputs(void) 
{
  pinMode(reed_Nord,INPUT_PULLUP);
  pinMode(rlt_Nord,INPUT_PULLUP);
  pinMode(FF_Sig_Nord_RX,INPUT_PULLUP);

  pinMode(reed_Sud,INPUT_PULLUP);
  pinMode(rlt_Sud,INPUT_PULLUP);
  pinMode(FF_Sig_Sud_RX,INPUT_PULLUP);
}

void set_signal(char side, boolean state)
{
  if(side == 'N') // Nordseite
  {
    if(state) // Frei oder Warnung
    {
      digitalWrite(relais_Nord,HIGH);
      belegt_Nord = false;
      if(MCP.read1(FF_Sig_Nord_RX))
      {
        //Signal Achtung
      }
      else
      {
        //Signal Freie Fahrt.
      }
    }
    else // Halt
    {

    }
  }
  else if(side == 'S') // Südseite
  {
    if(state) // Frei oder Warnung
    {
      digitalWrite(relais_Sud,HIGH);
      belegt_Sud = false;
      if(MCP.read1(FF_Sig_Sud_RX))
      {
        //Signal Achtung
      }
      else
      {
        //Signal Freie Fahrt.
      }
    }
    else // Halt
    {
      
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial << "Begin Setup.." << endl;
  Wire.begin();
  MCP.begin();
  Init_Pins_Communications();
  Init_Pins_Inputs();
  Init_Pins_Outputs();
  
  Serial << "Setup done" << endl;
}

void loop() 
{
  if (MCP.read1(FM_Sig_Nord_RX))
  {
    set_signal('N',true);
  }

  if(MCP.read1(FM_Sig_Sud_RX))
  {
    set_signal('S',true);
  }

  if(digitalRead(reed_Nord))
  {
    digitalWrite(FF_Sig_Nord_TX,HIGH);
    belegt_Nord = true;
    set_signal('N',false);
    MCP.write1(FF_Sig_Nord_TX,HIGH);
  }
  
  if(digitalRead(reed_Sud))
  {
    digitalWrite(FF_Sig_Sud_TX,HIGH);
    belegt_Sud = true;
    set_signal('S',false);
    MCP.write1(FF_Sig_Sud_TX,HIGH);
  }

  if(digitalRead(rlt_Nord) && belegt_Nord)
  {
    set_signal('N',false);
    digitalWrite(relais_Nord,LOW);
    MCP.write1(FM_Sig_Nord_TX,LOW);
  }

  if(digitalRead(rlt_Sud) && belegt_Sud)
  {
    set_signal('S',false);
    digitalWrite(relais_Sud,LOW);
    MCP.write1(FM_Sig_Sud_TX,LOW);
  }
}
