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




class BaseComponent;
struct BaseExposedState;

        class ComponentTracker{
            private:
                ComponentTracker() {}
            public:
                std::vector<BaseComponent*> components;
                std::vector<std::pair<BaseComponent*, BaseExposedState*>> states;

                static ComponentTracker& getInstance() {
                    static ComponentTracker instance;  
                    return instance;
                }
                //Returns a vector with pointers to all registered components. 
                std::vector<BaseComponent*> getComponets(){
                    return ComponentTracker::getInstance().components;
                }
                //Registers the given component with the ComponentTracker
                void registerComponent(BaseComponent* component){
                    ComponentTracker::getInstance().components.push_back(component);
                }
                void registerState(BaseExposedState* State){
                    ComponentTracker::getInstance().states.push_back({ComponentTracker::getInstance().components.back(), State});
                }
        };


template<typename T>
struct Selection{
    private:
        std::vector<std::pair<T, const char*>> _selection ;
    public:
        Selection(std::vector<std::pair<T, const char*>> selection) : _selection(selection){
        }
        int getIndexByValue(T value){
            uint16_t counter = 0;
            for(std::pair<T, const char*> &pair : _selection){
                if(pair.first == value){
                    return counter;
                }
                counter++;
            }
            return -1;
        }
        const char* getDescriptionByValue(T value){
            for(std::pair<T, const char*> &pair : _selection){
                if(pair.first == value){
                    return pair.second;
                }
            }
            return "not found";
        }

        std::vector<String> getOptions(){
            std::vector<const char*> ret;
            for(std::pair<T, const char*> &pair : _selection){
                ret.push_back(pair.second);
            }
            return ret;
        }
};
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

namespace Units{

    enum struct Temperature{
        K,
        C,
        F
    };
    enum struct Pressure{
        // mbar
        Pa,
        // Pascal
        mBar,
        // Torr mmHg
        Torr,
        // Pounds Per Square Inch
        psi,
        // Atmospheres
        atm
    };
}

class BaseUnit{
    public:
        virtual double convertFromSI(double SI) = 0;
        virtual String getSuffix() = 0;
};

template<typename T>
class Unit;

template<>
class Unit<Units::Temperature> : BaseUnit{
    private:
        
    public:
        Selection<Units::Temperature> selection;
        Units::Temperature unitType;
        Unit(Units::Temperature unitPtr) : unitType(unitPtr), selection({{Units::Temperature::K, "Kelvin"},{Units::Temperature::C, "Celsius"},{Units::Temperature::F, "Fahrenheit"}}){
            //Fill the selection 
        }

        double convertFromSI(double SI) override{
            switch (unitType) {
                case Units::Temperature::K :
                    return SI; 
                case Units::Temperature::C:
                    return SI - 273.15; 
                case Units::Temperature::F:
                    return SI * (9./5.) - 459.67; 
            }
        }
        String getSuffix() override{
            switch (unitType) {
                case Units::Temperature::K :
                    return "K"; 
                case Units::Temperature::C:
                    return "°C"; 
                case Units::Temperature::F:
                    return "°F"; 
            }
        }
};
template<>
class Unit<Units::Pressure> : BaseUnit{
    private:
        
    public:
        Selection<Units::Pressure> selection;
        Units::Pressure unitType;
        Unit(Units::Pressure unitPtr) : unitType(unitPtr), selection({{Units::Pressure::Pa, "Pascal"},{Units::Pressure::mBar, "mbar"},{Units::Pressure::Torr, "Torr"},{Units::Pressure::psi, "psi"},{Units::Pressure::atm, "atm"}}){
            //Fill the selection 
        }

        double convertFromSI(double SI) override{
            switch (unitType) {
                case Units::Pressure::Pa:
                    return SI; 
                case Units::Pressure::mBar:
                    return SI * 0.01; 
                case Units::Pressure::Torr:
                    return SI * 0.0007500617;
                case Units::Pressure::psi:
                    return SI * 0.0001450377;
                case Units::Pressure::atm:
                    return SI *0.00000986923; 
            }
        }
        String getSuffix() override{
            switch (unitType) {
                case Units::Pressure::Pa:
                    return "Pa"; 
                case Units::Pressure::mBar:
                    return "mbar"; 
                case Units::Pressure::Torr:
                    return "Torr";
                case Units::Pressure::psi:
                    return "psi";
                case Units::Pressure::atm:
                    return "atm"; 
            }
        }
};







//The class privides a base that all components should be derived from to enforce design pattern. This class is purely virtual.
class BaseComponent{
    public:
        virtual void update() = 0;

        BaseComponent(){
            ComponentTracker::getInstance().registerComponent(this);
        }
};

/*
IDEA:
components are definded by states-> 
create a state struct like a type exposed state with pointer to state variable
if you want to change the state change the member variables directly then call update()
update is responsible to:
    1. read the external changes and update the object
    2. if the object has been changed by the user/prog take apropriate acction such that the external 
        hardware reflects the changed state. 
Create a state tracker class -> add exposed states to the tracker. -> easy menu creation and external com=)

Put selections in theire own class. keep the struct but also provide something like a dict that contains the enum
with the explanation string -> makes it easy to get index.
*/



enum struct ExposedStateType{
    ReadOnly,
    ReadWrite,
    ReadWriteRanged,
    ReadWriteSelection
};


struct BaseExposedState{
    private:
        
    public:
        const String stateName;
        BaseExposedState(String StateName) : stateName(StateName) {
            ComponentTracker::getInstance().registerState(this);
        }
        virtual ~BaseExposedState(){

        }
};

template <ExposedStateType StateType, typename T>
struct ExposedState;

template <typename T>
//is the same as ReadWrite will check in menu if setting state is allowed
struct ExposedState<ExposedStateType::ReadOnly, T> : BaseExposedState {
    T* state;
    ExposedState(String StateName,T* State): BaseExposedState(StateName), state(State) {
    }
};

template <typename T>
struct ExposedState<ExposedStateType::ReadWrite, T> : BaseExposedState {
    T* state;
    ExposedState(String StateName,T* State): BaseExposedState(StateName), state(State){
    }
};

template <typename T>
struct ExposedState<ExposedStateType::ReadWriteRanged, T> : BaseExposedState {
    //static_assert if T is orderable
    T* state;
    T minState;
    T maxState;
    T stepState;
    ExposedState(String StateName,T* State, T MinState, T MaxState, T stepState): BaseExposedState(StateName), state(State), minState(MinState), maxState(MaxState), stepState(stepState){
    }
};
template <typename T>
struct ExposedState<ExposedStateType::ReadWriteSelection, T> : BaseExposedState {
    T* state;
    Selection<T>& _selection;
    ExposedState(String StateName,T* State, Selection<T>& selection): BaseExposedState(StateName), state(State), _selection(selection){}

};






//Contains a list of all available system components
class Components{
    public:
        class test : public BaseComponent{
            public:
                test(){
                    ComponentTracker::getInstance().registerComponent(this);

                }
                void update() override{

                }
                void reg(){
                    
                }
        };
        //Class that represents a TP100 temperature sensor
        class TemperatureSensor : BaseComponent {
            private:
                String componentName;
                AnalogInPt100 &analogInPt100;
                double temperature;
                ExposedState<ExposedStateType::ReadOnly, double> temeraturePtr;
                Unit<Units::Temperature> displayUnit;
                ExposedState<ExposedStateType::ReadWriteSelection, Units::Temperature> displayUnitPtr;
                

            public:
                TemperatureSensor(AnalogInPt100 &analogInPt100, String componentName = "genericTemperatureSensor") : analogInPt100(analogInPt100), componentName(componentName), temperature(0), temeraturePtr("Temp",&temperature), displayUnit(Units::Temperature::C), displayUnitPtr("dispUnit", &(displayUnit.unitType), displayUnit.selection){
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
                    return "Error"; 
                }
                //All calculations are done in SI units. In the case of temperature in Kelvin. But when the teperature is requested as string, it will be converted to the unit set here
                void setDisplayUnit(Units::Temperature unit){
                    displayUnit.unitType = unit;
                }

                //Reads the current temperature and updates the internal state
                void update() override {
                    temperature = analogInPt100.getTemperature();
                }

                //Retuns the component type
                String const getComponentType()   {
                    return "TemperatureSensor";
                }
                //Returns the component Name
                String const getComponentName() {
                    return componentName;
                }
                //Sets the component Name. This name is used for logging and ui purposes
                void setComponentName(const String& name) {
                    componentName = name;
                }
        };
        /*

        //Represents a pressure gauge
        class PressureGauge : BaseComponent {
            public:
                //Collection of all available gauge types
                enum struct GaugeTypes{
                    //PKR (Pirani/cold cathode gauge)
                    PKR,
                    //TPR (ActiveLine Pirani gauge)
                    TPR
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
        */   
       
};




#endif