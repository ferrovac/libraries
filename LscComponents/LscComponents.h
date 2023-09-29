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
#include "LscHardwareAbstraction.h"

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
        static std::vector<String> getOptions(){
            return {"mbar", "Torr mmHg", "Pounds Per Square Inch", "Atmospheres"};
        }
        static String getSuffix(Unit unit){
            const String suffix[]= {"mbar", "Pa","torr", "psi", "atm"};
            return suffix[static_cast<int>(unit)];
        }
    };
};

//The class privides a base that all components should be derived from. 
class BaseComponent{
    public:
        virtual void update() = 0;
        virtual String getComponentStateXML() = 0;
        virtual String const getComponentType() const = 0;
        virtual String const getComponentName() const = 0;
        virtual void setComponentName(const String& name) = 0;
};

class Components{
    public:
        class TemperatureSensor : BaseComponent {
            private:
                String componentName;
                AnalogInPt100 &analogInPt100;
                double temperature;
                Units::Temperature::Unit displayUnit;
            public:
                TemperatureSensor(AnalogInPt100 &analogInPt100, String componentName = "genericTemperatureSensor") : analogInPt100(analogInPt100), componentName(componentName) {
                    temperature = 0;
                    displayUnit = Units::Temperature::Unit::C;
                    ComponentTracker::getInstance().registerComponent(this);
                }
                double getTemperature(){
                    update();
                    return temperature;
                }
                String getTeperatureAsString() {
                    update();
                    if(displayUnit == Units::Temperature::Unit::K){
                        return String(temperature) + " " + Units::Temperature::getSuffix(Units::Temperature::Unit::K);
                    }else if(displayUnit == Units::Temperature::Unit::C){
                        return String(temperature - 273.15) + " " + Units::Temperature::getSuffix(Units::Temperature::Unit::C);
                    }else if(displayUnit == Units::Temperature::Unit::F){
                        return String(temperature * (9./5.) - 459.67) + " " + Units::Temperature::getSuffix(Units::Temperature::Unit::F);
                    }
                    return "Error";  
                }
                void setDisplayUnit(Units::Temperature::Unit unit){
                    displayUnit = unit;
                }
            
                void update() override {
                    temperature = analogInPt100.getTemperature();
                }
                String getComponentStateXML() override{
                    String xml = "<component>\n";
                    xml += "<type>" + getComponentType() + "</type>\n";
                    xml += "<name>" + getComponentName() + "</name>\n";
                    xml += "<temperature>" + String(temperature) + "</temperature>\n";
                    xml += "<temperatureAsString>" + getTeperatureAsString() + "</temperatureAsString>\n";
                    xml += "</component>";
                    return xml;
                }

                String const getComponentType() const override {
                    return "TemperatureSensor";
                }
                String const getComponentName() const override{
                    return componentName;
                }
                void setComponentName(const String& name) override{
                    componentName = name;
                }
        };
        class ComponentTracker{
            private:
                std::vector<BaseComponent*> components;
                ComponentTracker() {}

            public:

                static ComponentTracker& getInstance() {
                    static ComponentTracker instance;  // Guaranteed to be created once
                    return instance;
                }

                std::vector<BaseComponent*> getComponets(){
                    return components;
                }
                
                void registerComponent(BaseComponent* component){
                    components.push_back(component);

                }

                std::vector<String> listAllComponentNames(){
                    std::vector<String> componentList;
                    for(BaseComponent* comp : components){
                        componentList.push_back(comp->getComponentName());
                    }
                    return componentList;
                }
                
        };
};


#endif