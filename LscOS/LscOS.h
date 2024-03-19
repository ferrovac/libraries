/*  Main arduino programm that controlls the Ferrovac GloveBox 
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

#ifndef LSCOS_H
#define LSCOS_H

#include <Arduino.h>
#include "LscComponents.h"
#include "LscPersistence.h"
#include "LscHardwareAbstraction.h"
#include "LscError.h"
#include <SD.h>


namespace OS{
  bool getBootUpState();
  void init(String Version);
  void startWatchdog();
  void stopWatchdog();
  uint32_t getCycleCount();
  uint32_t getNextOsCall_ms();
  bool saveToRead();
  static volatile bool powerFailureImminent = false;

}




#endif