/*
AUTHOR:       Tobias Hofmänner
DATE:         29.09.2023
USAGE:        TODO
DESCRIPTION:  TODO
TODO:       
RECOURCES:  TODO  
*/

#ifndef LscComponents_H
#define LscComponents_H

#include <Arduino.h>
#include "LscError.h"
#include <vector>

//A list of all available physical units
struct Units{
    //List of all available temperature units
    struct Temperature {
        enum struct Unit{
            //Kelvin
            K, 
            //Celsius
            C, 
            //Fahrenheit
            F 
        };
        static std::vector<String> getOptions(){
            return {"Kelvin", "Celsius", "Fahrenheit"};
        }

    };
    struct Pressure{
        //List of all available pressure units
        enum struct Unit{
            //mbar
            mBar,
            //Pascal
            Pa,
            //Torr mmHg
            Torr,
            //Pounds per square inch
            psi,
            //Atmospheres
            atm
        };
        static std::vector<String> getOptions(){
            return {"mbar", "Torr mmHg", "Pounds Per Square Inch", "Atmospheres"};
        }
    };
};


class TemperatureSensor{
    private:
    public:
        
        

};


#endif