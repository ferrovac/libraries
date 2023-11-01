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
#include <type_traits>




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

/*  How to use the tracker =)
              for(std::pair<BaseComponent*,BaseExposedState*> pair : ComponentTracker::getInstance().states){
                Serial.println("Component: " + String(pair.first->componentName));
                Serial.println("-> State: " + pair.second->stateName);
                if(pair.second->stateType == ExposedStateType::ReadWriteSelection){
                  using T = typename std::remove_reference<decltype(pair.second)>::type;
                  auto myPtr = static_cast< ExposedState<ExposedStateType::ReadWriteSelection, T>* >(pair.second); 
                  Serial.println("--> Options: ");
                  for(const char* option : myPtr->_selection.getOptions()){
                    Serial.println(option);
                  }
                  Serial.println("IS: " + String(myPtr->_selection.getDescriptionByValue(*(myPtr->state))));
                  *(myPtr->state) = myPtr->_selection.getValueByIndex(0);
                }
              }
*/

template<typename T>
struct Selection{
    private:
        std::vector<std::pair<T, const char*>> _selection ;
    public:
        Selection(std::vector<std::pair<T, const char*>> selection) : _selection(selection){
        }
        Selection(std::initializer_list<std::pair<T, const char*>> selection): _selection(selection){

        }
        Selection& getSelection(){
            return _selection;
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
        T getValueByIndex(int index){
            if(index < _selection.size()){
                return _selection[index].first;
            }
            return _selection[0].first;
            
        }
        const char* getDescriptionByValue(T value){
            for(std::pair<T, const char*> &pair : _selection){
                if(pair.first == value){
                    return pair.second;
                }
            }
            return "not found";
        }

        std::vector<const char*> getOptions(){
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
        const char* componentName;

        BaseComponent(const char* ComponentName): componentName(ComponentName){
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
        ExposedStateType stateType;
        const String stateName;
        virtual String toString() =  0;
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
    String toString() override{
        return String(*state);
    }
    ExposedState(String StateName,T* State): BaseExposedState(StateName), state(State) {
        stateType = ExposedStateType::ReadOnly;
    }
};

template <typename T>
struct ExposedState<ExposedStateType::ReadWrite, T> : BaseExposedState {
    T* state;
    String toString() override{
        return String(*state);
    }
    ExposedState(String StateName,T* State): BaseExposedState(StateName), state(State){
        stateType = ExposedStateType::ReadWrite;
    }
};

template <typename T>
struct ExposedState<ExposedStateType::ReadWriteRanged, T> : BaseExposedState {
    //static_assert if T is orderable
    T* state;
    T minState;
    T maxState;
    T stepState;
    String toString() override{
        return String(*state);
    }
    ExposedState(String StateName,T* State, T MinState, T MaxState, T stepState): BaseExposedState(StateName), state(State), minState(MinState), maxState(MaxState), stepState(stepState){
        stateType = ExposedStateType::ReadWriteRanged;
    }
};
template <typename T>
struct ExposedState<ExposedStateType::ReadWriteSelection, T> : BaseExposedState {
    T* state;
    Selection<T>& _selection;
    String toString() override{
        return _selection.getDescriptionByValue(*state);
    }
    ExposedState(String StateName,T* State, Selection<T>& selection): BaseExposedState(StateName), state(State), _selection(selection){
        stateType = ExposedStateType::ReadWriteSelection;
    }
};






//Contains a list of all available system components
class Components{
    public:
        //Class that represents a TP100 temperature sensor
        class TemperatureSensor : BaseComponent {
            private:
                AnalogInPt100 &analogInPt100;
                double temperature;
                ExposedState<ExposedStateType::ReadOnly, double> temeraturePtr;
                Unit<Units::Temperature> displayUnit;
                ExposedState<ExposedStateType::ReadWriteSelection, Units::Temperature> displayUnitPtr;
                

            public:
                TemperatureSensor(AnalogInPt100 &analogInPt100, const char* componentName = "genericTemperatureSensor") : analogInPt100(analogInPt100), BaseComponent(componentName), temperature(0), temeraturePtr("Temp",&temperature), displayUnit(Units::Temperature::C), displayUnitPtr("dispUnit", &(displayUnit.unitType), displayUnit.selection){
                }

                //Returns the temperature in K
                double getTemperature(){
                    update();
                    return temperature;
                }
                //Returns the temperature as string including the unit suffix. The unit can be set with setDisplayUnit
                String getTeperatureAsString() {
                    update();
                    return String(displayUnit.convertFromSI(temperature)) + displayUnit.getSuffix();
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
                    return "Temperature Sensor";
                }
                //Returns the component Name
                const char* const getComponentName() {
                    return componentName;
                }
        };
        

        //Represents a pressure gauge
        class PressureGauge : BaseComponent {
            public:
                //Collection of all available gauge types
                enum struct GaugeType{
                    //PKR (Pirani/cold cathode gauge)
                    PKR,
                    //TPR (ActiveLine Pirani gauge)
                    TPR
                };

                double pressure;
                ExposedState<ExposedStateType::ReadOnly,double> pressureState;
                Unit<Units::Pressure> displayUnit;
                ExposedState<ExposedStateType::ReadWriteSelection, Units::Pressure> displayUnitState;
                GaugeType gaugeType;

                ExposedState<ExposedStateType::ReadWriteSelection, GaugeType> gaugeTypeState;


                Selection<GaugeType> gaugesTypesSelection = {{Components::PressureGauge::GaugeType::PKR, "PKR"} ,{Components::PressureGauge::GaugeType::TPR, "TPR"}};
                //Returns a list of all available pressure gauges.

                PressureGauge(AnalogInBase &analogIn, GaugeType gaugeType, const char* componentName = "genericPressureGauge") 
                    :BaseComponent(componentName),
                    analogIn(analogIn), 
                    gaugeType(gaugeType),
                    pressure(0), 
                    pressureState("Pressure",&pressure),
                    displayUnit(Units::Pressure::Pa),
                    displayUnitState("Display Unit",&(displayUnit.unitType),
                    displayUnit.selection),
                    gaugeTypeState("Gauge Type", &gaugeType, gaugesTypesSelection) {}



                //Returns the pressure in Pa
                double getPressure(){
                    update();
                    return pressure;
                }
    
                //Returns the pressure as string including the unit suffix. The unit can be set with setDisplayUnit
                String getPressureAsString() {
                    update();
                    return doubleToSciString(displayUnit.convertFromSI(pressure)) + displayUnit.getSuffix();
                }
                //All calculations are done in SI units. In the case of temperature in Kelvin. But when the teperature is requested as string, it will be converted to the unit set here
                void setDisplayUnit(Units::Pressure unit){
                    displayUnit.unitType = unit;
                }
                //Reads the current temperature and updates the internal state
                void update() override {
                    pressure = voltageToPressure(gaugeType, analogIn.getVoltage());
                }

                //Retuns the component type
                String const getComponentType() const  {
                    return "Pressure Gauge";
                }
                //Returns the component Name
                const char* getComponentName() const {
                    return componentName;
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

            private:
                const char* componentName;
                AnalogInBase &analogIn;
                //Converts a given voltage to a pressure value, with respect to the given gauge Type
                static double voltageToPressure(GaugeType gauge, double voltage){
                    if(gauge == GaugeType::PKR){
                        return pow(10., (1.667*voltage-9.33));
                    }else if (gauge == GaugeType::TPR){
                        return pow(10., (voltage-3.5));
                    }
                    return 0.;
                }

        };     
         
       
};




#endif