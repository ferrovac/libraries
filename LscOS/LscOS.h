/*
AUTHOR:       Tobias Hofmänner
DATE:         29.09.2023
USAGE:        TODO
DESCRIPTION:  TODO
TODO:       
RECOURCES:  TODO  
*/

#ifndef LSCOS_H
#define LSCOS_H

#include <Arduino.h>
#include "LscComponents.h"
#include "LscHardwareAbstraction.h"
#include "LscError.h"

namespace OS{

  void init();
  void startWatchdog();
  void stopWatchdog();
}


#endif