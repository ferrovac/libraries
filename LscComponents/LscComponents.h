/*  
    Copyright (C) 2024 Ferrovac AG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    NOTE: This specific version of the license has been chosen to ensure compatibility 
          with the SD library, which is an integral part of this application and is 
          licensed under the same version of the GNU General Public License.
*/

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
#include "LscOS.h"
#include <Arduino.h>
#include <vector>
#include "LscHardwareAbstraction.h"
#include <type_traits>
#include "LscPersistence.h"
#include <functional>


extern void waitForSaveReadWrite();
class BaseComponent;
struct BaseExposedState;

        class ComponentTracker{
            private:
                ComponentTracker() {}
            public:
                volatile unsigned long lastOsCall;
                std::vector<BaseComponent*> components;
                std::vector<std::pair<BaseComponent*, BaseExposedState*>> states;

                static ComponentTracker& getInstance() {
                    static ComponentTracker instance;  
                    return instance;
                }


                //Returns a vector with pointers to all registered components. 
                std::vector<BaseComponent*> getComponets(){
                    waitForSaveReadWrite();
                    return ComponentTracker::getInstance().components;
                }
                //Registers the given component with the ComponentTracker
                void registerComponent(BaseComponent* component){
                    waitForSaveReadWrite();
                    ComponentTracker::getInstance().components.push_back(component);
                }
                void registerState(BaseExposedState* State){
                    waitForSaveReadWrite();
                    ComponentTracker::getInstance().states.push_back({ComponentTracker::getInstance().components.back(), State});
                }
                String getIDAsString(){
                  return String(components.size() + String(states.size()));
                }
        };
          
        

/*  How to use the tracker 
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
        const std::vector<std::pair<T, const char*>> _selection ;
    public:
        Selection(std::vector<std::pair<T, const char*>> selection) : _selection(selection){
        }
        Selection(std::initializer_list<std::pair<T, const char*>> selection): _selection(selection){
        }
        std::vector<std::pair<T, const char*>> getSelection(){
            return _selection;
        }
        int getIndexByValue(T value){
            int counter = 0;
            for(auto& pair : _selection){
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
            for(std::pair<T, const char*> pair : _selection){
                if(pair.first == value){
                    return pair.second;
                }
            }
            return "not found";
        }

        std::vector<const char*> getOptions(){
            std::vector<const char*> ret;
            for(std::pair<T, const char*> pair : _selection){
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
                    return "C"; 
                case Units::Temperature::F:
                    return "F"; 
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
                    return " Pa"; 
                case Units::Pressure::mBar:
                    return " mbar"; 
                case Units::Pressure::Torr:
                    return " Torr";
                case Units::Pressure::psi:
                    return " psi";
                case Units::Pressure::atm:
                    return " atm"; 
            }
        }
};


enum class GaugeType{
    PKR,
    TPR,
    VPM5,
    BGP400,
    IKR270
};
enum class GassType{
    N2,
    He,
    Ne,
    Ar,
    Kr,
    Xe
};

class Gauge{
    public:
        GaugeType gaugeType;
        Selection<GaugeType> selection;
        Gauge(GaugeType gaugeType) : gaugeType(gaugeType), selection({{GaugeType::PKR, "PKR"} ,{GaugeType::TPR, "TPR"}, {GaugeType::VPM5, "VPM5"}, {GaugeType::BGP400, "BGP400"}, {GaugeType::IKR270, "IKR270"}}){
        }
        double getPressureFromVoltage(double voltage){
            switch(gaugeType){
                case GaugeType::PKR:
                    return pow(10., (1.667*voltage-9.33));
                case GaugeType::TPR:
                    return pow(10., (voltage-3.5));
                case GaugeType::VPM5:
                    return pow(10., (voltage-4.5));  //still not sure this is correct....
                case GaugeType::BGP400:
                    return pow(10., (voltage-7.75)/0.75+2);
                case GaugeType::IKR270:
                    return pow(10., (1.25*voltage)-10.75);
            }
            return 0;
        }
        double getLowerCutOff(){
            switch(gaugeType){
                case GaugeType::PKR:
                    return 0.00000015;
                case GaugeType::TPR:
                    return 0.015;
                case GaugeType::VPM5:
                    return 0.00000015;
                case GaugeType::BGP400:
                    return 1e-9;
                case GaugeType::IKR270:
                    return 1e-9;
            }
            return 0;
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



enum struct ExposedStateType{
    ReadOnly,
    ReadWrite,
    ReadWriteRanged,
    ReadWriteSelection,
    Action
};
enum struct TypeMetaInformation{
    DOUBLE,
    INT,
    BOOL,
    INDEX,
    UNKNOWN
};



template <ExposedStateType StateType, typename T>
struct ExposedState;

struct BaseExposedState{
    private:        
    public:
        ExposedStateType stateType;
        TypeMetaInformation typeInfo;
        const char* stateName;
        virtual void writeToSD() = 0;
        virtual void readFromSD() = 0;
        virtual void executeAction() = 0;

        BaseExposedState(const char* StateName) : stateName(StateName), typeInfo(TypeMetaInformation::UNKNOWN) {
            ComponentTracker::getInstance().registerState(this);
        }
        virtual ~BaseExposedState() {};
};



template<typename T>
TypeMetaInformation getTypeMetaInformation(){
    if(std::is_same<T, volatile int>::value) return TypeMetaInformation::INT;
    if(std::is_same<T, volatile double>::value) return TypeMetaInformation::DOUBLE;
    if(std::is_same<T, volatile bool>::value) return TypeMetaInformation::BOOL;
    return TypeMetaInformation::UNKNOWN;
}


template <typename T>
//is the same as ReadWrite will check in menu if setting state is allowed
struct ExposedState<ExposedStateType::ReadOnly, T> : BaseExposedState {
    static_assert(std::is_same<T, volatile int>::value || std::is_same<T,volatile double>::value || std::is_same<T,volatile bool>::value,
                  "ExposedState<ExposedStateType::ReadOnly, T> only supports volatile: int, double and bool ");
    T* state;
    Persistent<T> persistentState;        
    void writeToSD() override{
        waitForSaveReadWrite();
        persistentState = *state;
    }
    void readFromSD() override{
        waitForSaveReadWrite();
        *state = persistentState;
    }
    void executeAction() override{

    }
    

    ExposedState(const char* StateName,T* State): BaseExposedState(StateName), state(State), persistentState(ComponentTracker::getInstance().getIDAsString()) {
        stateType = ExposedStateType::ReadOnly;
        typeInfo = getTypeMetaInformation<T>();
    }
};

template <typename T>
struct ExposedState<ExposedStateType::ReadWrite, T> : BaseExposedState {
    static_assert(std::is_same<T, volatile int>::value || std::is_same<T,volatile double>::value || std::is_same<T,volatile bool>::value,
                  "ExposedState<ExposedStateType::ReadWrite, T> only supports volatile: int, double and bool ");
    T* state;
    Persistent<T> persistentState;        
    void writeToSD() override{
        waitForSaveReadWrite();
        persistentState = *state;
    }
    void readFromSD() override{
        waitForSaveReadWrite();
        *state = persistentState;
    }
    void executeAction() override{

    }

    ExposedState(const char* StateName,T* State): BaseExposedState(StateName), state(State){
        stateType = ExposedStateType::ReadWrite;
        typeInfo = getTypeMetaInformation<T>();
    }
};

template <typename T>
struct ExposedState<ExposedStateType::Action, T> : BaseExposedState {
    T* object;
    using CallbackType = void (T::*)();
    CallbackType callback;
    void writeToSD() override{

    }
    void readFromSD() override{
        
    }
    ExposedState(const char* StateName, T* object, CallbackType callback): BaseExposedState(StateName),object(object),callback(callback){
        stateType = ExposedStateType::Action;
        typeInfo = TypeMetaInformation::UNKNOWN;
    }
    void executeAction() override{
        if(object && callback){
            (object->*callback)();
        }
        
    }
};

template <typename T>
struct ExposedState<ExposedStateType::ReadWriteRanged, T> : BaseExposedState {
    static_assert(std::is_same<T, volatile int>::value || std::is_same<T,volatile double>::value ,
                  "ExposedState<ExposedStateType::ReadWriteRanged, T> only supports volatile: int and double ");
    T* state;
    T minState;
    T maxState;
    T stepState;

    Persistent<T> persistentState;        
    void writeToSD() override{
        waitForSaveReadWrite();
        persistentState = *state;
    }
    void readFromSD() override{
        waitForSaveReadWrite();
        *state = persistentState;
    }
    void executeAction() override{

    }
   
    ExposedState(const char* StateName,T* State, T MinState, T MaxState, T stepState): BaseExposedState(StateName), state(State), minState(MinState), maxState(MaxState), stepState(stepState){
        stateType = ExposedStateType::ReadWriteRanged;
        typeInfo = getTypeMetaInformation<T>();
    }
    
    
};
template <typename T>
struct ExposedState<ExposedStateType::ReadWriteSelection, T> : BaseExposedState {
    T* state;
    int index;
    int* indexPtr;
    Selection<T> _selection;
    Persistent<int> persistentIndex;        
    void writeToSD() override{
        waitForSaveReadWrite();
        persistentIndex = index;
        persistentIndex.writeObjectToSD();
        Serial.println("State: "+ String(stateName) + " wrote index: " + String(index));
            }
    void readFromSD() override{
        waitForSaveReadWrite();
        index = persistentIndex;
        Serial.println("State: "+ String(stateName) + " read index: " + String(index));
        writeSelectionItemToState();
    }
    void executeAction() override{

    }

    ExposedState(const char* StateName,T* State, Selection<T> selection): BaseExposedState(StateName), state(State), _selection(selection), persistentIndex(ComponentTracker::getInstance().getIDAsString(),index){
        persistentIndex.setMinIntervall(0);
        stateType = ExposedStateType::ReadWriteSelection;
        typeInfo = TypeMetaInformation::INDEX;
        index = _selection.getIndexByValue(*state);
        persistentIndex = index;
    }
    
    void writeSelectionItemToState(){
        *state = _selection.getSelection()[index].first;
    }
};

struct ExposedStateInterface {
    private:
        BaseExposedState* exposedState;
        
    public:
        ExposedStateInterface(BaseExposedState* exposedState): exposedState(exposedState) {}
        ExposedStateType getStateType(){
            return exposedState->stateType;
        }
        void executeAction(){
            waitForSaveReadWrite();
            if(exposedState->stateType == ExposedStateType::Action){
                exposedState->executeAction();
            }
        }
        void saveState(){
            exposedState->writeToSD();
        }
        void loadState(){
            exposedState->readFromSD();
        }

        template<typename T>
        void setStateValue(T Value){
            waitForSaveReadWrite();
            switch(exposedState->stateType){
                case ExposedStateType::ReadWriteSelection:{
                    auto castStatePtr = static_cast<ExposedState<ExposedStateType::ReadWriteSelection, void*>*>(exposedState);
                    if((int)Value >= 0 && (int)Value < castStatePtr->_selection.getSelection().size()){
                        castStatePtr->index = (int)Value;
                        castStatePtr->writeSelectionItemToState();
                    }
                    break;
                }
                
                case ExposedStateType::ReadWrite:{
                    if(getTypeMetaInformation<T>() != exposedState->typeInfo) return; //implement error handling
                    switch(exposedState->typeInfo){
                        case TypeMetaInformation::DOUBLE:{
                            auto castStatDoublePtr = static_cast<ExposedState<ExposedStateType::ReadWrite, volatile double>*>(exposedState);
                            *(castStatDoublePtr->state) = Value;
                            break;
                        }
                        case TypeMetaInformation::BOOL:{
                            auto castStatBoolPtr = static_cast<ExposedState<ExposedStateType::ReadWrite, volatile bool>*>(exposedState);
                            *(castStatBoolPtr->state) = Value;
                            break;
                        }
                        case TypeMetaInformation::INT:{
                            auto castStatIntPtr = static_cast<ExposedState<ExposedStateType::ReadWrite, volatile int>*>(exposedState);
                            *(castStatIntPtr->state) = Value;
                            break;
                        }
                    }
                    break;
                }
                case ExposedStateType::ReadWriteRanged:{
                    if(getTypeMetaInformation<T>() != exposedState->typeInfo) return; //implement error handling
                    switch(exposedState->typeInfo){
                        case TypeMetaInformation::DOUBLE:{
                            auto castStatDoublePtr = static_cast<ExposedState<ExposedStateType::ReadWriteRanged, volatile double>*>(exposedState);
                            *(castStatDoublePtr->state) = Value;
                            break;
                        }
                        case TypeMetaInformation::INT:{
                            auto castStatIntPtr = static_cast<ExposedState<ExposedStateType::ReadWriteRanged, volatile int>*>(exposedState);
                            *(castStatIntPtr->state) = Value;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        template<typename T>
        T getStateValue(){
            waitForSaveReadWrite();
            if(exposedState->stateType == ExposedStateType::ReadWriteSelection){
                auto castStatePtr = static_cast<ExposedState<ExposedStateType::ReadWriteSelection, void*>*>(exposedState);
                return static_cast<T>(castStatePtr->index);
            }else if(exposedState->stateType == ExposedStateType::ReadOnly){
                auto castStatePtr = static_cast<ExposedState<ExposedStateType::ReadOnly,volatile T>*>(exposedState);
                return static_cast<T>(*(castStatePtr->state));
            }else if(exposedState->stateType == ExposedStateType::ReadWrite){
                auto castStatePtr = static_cast<ExposedState<ExposedStateType::ReadWrite,volatile T>*>(exposedState);
                return static_cast<T>(*(castStatePtr->state));
            }else if(exposedState->stateType == ExposedStateType::ReadWriteRanged){
                if(exposedState->typeInfo == TypeMetaInformation::DOUBLE){
                    auto castStatePtr = static_cast<ExposedState<ExposedStateType::ReadWriteRanged,volatile double>*>(exposedState);
                    return static_cast<double>(*(castStatePtr->state));
                }else if(exposedState->typeInfo == TypeMetaInformation::INT){
                    auto castStatePtr = static_cast<ExposedState<ExposedStateType::ReadWriteRanged,volatile int>*>(exposedState);
                    return static_cast<int>(*(castStatePtr->state));
                }
                
            }
        }
        const String getStateValueAsString(){
            waitForSaveReadWrite();
            switch(exposedState->typeInfo){
                case TypeMetaInformation::DOUBLE:
                    return String(getStateValue<double>(), 10);
                case TypeMetaInformation::BOOL:
                    return String(getStateValue<bool>());
                case TypeMetaInformation::INT:
                    return String(getStateValue<int>());
                case TypeMetaInformation::INDEX:
                    {
                    int index = getStateValue<int>();
                    if(exposedState->stateType == ExposedStateType::ReadWriteSelection){
                        std::vector<String> options;
                        for(String option : getOptions()){
                            options.push_back(option);
                        }
                        if(index >= 0 && index < options.size()) return options[index];
                        return String(index);
                    }
                    return String(index);
                    }
                default:
                    return "Unknown Type"; 
            }
        }
        const char* getStateTypeAsConstChar(){
            waitForSaveReadWrite();
            switch(exposedState->typeInfo){
                case TypeMetaInformation::DOUBLE:
                    return "double";
                case TypeMetaInformation::BOOL:
                    return "bool";
                case TypeMetaInformation::INT:
                    return "int";
                case TypeMetaInformation::INDEX:
                    return "index";
                default:
                    return "Unknown Type";
            }
        }
        std::vector<const char*> getOptions(){
            waitForSaveReadWrite();
            if(exposedState->stateType != ExposedStateType::ReadWriteSelection) return {""};
            auto castStatePtr = static_cast<ExposedState<ExposedStateType::ReadWriteSelection, void*>*>(exposedState);
            return castStatePtr->_selection.getOptions();
        }
};



//Contains a list of all available system components
class Components{
    public:

        //Class that represents a TP100 temperature sensor
        class TemperatureSensor : BaseComponent {
            private:
                AnalogInPt100 &analogInPt100;
                volatile double temperature;
                ExposedState<ExposedStateType::ReadOnly, volatile double> temeraturePtr;
                Unit<Units::Temperature> displayUnit;
                ExposedState<ExposedStateType::ReadWriteSelection, Units::Temperature> displayUnitPtr;
                ExposedState<ExposedStateType::Action, Components::TemperatureSensor> myAcction;
                void test(){
                    Serial.println("callback successfull");
                    Serial.println(__PRETTY_FUNCTION__);
                }
                

            public:
                TemperatureSensor(AnalogInPt100 &analogInPt100, const char* componentName = "genericTemperatureSensor") 
                        :   analogInPt100(analogInPt100), 
                            BaseComponent(componentName), 
                            temperature(0), 
                            temeraturePtr("Temperature",&temperature), 
                            displayUnit(Units::Temperature::C), 
                            displayUnitPtr("Display Unit", &(displayUnit.unitType), displayUnit.selection),
                            myAcction("test",this , &TemperatureSensor::test)
                        {                            
                }
                
                //Returns the temperature in K
                double getTemperature(){
                    waitForSaveReadWrite();
                    return temperature;
                }
                //Returns the temperature as string including the unit suffix. The unit can be set with setDisplayUnit
                String getTeperatureAsString() {
                    waitForSaveReadWrite();
                    return String(displayUnit.convertFromSI(getTemperature()),0) + displayUnit.getSuffix();
                }
                //All calculations are done in SI units. In the case of temperature in Kelvin. But when the teperature is requested as string, it will be converted to the unit set here
                void setDisplayUnit(Units::Temperature unit){
                    waitForSaveReadWrite();
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
        class FIB;
        class PressureGauge : BaseComponent {
            private:
                AnalogInBase &analogIn;
                volatile double pressure;
                ExposedState<ExposedStateType::ReadOnly, volatile double> pressurePtr;
                Unit<Units::Pressure> displayUnit;
                ExposedState<ExposedStateType::ReadWriteSelection, Units::Pressure> displayUnitPtr;
                Gauge gauge;
                ExposedState<ExposedStateType::ReadWriteSelection, GaugeType> gaugePtr;
                volatile bool errorState;
                volatile bool ignoreErrorState;
                Selection<volatile bool> ignoreErrorStateSelection;
                ExposedState<ExposedStateType::ReadWriteSelection, volatile bool> ignoreErrorStatePtr;
                GassType gassType;
                Selection<GassType> gasTypeSelection;
                ExposedState<ExposedStateType::ReadWriteSelection, GassType> gassTypePtr;
                
                
            public:
                friend class FIB;
                PressureGauge(AnalogInBase &analogIn, const char* componentName = "genericPressureSensor", GaugeType gaugeType = GaugeType::PKR, GassType gassType = GassType::N2) 
                        :   analogIn(analogIn), 
                            BaseComponent(componentName), 
                            pressure(0), 
                            pressurePtr("Pressure",&pressure), 
                            displayUnit(Units::Pressure::mBar), 
                            displayUnitPtr("Display Unit", &(displayUnit.unitType), displayUnit.selection),
                            gauge(gaugeType), 
                            gaugePtr("Gauge Type",&(gauge.gaugeType),gauge.selection),
                            errorState(false),
                            ignoreErrorState(false),
                            ignoreErrorStateSelection({{true,"True"},{false,"False"}}),
                            ignoreErrorStatePtr("Ignore Error State", &ignoreErrorState, ignoreErrorStateSelection),
                            gassType(gassType),
                            gasTypeSelection({{GassType::N2, "N2"}, {GassType::He, "He"}, {GassType::Ne, "Ne"}, {GassType::Ar, "Ar"}, {GassType::Kr, "Kr"}, {GassType::Xe, "Xe"}}),
                            gassTypePtr("Gas Type", &gassType, gasTypeSelection)
                        {
                }
                String doubleToSciString(double value) {
                    if(value == 0) return "0.00E+00";
                    
                    int exponent = static_cast<int>(floor(log10(value)));
                    double mantissa = value / pow(10, exponent);
                    int correction = 0;
                    if(String(mantissa,2) == "10.00"){
                        mantissa = 1.;
                        correction = -1;
                    }
                    if(exponent >=11) return String(mantissa,2) + "E+" + String(exponent+correction);
                    if(exponent >= 0) return String(mantissa,2) + "E+0" + String(exponent+correction);
                    if(exponent <= -10) return String(mantissa,2) + "E" + String(exponent+correction);
                    if(exponent <  0) return String(mantissa,2) + "E-0" + String(abs(exponent-correction));
                    ERROR_HANDLER.throwError(0x0, "Failed to corretly format number in scientific notation. This has to be an error in LscComponents lib. see doubleToSciString()",SeverityLevel::NORMAL);
                    
                    return String(value,10);
                }
                //Returns the temperature in K
                double getPressure(){
                    waitForSaveReadWrite();
                    switch (gauge.gaugeType){
                        case GaugeType::PKR :
                            switch (gassType){
                                case GassType::N2 :
                                    return pressure * 1.0;
                                    break;
                                case GassType::He :
                                    return pressure * 5.9;
                                    break;
                                case GassType::Ne :
                                    return pressure * 4.1;
                                    break;
                                case GassType::Ar :
                                    return pressure * 0.8;
                                    break;
                                case GassType::Kr :
                                    return pressure * 0.5;
                                    break;
                                case GassType::Xe :
                                    return pressure * 0.4;
                                    break;
                                default:
                                    return pressure * 1.0;
                                    break;
                            }

                        break;
                        case GaugeType::TPR :
                            switch (gassType){
                                case GassType::N2 :
                                    return pressure * 1.0;
                                    break;
                                case GassType::He :
                                    return pressure * 0.8;
                                    break;
                                case GassType::Ne :
                                    return pressure * 1.4;
                                    break;
                                case GassType::Ar :
                                    return pressure * 1.7;
                                    break;
                                case GassType::Kr :
                                    return pressure * 2.4;
                                    break;
                                case GassType::Xe :
                                    return pressure * 3.0;
                                    break;
                                default:
                                    return pressure * 1.0;
                                    break;
                            }

                        break;
                    
                    default:
                        return pressure;
                        break;
                    }   
                    return pressure;
                }
                
                //Returns the temperature as string including the unit suffix. The unit can be set with setDisplayUnit
                String getPressureAsString(bool printUnitSuffix = true) {
                    waitForSaveReadWrite();
                    if(errorState && !ignoreErrorState) return "ERROR";
                    if(pressure <= gauge.getLowerCutOff()) return "---UR---" ;
                    if(printUnitSuffix){
                        return doubleToSciString(displayUnit.convertFromSI(getPressure())) + displayUnit.getSuffix();
                    }else{
                        return doubleToSciString(displayUnit.convertFromSI(getPressure()));
      
                    }
                }
                String getUnitSuffixAsString() {
                    waitForSaveReadWrite();
                    return displayUnit.getSuffix();
                }
                //All calculations are done in SI units. In the case of temperature in Kelvin. But when the teperature is requested as string, it will be converted to the unit set here
                void setDisplayUnit(Units::Pressure unit){
                    waitForSaveReadWrite();
                    displayUnit.unitType = unit;
                }

                //Reads the current temperature and updates the internal state
                void update() override {
                    double voltage = analogIn.getVoltage();
                    //Serial.println(voltage);
                    if(voltage < 0.5){
                        errorState = true;
                    } else{
                        errorState = false;
                    }
                    pressure = gauge.getPressureFromVoltage(voltage);
                    
                }
                bool error(){
                    waitForSaveReadWrite();
                    return errorState;
                }
                bool ignoreError(){
                    waitForSaveReadWrite();
                    return ignoreErrorState;
                }

                //Retuns the component type
                String const getComponentType()   {
                    return "Pressure Gauge";
                }
                //Returns the component Name
                const char* const getComponentName() {
                    return componentName;
                }
        };
        
        class Valve : BaseComponent {
            private:
                DigitalOutBase &powerSwitch;
                volatile bool _isState;
                ExposedState<ExposedStateType::ReadOnly, volatile bool> state;
                ExposedState<ExposedStateType::Action, Components::Valve>  actionOpen;
                ExposedState<ExposedStateType::Action, Components::Valve>  actionClose;
                volatile long openTimer;
                volatile long closeTimer;
            public:
                bool* mystateptr;
                Valve(DigitalOutBase &powerSwitch, const char* componentName = "genericValve") 
                    :   BaseComponent(componentName),   
                        powerSwitch(powerSwitch),
                        _isState(false),
                        state("State",&_isState),
                        actionOpen("Open", this, &Valve::open),
                        actionClose("Close", this, &Valve::close),
                        openTimer(0),
                        closeTimer(0)
                    {
                    
                }
            
                void update() override {
                    if(openTimer > (long)0){
                        openTimer -= 100;
                        if(openTimer < (long)0 ){
                            powerSwitch.setState(true);
                            _isState = true;
                        }
                    }
                    if(closeTimer > (long)0){
                        closeTimer -= 100;
                        if(closeTimer < (long)0 ){
                            powerSwitch.setState(true);
                        _isState = false;
                        }
                    }
                    
                }

                bool getState(){
                    waitForSaveReadWrite();
                    return _isState;
                }
                void setState(bool State){
                    waitForSaveReadWrite();
                    if(State){
                        open();
                    }else{
                        close();
                    }
                }
                void open(){
                    if(openTimer > 0) return;
                    waitForSaveReadWrite();
                    if(_isState) return;
                    powerSwitch.setState(true);
                    _isState = true;
                }
                void close(){
                    if(closeTimer > 0) return;
                    waitForSaveReadWrite();
                    if(!_isState) return;
                    powerSwitch.setState(false);
                    _isState = false;
                }
                //open with a delay in 100ms increments
                void open(long delay_ms){
                    openTimer = delay_ms;
                }
                //close with a delay in 100ms increments
                void close(long delay_ms){ 
                    closeTimer = delay_ms;
                }

                //Retuns the component type
                String const getComponentType()   {
                    return "Valve";
                }
                //Returns the component Name
                const char* const getComponentName() {
                    return componentName;
                }
        };

        class LN2LevelMeter : BaseComponent {
            private:
                AnalogInBase &analogIn;
                volatile int _state;
                long lastMesurementTime;
                ExposedState<ExposedStateType::ReadOnly, volatile int> state;
            public:
                LN2LevelMeter(AnalogInBase &analogIn, const char* componentName = "genericLN2Meter") 
                    :   BaseComponent(componentName),   
                        analogIn(analogIn),
                        _state(0),
                        lastMesurementTime(millis()),
                        state("State",&_state)
                    {
                    
                }
            
                void update() override {
                    if(millis() - lastMesurementTime > 100){
                        _state = ((double)_state +  (0.0307*(analogIn.getVoltage()/10.*4096.)) -24.61) / 2.;
                        if(_state < 0 ) _state = 0;
                    }
                    
                }

                int getState(){
                    waitForSaveReadWrite();
                    return _state;
                }

                //Retuns the component type
                String const getComponentType()   {
                    return "LN2 Level Meter";
                }
                //Returns the component Name
                const char* const getComponentName() {
                    return componentName;
                }
        };

        class RoughingPump : BaseComponent {
            private:
                MOSContact &mosContact;
                bool _isState;
                ExposedState<ExposedStateType::Action, Components::RoughingPump> actionTurnOn;
                ExposedState<ExposedStateType::Action, Components::RoughingPump> actionTurnOff;
                
                
            public:
                RoughingPump(MOSContact &mosContact, const char* componentName = "RoughingPump") 
                    :   BaseComponent(componentName),   
                        mosContact(mosContact),
                        _isState(false),
                        actionTurnOn("Turn On", this,&RoughingPump::turnOn),
                        actionTurnOff("Turn Off", this, &RoughingPump::turnOff)
                        {
                }
            
                void update() override {
                    
                }
                bool getState(){
                    waitForSaveReadWrite();
                    return _isState;
                }
                void setState(bool State){
                    waitForSaveReadWrite();
                    if(State == _isState) return;
                    State ? turnOn() : turnOff();
                    _isState = State;
                }
                void turnOn(){
                    waitForSaveReadWrite();
                    mosContact.setState(true);
                    _isState = true;
                }
                void turnOff(){
                    waitForSaveReadWrite();
                    mosContact.setState(false);
                    _isState = false;
                }

                //Retuns the component type
                String const getComponentType()   {
                    return "Roughing Pump";
                }
                //Returns the component Name
                const char* const getComponentName() {
                    return componentName;
                }
        };

        class GateValve : BaseComponent {
            private:
                PowerSwitch &powerSwitchOpen;
                PowerSwitch &powerSwitchClose;
                DigitalInIsolated &digitalInIsolatedGateValveState;
                ExposedState<ExposedStateType::Action, Components::GateValve> actionOpen;
                ExposedState<ExposedStateType::Action, Components::GateValve> actionClose;
                volatile bool state;
                ExposedState<ExposedStateType::ReadOnly, volatile bool> statePtr;
                volatile long openTimer;
                volatile long closeTimer;
                
            public:
                GateValve(PowerSwitch &powerSwitchOpen, PowerSwitch &powerSwitchClose, DigitalInIsolated& digitalInIsolatedGateValveState,  const char* componentName = "GateValve") 
                    :   BaseComponent(componentName),   
                        powerSwitchOpen(powerSwitchOpen),
                        powerSwitchClose(powerSwitchClose),
                        digitalInIsolatedGateValveState(digitalInIsolatedGateValveState),
                        actionOpen("Open", this, &GateValve::open),
                        actionClose("Close", this, &GateValve::close),
                        state(digitalInIsolatedGateValveState.getState()),
                        statePtr("State",&state),
                        openTimer(0),
                        closeTimer(0)
                        {
                }
            
                void update() override {
                    state = digitalInIsolatedGateValveState.getState();
                    //Serial.println(digitalInIsolatedGateValveState.getDescription());
                    if(openTimer > (long)0){
                        openTimer -= 100;
                        if(openTimer < (long)0 ){
                            open();
                        }
                    }
                    if(closeTimer > (long)0){
                        closeTimer -= 100;
                        if(closeTimer < (long)0 ){
                            close();
                        }
                    }
                }
                bool getState(){
                    waitForSaveReadWrite();
                    state = digitalInIsolatedGateValveState.getState();
                    return state;
                }
                void setState(int State){
                    State ? open() : close();
                }
                void open(){
                    if(openTimer > 0) return;
                    powerSwitchClose.setState(false);
                    powerSwitchOpen.setState(true);
                }
                void close(){
                    if(closeTimer > 0) return;
                    powerSwitchOpen.setState(false);
                    powerSwitchClose.setState(true);
                }
                //open with a delay in 100ms increments
                void open(long delay_ms){
                    openTimer = delay_ms;
                }
                //close with a delay in 100ms increments
                void close(long delay_ms){ 
                    closeTimer = delay_ms;
                }

                //Retuns the component type
                String const getComponentType()   {
                    return "Gate Valve";
                }
                //Returns the component Name
                const char* const getComponentName() {
                    return componentName;
                }
        };

        class FIB : BaseComponent {
            private:
                volatile bool transferRequestSent;
                volatile unsigned long transferRequestSentTime;
                MOSContact &transferRequest;
                AnalogOutIsolated &pressureFibSim;
                DigitalInIsolated &transferRequestResponse;
                PressureGauge &pressureGauge;
                ExposedState<ExposedStateType::Action, Components::FIB> sendTransferRequestAction;
                volatile bool requestGranted;
                ExposedState<ExposedStateType::ReadOnly, volatile bool> requestGrantedPtr;
                
                
            public:
                FIB(MOSContact &TransferRequest, AnalogOutIsolated &PressureFibSim, DigitalInIsolated &TransferRequestResponse, PressureGauge &pressureGauge,  const char* componentName = "FIB") 
                    :   BaseComponent(componentName),
                        transferRequestSent(false),
                        transferRequestSentTime(0),   
                        transferRequest(TransferRequest),
                        pressureFibSim(PressureFibSim),
                        transferRequestResponse(TransferRequestResponse),
                        pressureGauge(pressureGauge),
                        sendTransferRequestAction("Send Transfer Request", this, &FIB::sendTransferRequest),
                        requestGranted(transferRequestResponse.getState()),
                        requestGrantedPtr("Request Granted",&requestGranted)
                        {
                            
                }
            
                void update() override {
                    requestGranted = transferRequestResponse.getState();
                    if(transferRequestSent){
                        if(millis() - transferRequestSentTime > 1000){
                            transferRequest.setState(false);
                            transferRequestSent = false;
                        }
                    }
                    //Simulate the signal for the FIB pressure gauge.
                    double pressure = pressureGauge.pressure;
                    if(pressure < 3) pressure = 3; //The fib fill show a gauge error if the pressure is too low we cap it at 3Pa
                    if(pressure > 9.99) pressure = 9.99;
                    pressureFibSim.setVoltage(pressure); 
                }
                
                void sendTransferRequest(){
                    waitForSaveReadWrite();   
                    transferRequest.setState(true);
                    transferRequestSent = true;
                    transferRequestSentTime = millis();
                }
                bool getRequestState(){
                    waitForSaveReadWrite();
                    return transferRequestResponse.getState();
                }

                //Retuns the component type
                String const getComponentType()   {
                    return "FIB";
                }
                //Returns the component Name
                const char* const getComponentName() {
                    return componentName;
                }
        };
        
        

        
};




#endif
