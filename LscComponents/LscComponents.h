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
#include <vector>
#include "LscHardwareAbstraction.h"


    //---- UNITS EXPLANATION ----
/*
    Generally all calculations should be done in SI units i.e. whenever a value that represents a pysical unit is returned by a function it HAS
    to be in SI units. Whenever a function returns a value that represents a physical unit as string, the user should be able to select the unit
    from a list. 
    Every Unit struct should contain the following:
    -enum struct Unit
        This struct should contain a list of all available units. The SI unit should always be the first entry.
    -struct Conversions
        For every unit defined, one should implement a function to convert form the SI unit to the others.
    -function getOptions
        A function that returns a string vector of all units defined above. The vector has to be in the same order as the enum struct.
        This function is important for UI automation. As an exemple if we want to create a menue that lets the user set the display unit
        we need a way to figure out what units are available (i did not figure out a way of doing this only with the struct)
    -function getSuffix
        This is a helper function for printing values as String. It should return the string that has to be appended at the end of the value string.
*/
    //---- END UNITS EXPLANATION ----

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
        //Provides a collection of conversion funcions
        struct Conversions{
            //Converts Kelvin to Celsius
            static double K_C(double K){
                return K -273.15;
            }
            //Converts Kelvin to Fahrenheit
            static double K_F(double K){
                return K * (9./5.) - 459.67;
            }
        };
        //Returns a list of all available temperature units
        static std::vector<String> getOptions() {
            return {"Kelvin", "Celsius", "Fahrenheit"};
        }
        //Returns the unit given an index, carefull there is no bound check on the index!
        static Unit getUnitByIndex(int index) {
            return static_cast<Units::Temperature::Unit>(index);
        }
        //Retruns a string that represents the unit suffix
        static String getSuffix(Unit unit){
            const String suffix[]= {"K","°C", "°F"};
            return suffix[static_cast<int>(unit)];
        }
    };
    //List of all available pressure units
    struct Pressure{
        enum struct Unit{
            //mbar
            mBar,
            //Pascal
            Pa,
            //Torr mmHg
            Torr,
            //Pounds Per Square Inch
            psi,
            //Atmospheres
            atm
        };
        //Provides a collection of conversion funcions
        struct Conversions{
            //Converts Pascal to mbar
            static double Pa_mbar(double Pa){
                return Pa * 0.01;
            }
            //Converts Pascal to Torr
            static double Pa_Torr(double Pa){
                return Pa * 0.0007500617;
            }
            //Converts Pascal to psi
            static double Pa_psi(double Pa){
                return Pa * 0.0001450377;
            }
            //Converts Pascal to atm
            static double Pa_atm(double Pa){
                return Pa *0.00000986923;
            }
        };
        //Returns a list of all available pressure units
        static std::vector<String> getOptions(){
            return {"mbar", "Torr mmHg", "Pounds Per Square Inch", "Atmospheres"};
        }

        //Retruns a string that represents the unit suffix
        static String getSuffix(Unit unit){
            const String suffix[]= {"mbar", "Pa","torr", "psi", "atm"};
            return suffix[static_cast<int>(unit)];
        }
    };
};


//The class privides a base that all components should be derived from to enforce design pattern. This class is purely virtual.
class BaseComponent{
    public:
        virtual void update() = 0;
        virtual String const getComponentType() const = 0;
        virtual String const getComponentName() const = 0;
        virtual void setComponentName(const String& name) = 0;
        virtual std::vector<String> getComponentConfiguration() = 0;
};



//Contains a list of all available system components
class Components{
    public:
        static String flattenOptionsString(std::vector<String> options){
            String optionsString;
                for(String options : Units::Temperature::getOptions()){
                    optionsString += options + ",";
                }
            return optionsString.substring(0,optionsString.length()-1);
        }

        //Class that represents a TP100 temperature sensor
        class TemperatureSensor : BaseComponent {
            private:
                String componentName;
                AnalogInPt100 &analogInPt100;
                double temperature;
                Units::Temperature::Unit displayUnit;
            public:
                //Sets the display unit based on the index of the getOptions() function. CAREFULL there are no checks on the index!
                void setDisplayUnitByIndex(uint16_t index){
                    setDisplayUnit(Units::Temperature::getUnitByIndex(index));
                }
                TemperatureSensor(AnalogInPt100 &analogInPt100, String componentName = "genericTemperatureSensor") : analogInPt100(analogInPt100), componentName(componentName) {
                    temperature = 0;
                    displayUnit = Units::Temperature::Unit::C;
                    ComponentTracker::getInstance().registerComponent(this);
                }
                //Returns the temperature in K
                double getTemperature(){
                    update();
                    return temperature;
                }
                //Returns the temperature as string including the unit suffix. The unit can be set with setDisplayUnit
                String getTeperatureAsString() {
                    update();
                    if(displayUnit == Units::Temperature::Unit::K){
                        return String(temperature) + " " + Units::Temperature::getSuffix(Units::Temperature::Unit::K);
                    }else if(displayUnit == Units::Temperature::Unit::C){
                        return String(Units::Temperature::Conversions::K_C(temperature)) + " " + Units::Temperature::getSuffix(Units::Temperature::Unit::C);
                    }else if(displayUnit == Units::Temperature::Unit::F){
                        return String(Units::Temperature::Conversions::K_F(temperature)) + " " + Units::Temperature::getSuffix(Units::Temperature::Unit::F);
                    }
                    return "Error"; 
                }
                //All calculations are done in SI units. In the case of temperature in Kelvin. But when the teperature is requested as string, it will be converted to the unit set here
                void setDisplayUnit(Units::Temperature::Unit unit){
                    displayUnit = unit;
                }

                //Reads the current temperature and updates the internal state
                void update() override {
                    temperature = analogInPt100.getTemperature();
                }

                //Retuns the component type
                String const getComponentType() const override {
                    return "TemperatureSensor";
                }
                //Returns the component Name
                String const getComponentName() const override{
                    return componentName;
                }
                //Sets the component Name. This name is used for logging and ui purposes
                void setComponentName(const String& name) override{
                    componentName = name;
                }

                std::vector<String> getComponentConfiguration() override {
                    return {    ("Temperature,S,R," + String(getTemperature())), //ID 0
                                ("DisplayUnit,C,S,{"+flattenOptionsString(Units::Temperature::getOptions())+ "}" + String(static_cast<int>(displayUnit))) // ID 1
                            }; 
                }                        
                std::vector<String> getComponentState(){
                    std::vector<String> tempRet;
                    tempRet.push_back(String(getTemperature())); //ID0
                    return tempRet;
                }
                void setComponentState(uint16_t ID, String ComandString){
                    switch (ID){
                        case 0:
                            //error cant set Temperature
                            break;
                        default:
                            //error invalid ID
                            break;
                    }
                }
        };

        //Represents a pressure gauge
        class PressureGauge : BaseComponent {
            public:
                //Collection of all available gauge types
                enum struct GaugeTypes{
                    //PKR (Pirani/cold cathode gauge)
                    PKR,
                    //TPR (ActiveLine Pirani gauge)
                    TPR,
                };
                //Returns a list of all available pressure gauges.
                static std::vector<String> getOptions(){
                    return {"PKR (Pirani/cold cathode gauge)", "TPR (ActiveLine Pirani gauge)"};
                }
                
                PressureGauge(AnalogInBase &analogIn, GaugeTypes gaugeType, String componentName = "genericPressureGauge") : analogIn(analogIn), gaugeType(gaugeType) ,componentName(componentName) {
                    pressure = 0;
                    displayUnit = Units::Pressure::Unit::Pa;
                    ComponentTracker::getInstance().registerComponent(this);
                }



                //Returns the pressure in Pa
                double getPressure(){
                    update();
                    return pressure;
                }
    
                //Returns the pressure as string including the unit suffix. The unit can be set with setDisplayUnit
                String getPressureAsString() {
                    update();
                    if(displayUnit == Units::Pressure::Unit::atm){
                        return String(doubleToSciString(Units::Pressure::Conversions::Pa_atm(pressure))) + " " + Units::Pressure::getSuffix(Units::Pressure::Unit::atm);
                    }else if(displayUnit == Units::Pressure::Unit::mBar){
                        return String(doubleToSciString(Units::Pressure::Conversions::Pa_mbar(pressure))) + " " + Units::Pressure::getSuffix(Units::Pressure::Unit::mBar);
                    }else if(displayUnit == Units::Pressure::Unit::psi){
                        return String(doubleToSciString(Units::Pressure::Conversions::Pa_psi(pressure))) + " " + Units::Pressure::getSuffix(Units::Pressure::Unit::psi);
                    }else if(displayUnit == Units::Pressure::Unit::Torr){
                        return String(doubleToSciString(Units::Pressure::Conversions::Pa_Torr(pressure))) + " " + Units::Pressure::getSuffix(Units::Pressure::Unit::Torr);
                    }else if(displayUnit == Units::Pressure::Unit::Pa){
                        return String(doubleToSciString(pressure)) + " " + Units::Pressure::getSuffix(Units::Pressure::Unit::Pa);
                    }
                    return "Error";  
                }
                //All calculations are done in SI units. In the case of temperature in Kelvin. But when the teperature is requested as string, it will be converted to the unit set here
                void setDisplayUnit(Units::Pressure::Unit unit){
                    displayUnit = unit;
                }
                //Reads the current temperature and updates the internal state
                void update() override {
                    pressure = voltageToPressure(gaugeType, analogIn.getVoltage());
                }

                //Retuns the component type
                String const getComponentType() const override {
                    return "PressureGauge";
                }
                //Returns the component Name
                String const getComponentName() const override{
                    return componentName;
                }
                //Sets the component Name. This name is used for logging and ui purposes
                void setComponentName(const String& name) override{
                    componentName = name;
                }
                //This is probably horrible and one should not do this, but it works remakably well XD... TODO: find a good library to handle this (MathHelper.h is terrible because it uses a global buffer, which makes it not thread save)!!!
                static String doubleToSciString(double value) {
                    if(value == 0) return "0.000E+00";
                    int exponent = static_cast<int>(floor(log10(value)));
                    double mantissa = value / pow(10, exponent);
                    if(exponent >=11) return String(mantissa,2) + "E+" + String(exponent);
                    if(exponent >= 0) return String(mantissa,2) + "E+0" + String(exponent);
                    if(exponent <= -10) return String(mantissa,2) + "E" + String(exponent);
                    if(exponent <  0) return String(mantissa,2) + "E-0" + String(abs(exponent));
                    ERROR_HANDLER.throwError(0x0, "Failed to corretly format number in scientific notation. This has to be an error in LscComponents lib. see doubleToSciString()",SeverityLevel::NORMAL);
                    return String(value,10);
                }

                std::vector<String> getComponentConfiguration() override{
                   return   {   ("Pressure,S,R," + String(getPressure(),20)), //ID 0
                                "DisplayUnit,C,S,{" + flattenOptionsString(Units::Pressure::getOptions()) + "}" + String(static_cast<int>(displayUnit)), // ID 1
                                ("GaugeType,C,S,{" + flattenOptionsString(getOptions()) + "}" + String(static_cast<int>(gaugeType))) //ID 2
                            };
                }  

            private:
                String componentName;
                AnalogInBase &analogIn;
                double pressure;
                Units::Pressure::Unit displayUnit;
                GaugeTypes gaugeType;
                //Converts a given voltage to a pressure value, with respect to the given gauge Type
                static double voltageToPressure(GaugeTypes gauge, double voltage){
                    if(gauge == GaugeTypes::PKR){
                        return voltage;
                        return pow(10., (1.667*voltage-9.33));
                    }else if (gauge == GaugeTypes::TPR){
                        return pow(10., (voltage-3.5));
                    }
                    return 0.;
                }

        };        
        /*
            The ComponentTracker is resposible to keep track of all instances of components. Every component should have:
                ComponentTracker::getInstance().registerComponent(this);
            in the constructor to register the instance with the Tracker. 
            This is usefull for two main reasons:
                1. We can create automatic UI menues for all the component that have been defined.
                2. We can inform external controll/monitoring hardware about which components are available.
            It is also usefull for debugging. The ComponentTracker is designed as a singelton.
        */
        class ComponentTracker{
            private:
                std::vector<BaseComponent*> components;
                ComponentTracker() {}

            public:

                static ComponentTracker& getInstance() {
                    static ComponentTracker instance;  
                    return instance;
                }
                //Returns a vector with pointers to all registered components. 
                std::vector<BaseComponent*> getComponets(){
                    return components;
                }
                //Registers the given component with the ComponentTracker
                void registerComponent(BaseComponent* component){
                    components.push_back(component);

                }
                //Returns a vector with the name strings of all registered components
                std::vector<String> listAllComponentNames(){
                    std::vector<String> componentList;
                    for(BaseComponent* comp : components){
                        componentList.push_back(comp->getComponentName());
                        
                        for(String comStr :comp->getComponentConfiguration()){
                            Serial.println(comStr);
                        }
                    }

                    return componentList;
                }
                
        };
};




#endif