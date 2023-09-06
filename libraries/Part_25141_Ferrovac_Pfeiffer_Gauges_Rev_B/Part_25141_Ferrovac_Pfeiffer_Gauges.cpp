#include "Arduino.h"
#include "Part_25141_Ferrovac_Pfeiffer_Gauges.h"


ReadGaugePressure::ReadGaugePressure(int pressure_type1, int pressure_medium1, int pressure_unit1)
{
	p_type = pressure_type1;
    p_medium = pressure_medium1;
    p_unit = pressure_unit1;
 }

double PKR_261_360_voltageToPressure(double U){ //return pressure in Pa given a voltage U
	return pow(10. , (1.667*U-9.33));
}

double PKR_261_360_PtoPeff(double P, uint8_t gas){ //returns the effective pressure 
	switch(gas){
		case 0: //N2
			return P* 1.;
			break;
		case 1: //Ar
			return P * 0.8;		
			break;
	}
	
	return 0;
}

double TPR_280_281_PCR_280_voltageToPressure(double U){ //return pressure in Pa given a voltage U
	return pow(10. , (U-3.5));
}

double TPR_280_281_PCR_280_PtoPeff(double P, uint8_t gas){ //returns the effective pressure
	switch(gas){
		case 0: //N2
			return P* 1.;
			break;
		case 1: //Ar
			return P * 1.7;		
			Serial.println("Ar_applied");
			break;
	}
	
	return 0;
}
double pressureUnitConversion(double P, uint8_t unit){ //converts pressure P in passcall to different units
	switch(unit){
		case 0: //mbar
			return P * 0.01;
		case 1: //Torr
			return P / 133.3;
		case 2:
			return P;
	}
	return 0;
}

double ReadGaugePressure::printPressure(int pressure_pin, double pressure_type, double pressure_medium, double pressure_unit){
	//Serial.println("call");
	//Serial.print("pin: ");
	//Serial.println(pressure_pin);
	double U = 0;
	double P = 0;
	
	
	analogReadResolution(12);
	
	if(pressure_pin == 57){// Analog in AD3 
		uint16_t adcIn = analogRead(pressure_pin);
	//	Serial.println(adcIn);
		U = (double)adcIn/4065. * 8.5;
	//	Serial.println(U,5);
	}else if(pressure_pin == 61){// Analog in AD7 
		uint16_t adcIn = analogRead(61);
	//	Serial.println(adcIn);
		U = (double)adcIn/4055. * 10.;
	//	Serial.println(U,5);
	}
	
	switch((int)pressure_type){
		case 0: ////PKR 261
			P = PKR_261_360_voltageToPressure(U);
			P = PKR_261_360_PtoPeff(P,(int)pressure_medium);
			break;
		case 1: //PKR 360
			P = PKR_261_360_voltageToPressure(U);
			P = PKR_261_360_PtoPeff(P,(int)pressure_medium);
			break;
		case 2: //TPR 280
			P = TPR_280_281_PCR_280_voltageToPressure(U);
			P = TPR_280_281_PCR_280_PtoPeff(P,(int)pressure_medium);
			break;
		case 3: //PCR 280
			P = TPR_280_281_PCR_280_voltageToPressure(U);
			P = TPR_280_281_PCR_280_PtoPeff(P,(int)pressure_medium);
			break;
		default:
			P = 0;
	}
	//Serial.println(P,15);

	P = pressureUnitConversion(P,(int)pressure_unit);
	
	
	analogReadResolution(16); //yes this is needed because of reasons ¯\_(ツ)_/¯ -HOTO
	//Serial.println("return");
	return P;

}

/*
if(p_pin == 61){
	p_pin = A3;
}
if(p_pin == 62){
	p_pin = A4;
}	
if(p_pin == 63){
	p_pin = A5;
}	
if(p_pin == 64){
	p_pin = A6;
}	
if(p_pin == 65){
	p_pin = A7;
}		
	p_type = pressure_type;
    p_medium = pressure_medium;
    p_unit = pressure_unit;
	
	double in_min;
    double in_max;
    double out_min;
    double out_max;
	
//Mit 16Bit Auflösung -> 1 DIGIT -> 0.0001 mbar

// Schritt 1: 0V - 3.3V ->  0 - 65536
// Schritt 2: Eingang nach Spannungsteiler -> Minimum - 0.847540 V (entspricht 2.2 V) bis Maximum - 3.27459 V (entspricht 8.5 V)
// Schritt 3: 0.847540 - 16831.6307394    3.27459 - 65031.372800
// Schritt 4: Skalierung des Analogwertes in Volt-Realer-Wert



 
//*****************************************PKR 261***********************************************************************************************************
  if(p_type == 0){//PKR 261
  
//---------------A3-------------------------
  if(p_pin == A3){
    _analog_value = analogRead(p_pin);
    in_min = 13771.35022;
    in_max = 65536;
    out_min = 1.8000000;
    out_max = 8.5659574467;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
   if(p_medium == 0){//Air
     
     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
	 Serial.println(Voltage_U,10);
	Serial.println(pressureValue,15);

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }

   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10,((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }


   }
   
  }
//-----------------------------------------------------------------
//---------------A4-------------------------
if(p_pin == A4){
    _analog_value = analogRead(p_pin);
     in_min = 11679.68317;
     in_max = 55802.93069;
     out_min = 1.8000000;
     out_max = 8.6;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	
   if(p_medium == 0){//Air
     
     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
	 

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }

   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10,((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }


   }
   
  }
//---------------------------------------------------------------------------------------
  
//---------------A5-------------------------
if(p_pin == A5){
    _analog_value = analogRead(p_pin);
     in_min = 11679.68317;
     in_max = 55802.93069;
     out_min = 1.8000000;
     out_max = 8.6;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
   if(p_medium == 0){//Air
     
     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
	 

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }

   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10,((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }


   }
   
  }
//--------------------------------------------------------------------------------------- 

//---------------A6-------------------------
if(p_pin == A6){
    _analog_value = analogRead(p_pin);
     in_min = 11679.68317;
     in_max = 55802.93069;
     out_min = 1.8000000;
     out_max = 8.6;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
   if(p_medium == 0){//Air
     
     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
	 

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }

   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10,((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }


   }
   
  }
//---------------------------------------------------------------------------------------

//---------------A7-------------------------
if(p_pin == A7){
    _analog_value = analogRead(p_pin);
     in_min = 11679.68317;
     in_max = 55802.93069;
     out_min = 1.8000000;
     out_max = 8.6;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
   if(p_medium == 0){//Air
     
     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
	 

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }

   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10,((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }


   }
   
  }
//---------------------------------------------------------------------------------------
  }
//***********************************************************************************************************************************************************

//*****************************************PKR 360***********************************************************************************************************
else if(p_type == 1){//PKR 360

//---------------A3-------------------------
  if(p_pin == A3){
	_analog_value = analogRead(p_pin);
	
	//Serial.print("ADC: ");
	//Serial.println(_analog_value,0);
	
    in_min = 15301.50025;
    in_max = 65536;
    out_min = 2.0000000;
    out_max = 8.5659574467;
	
    //Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	
		//Changed: 24.05.23 by HOTO
			//I don't understand the math in this part... changed it to the regular 16-bit math to get voltage.
			
	
	 double Voltage_U = (double)_analog_value / (double)65535.00 * (double)8.5;
	
	
	//Serial.print("ADC [V]: ");
	//Serial.println(Voltage_U,4);

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10.0, ((1.667 * Voltage_U) - 11.33));
		
		//Serial.print("P [mbar]: ");
		//Serial.println(pressureValue,5);

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }




   }

   else if(p_medium == 1){//Argon


     if(p_unit == 0){//mbar
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }



   }
  }
//---------------------------------------------------------------------------------------

//---------------A4-------------------------
  if(p_pin == A4){
	_analog_value = analogRead(p_pin);
    in_min = 12977.42574;
    in_max = 55802.93069;
    out_min = 2.0000000;
    out_max = 8.6;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }




   }

   else if(p_medium == 1){//Argon


     if(p_unit == 0){//mbar
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }



   }
  }
//---------------------------------------------------------------------------------------

//---------------A5-------------------------
  if(p_pin == A5){
	_analog_value = analogRead(p_pin);
    in_min = 12977.42574;
    in_max = 55802.93069;
    out_min = 2.0000000;
    out_max = 8.6;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }




   }

   else if(p_medium == 1){//Argon


     if(p_unit == 0){//mbar
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }



   }
  }
//---------------------------------------------------------------------------------------

//---------------A6-------------------------
  if(p_pin == A6){
	_analog_value = analogRead(p_pin);
    in_min = 12977.42574;
    in_max = 55802.93069;
    out_min = 2.0000000;
    out_max = 8.6;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }




   }

   else if(p_medium == 1){//Argon


     if(p_unit == 0){//mbar
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }



   }
  }
//---------------------------------------------------------------------------------------

//---------------A7-------------------------
  if(p_pin == A7){
	_analog_value = analogRead(p_pin);
    in_min = 12977.42574;
    in_max = 55802.93069;
    out_min = 2.0000000;
    out_max = 8.6;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     
     
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     }




   }

   else if(p_medium == 1){//Argon


     if(p_unit == 0){//mbar
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.33));
     pressureValue = pressureValue * 0.8;
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, ((1.667 * Voltage_U) - 11.46));
     pressureValue = pressureValue * 0.8;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, ((1.667 * Voltage_U) - 9.33));
     pressureValue = pressureValue * 0.8;

     }



   }
  }
//---------------------------------------------------------------------------------------
}

//***********************************************************************************************************************************************************

//*****************************************TPR 280***********************************************************************************************************

 else if(p_type == 2){//TPR 280
 
 //---------------A3-------------------------
  if(p_pin == A3){
	_analog_value = analogRead(p_pin);
    in_min = 16831.65027;
    in_max = 65031.37605;
    out_min = 2.2000000;
    out_max = 8.5000000;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }

  }
//---------------------------------------------------------------------------------------

 //---------------A4-------------------------
  if(p_pin == A4){
	_analog_value = analogRead(p_pin);
    in_min = 14275.16832;
    in_max = 55154.05941;
    out_min = 2.2000000;
    out_max = 8.5000000;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }

  }
//---------------------------------------------------------------------------------------

 //---------------A5-------------------------
  if(p_pin == A5){
	_analog_value = analogRead(p_pin);
    in_min = 14275.16832;
    in_max = 55154.05941;
    out_min = 2.2000000;
    out_max = 8.5000000;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }

  }
//---------------------------------------------------------------------------------------

 //---------------A6-------------------------
  if(p_pin == A6){
	_analog_value = analogRead(p_pin);
    in_min = 14275.16832;
    in_max = 55154.05941;
    out_min = 2.2000000;
    out_max = 8.5000000;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }

  }
//---------------------------------------------------------------------------------------

 //---------------A7-------------------------
  if(p_pin == A7){
	_analog_value = analogRead(p_pin);
    in_min = 14275.16832;
    in_max = 55154.05941;
    out_min = 2.2000000;
    out_max = 8.5000000;
	
    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }

  }
//---------------------------------------------------------------------------------------

 }

//***********************************************************************************************************************************************************

//*****************************************PCR 280***********************************************************************************************************

 else if(p_type == 3){//PCR 280

 //---------------A3-------------------------
  if(p_pin == A3){
	_analog_value = analogRead(p_pin);
    in_min = 9180.90015;
    in_max = 65536;
    out_min = 1.2000000;
    out_max = 8.5659574467;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }
  }
//---------------------------------------------------------------------------------------


//---------------A5-------------------------
  if(p_pin == A5){
	_analog_value = analogRead(p_pin);
    in_min = 7786.455445;
    in_max = 56322.02772;
    out_min = 1.2000000;
    out_max = 8.68;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }
  }
//---------------------------------------------------------------------------------------

//---------------A6-------------------------
  if(p_pin == A6){
	_analog_value = analogRead(p_pin);
    in_min = 7786.455445;
    in_max = 56322.02772;
    out_min = 1.2000000;
    out_max = 8.68;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }
  }
//---------------------------------------------------------------------------------------

//---------------A7-------------------------
  if(p_pin == A7){
	_analog_value = analogRead(p_pin);
    in_min = 7786.455445;
    in_max = 56322.02772;
    out_min = 1.2000000;
    out_max = 8.68;

    Voltage_U = (_analog_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

   if(p_medium == 0){//N2

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     
     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     
     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     
     }


   }

   else if(p_medium == 1){//Argon

     if(p_unit == 0){//mbar
     pressureValue = pow(10, (Voltage_U - 5.5));
     pressureValue = pressureValue * 1.7;

     }

     else if(p_unit == 1){//Torr
     pressureValue = pow(10, (Voltage_U - 5.625));
     pressureValue = pressureValue * 1.7;

     }


     else if(p_unit == 2){//Pa
     pressureValue = pow(10, (Voltage_U - 3.5));
     pressureValue = pressureValue * 1.7;

     }



   }
  }
//---------------------------------------------------------------------------------------
 }  
  

     return pressureValue;
	
}
*/
