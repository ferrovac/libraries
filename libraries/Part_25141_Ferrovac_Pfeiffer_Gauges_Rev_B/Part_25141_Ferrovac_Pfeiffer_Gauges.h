/*********************************************
by FERROVAC

    Date: 09.03.2023
    Name: Part_25141_Ferrovac_Pfeiffer_Gauges.h
    Following Libraries are needed for Adafruit 2.2" TFT Breakout Card:
      - Adafruit_GFX
      - Adafruit_ILI9341

Revision directory:
- Revision A: 09.03.2023, S.Kovacevic:
              - Created



*********************************************/

#ifndef Part_25141_Ferrovac_Pfeiffer_Gauges_h
#define Part_25141_Ferrovac_Pfeiffer_Gauges_h

#include "Arduino.h"

class ReadGaugePressure
{
  public:
    
  ReadGaugePressure(int pressure_type1, int pressure_medium1, int pressure_unit1);
  double printPressure(int pressure_pin, double pressure_type, double pressure_medium, double pressure_unit);

  private:
  
  double p_type;
  double p_medium;
  double p_unit;
  int p_pin;
  double _analog_value;
  double pressureValue;
  double in_min;
  double in_max;
  double out_min;
  double out_max;
  double Voltage_U;
};

#endif
