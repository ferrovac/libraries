/*
AUTHOR: Tobias Hofmänner
DATE:   14.07.2023
USAGE:  This class contains all necessary logic for error hanlding to use this class define the following memeo in your code:
              #include "LSC_ERROR.h"
              #define ERROR_HANDLER ErrorHandler::getInstance()
        Then you can use all functions in the Assert struct for input validation:
              ERROR_HANDLER.Assert.isValidDACValue()
        This will throw an error and acction will be taken depending on the SeverityLevel (see Assert Struct)
        To add a custom Assert function simply add the new validation function to the Assert Struct.
TODO:   Define some meaningfull error codes
*/

#ifndef LscError_H
#define LscError_H

#include <Arduino.h>
#include <vector>

// Enumeration representing the different Severity Levels of errors
enum struct SeverityLevel {
  NORMAL, // The program can continue normally, and the error will be logged.
  CRITICAL, // The program cannot continue, and the calling function will be terminated.
  FATAL // The program cannot continue, and the system cannot recover. A complete restart is required.
};

// Structure representing an error with its associated information.
struct Error {
    const int errorCode;
    const String errorMessage;
    const SeverityLevel severityLevel;
    Error(int errorCode, const String& errorMessage, const SeverityLevel severityLevel) : errorCode(errorCode), errorMessage(errorMessage), severityLevel(severityLevel){}
};

/* 
Class responsible for handling errors and managing instances.
The class is designed as singelton i.e. there can always be only one instance of this class. the errors container is static this means that all instances share the same container (we only have one instance anyway)
This desige allows every object to throw errors that then can be handled by the os.
 */
class ErrorHandler {
  private:
    static std::vector<Error> errors; // Static container to strore error instances. 
    ErrorHandler(){} // Private constructor to prevent direct external instantiation, following the singelton pattern.

  public:
    // Function that return the instance of ErrorHandler (if the instance does not exist jet it is created)
    static ErrorHandler& getInstance(){ 
      static ErrorHandler instance;     // Public member holding the single instance of ErrorHandler with lazy inizialisation of the singelton
      return instance;
    }

    // Function to add an Error to the errors container
    // errorCode: Error Code of your exception (not jet clearly definded 0 is fine for now)
    // errorMessage: A meaningfull error message explaining why the exception was thrown and what the current stat is the system is. This will be displayed to the user in some form.
    // severityLevel: SeverityLevel::NORMAL, SeverityLevel::CRITICAL, SeverityLevel::FATAL
    static void throwError(int errorCode, const String& errorMessage, const SeverityLevel serverityLevel) {
        errors.push_back(Error(errorCode, errorMessage, serverityLevel));
        if(serverityLevel == SeverityLevel::CRITICAL || serverityLevel == SeverityLevel::FATAL){ // Stop program execution if the error is CRITICSL or FATAL
          while(true){} // Endless Loop to halt programm execution.
        }
    }

    // Returns the vector containing the list of errors
    static const std::vector<Error>& getErrors() {
      return errors;
    }
    static void clearAll(){
      errors.clear();
    }
};


/*
Add custom Assert functions here. use the ErrorHandler::throwError function th throw an error.
There are 3 different SeverityLevles available:
  SeverityLevel::NORMAL     ->      The programm continues without intervention, but the os will be mad aware that the error happened for logging purposes
  SeverityLevel::CRITICLA   ->      The programm halts immediately (enter endless loop) and the os is notified. The os can then decide how to proceed. Generally the idea is that the os terminates the execution of the function that caused
                                    the exception and returns to a well defined state.
  SeverityLevel::FATAL      ->      The programm halts immediately (enter endless loop) in this case save execution of the programm is not possible and even the os can not return the controller to a well defined state. In this case the os
                                    will notify the user of the error and instruct the user to performe a hard reset.
*/

// Structure containing an assortment of functions to perform input validation
struct Assert{
  // validates input for a DAC, this is considered a NORMAL error
  static void isValidDACValue(int value){
    if(value < 0 || value > 4096)
    ErrorHandler::throwError(0, "Provided value for DAC no within the allowed range. Only values form 0 to 4096 are allowed, " + String(value) + " given.", SeverityLevel::NORMAL );  
  }

  // checks whether the analogReadResolution is set to 12 bit, if not a NORMAL error is raised
  static void adcResolutionIs12bit(){
    if( DACC_RESOLUTION != 12){ 
      ErrorHandler::throwError(0, "analogReadResolution has to be set to 12bit for all analogRead operations, but is not set to 12bit!", SeverityLevel::NORMAL );
    }
  }
  
  // checks whether the analogReadResolution is set to 12 bit, if not a NORMAL error is raised
  static void dacResolutionIs12bit(){
    if( PWM_RESOLUTION !=12){
     // ErrorHandler::throwError(0, "analogWriteResolution has to be set to 12bit for all analogWrite operations, but is not set to 12bit!", SeverityLevel::NORMAL );
    }
  }

//TODO:COMPLETE THIS FUNCTION
// checks whether a higer than allowed voltage is supplied to an analog input, if it is the case a FATAL error is raised
//description: Object description, helps the user to locate the device that is causing trouble
//voltage: The measured voltage on the input
//criticalVoltageThreshold: The max allowed voltage for the input
  static void overVoltageCondition(String& description, double voltage, double criticalVoltageThreshold){
    if( voltage >= criticalVoltageThreshold){
      ErrorHandler::throwError(0, "The voltage on\n" + description + " \nis to high. To prevent damage to the device disconnect it immediately!", SeverityLevel::FATAL );
    }
  }

};

#endif