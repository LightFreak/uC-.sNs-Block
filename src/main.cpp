#include <Arduino.h>
#include <Streaming.h>
#include <MCP23017.h>
#include <FastLED.h>

MCP23017 MCP(0x20);

// Definitions
#define Data_Pin 6
#define NumberOfLED 4
CRGB leds[NumberOfLED];

//Sensors Pins: Arduino
#define reed_Nord A0 
#define rlt_Nord A1
#define reed_Sud A2
#define rlt_Sud A3

// Fahrstrom - Out / Pins: Arduino
#define relais_Nord 9
#define relais_Sud 10

//Belegtmeldungen 
boolean belegt_Nord = false;
boolean belegt_Sud = false;
boolean freieFahrt_Nord = true;
boolean freieFahrt_Sud = true;

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
  MCP.write1(FF_Sig_Nord_TX,HIGH);
  MCP.write1(FF_Sig_Sud_TX,HIGH);
  MCP.write1(FM_Sig_Nord_TX,HIGH);
  MCP.write1(FM_Sig_Sud_TX,HIGH);
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
      Serial << "Fahrstrom Nord An" << endl;
      belegt_Nord = false;
      MCP.write1(FM_Sig_Nord_TX,HIGH);
      MCP.write1(FF_Sig_Nord_TX,HIGH);
      if(MCP.read1(FF_Sig_Nord_RX))
      {
        Serial << "Signal Nord: Achtung" << endl;
        //Signal Achtung
      }
      else
      {
        Serial << "Signal Nord: Freie Fahrt" << endl;
        //Signal Freie Fahrt.
      }
    }
    else // Halt
    {
      Serial << "Signal Nord: Halt" << endl;
    }
  }
  else if(side == 'S') // Südseite
  {
    if(state) // Frei oder Warnung
    {
      digitalWrite(relais_Sud,HIGH);
      belegt_Sud = false;
      MCP.write1(FM_Sig_Sud_TX,HIGH);
      MCP.write1(FF_Sig_Sud_TX,HIGH);
      Serial << "Fahrstrom Süd An" << endl;
      if(MCP.read1(FF_Sig_Sud_RX))
      {
        Serial << "Signal Süd: Achtung" << endl;  
        //Signal Achtung
      }
      else
      {
        Serial << "Signal Süd: Freie Fahrt" << endl;  
        //Signal Freie Fahrt.
      }
    }
    else // Halt
    {
      Serial << "Signal Süd: Halt" << endl;  
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial << "Begin Setup.." << endl;
  FastLED.addLeds<NEOPIXEL, Data_Pin>(leds, NumberOfLED);
  Wire.begin();
  MCP.begin();
  Init_Pins_Communications();
  Init_Pins_Inputs();
  Init_Pins_Outputs();
  for (size_t i = 0; i < NumberOfLED; i++)
  {
    leds[i] = CRGB::Red;  /* code */
  }
  
  delay(500);

  for (size_t i = 0; i < NumberOfLED; i++)
  {
    leds[i] = CRGB::Yellow;  /* code */
  }
  delay(500);

  for (size_t i = 0; i < NumberOfLED; i++)
  {
    leds[i] = CRGB::Blue;  /* code */
  }
  delay(500);
  
  for (size_t i = 0; i < NumberOfLED; i++)
  {
    leds[i] = CRGB::Green;  /* code */
  }
  delay(500);

  for (size_t i = 0; i < NumberOfLED; i++)
  {
    leds[i] = CRGB::Black;  /* code */
  }

  
  Serial << "Setup done" << endl;
}

void loop() 
{
  /**
   * @brief Empfängt Freimeldung auf dem Nordgleis
   * Ablauf:
   * - Signal auf Grün Stellen
   * - Fahrstrom AN
   * - FM Nord TX auf Active
   * - FF Nord TX auf Active
   */
  if (!MCP.read1(FM_Sig_Nord_RX))
  {
    set_signal('N',true);
    Serial << "Frei Meldung Nord RX" << endl;
        
  }

   /**
   * @brief Empfängt Freimeldung auf dem Südgleis
   * Ablauf:
   * - Signal auf Grün Stellen
   * - Fahrstrom AN
   * - FM Sud TX auf Active
   * - FF Sud TX auf Active
   */
  if(!MCP.read1(FM_Sig_Sud_RX))
  {
    set_signal('S',true);
    Serial << "FM_Sig_Sud_RX > 1" << endl; 
  }

  /**
   * @brief Empfängt Freie Fahrt auf dem Nordgleis
   * Active & nicht Belegt: Grün
   * Low & nicht Belegt: Achtung.
   */
  if(MCP.read1(FF_Sig_Nord_RX) != freieFahrt_Nord)
  {
    Serial << "FF_Sig_Nord_RX > " << MCP.read1(FF_Sig_Sud_RX) << endl; 
    freieFahrt_Nord = MCP.read1(FF_Sig_Nord_RX);
    if(!belegt_Nord){
      set_signal('N',true);
    }
  }

  /**
   * @brief Empfängt Freie Fahrt auf dem Südgleis
   * Active & nicht Belegt: Grün
   * Low & nicht Belegt: Achtung.
   */
  if(MCP.read1(FF_Sig_Sud_RX) != freieFahrt_Sud)
  {
    Serial << "FF_Sig_Sud_RX > " << MCP.read1(FF_Sig_Sud_RX) << endl; 
    freieFahrt_Sud = MCP.read1(FF_Sig_Sud_RX);
    if(!belegt_Sud){
      set_signal('S',true);
    }
  }

  /**
   * @brief Reed auf Nordgleis löst aus.
   * Ablauf:
   * - Belegung Speichern
   * - Signal Rot
   * - FF Nord Tx auf Inactive
   */
  if(!digitalRead(reed_Nord))
  {
    belegt_Nord = true;
    set_signal('N',false);
    MCP.write1(FF_Sig_Nord_TX,LOW);
    Serial << "reed Nord > 1" << endl; 
  }
  
  /**
   * @brief Reed auf Südgleis löst aus.
   * Ablauf:
   * - Belegung Speichern
   * - Signal Rot
   * - FF Süd Tx auf Inactive
   */
  if(!digitalRead(reed_Sud))
  {
    belegt_Sud = true;
    set_signal('S',false);
    MCP.write1(FF_Sig_Sud_TX,LOW);
    Serial << "Reed sud > 1" << endl; 
  }

  /**
   * @brief RLT Nord wird bei Belegung ausgelöst.
   * Ablauf:
   * - Fahrstrom aus.
   */
  if(!digitalRead(rlt_Nord) && belegt_Nord)
  {
    //set_signal('N',false);
    digitalWrite(relais_Nord,LOW);
    Serial << "Fahrstrom Nord Aus" << endl;
    //MCP.write1(FM_Sig_Nord_TX,LOW);
  }

  /**
   * @brief RLT Süd wird bei Belegung ausgelöst.
   * Ablauf:
   * - Fahrstrom aus.
   */
  if(!digitalRead(rlt_Sud) && belegt_Sud)
  {
    //set_signal('S',false);
    digitalWrite(relais_Sud,LOW);
    Serial << "Fahrstrom Süd Aus" << endl;
    //MCP.write1(FM_Sig_Sud_TX,LOW);
  }
}
