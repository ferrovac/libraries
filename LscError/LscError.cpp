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

#include "LscError.h"

// This line defines the static member variable 'errors' of the ErrorHandler class.
// The static member variable is defined outside of any class or function definition.
// It is associated with the ErrorHandler class and shared among all instances of the class.
// Here, it is of type std::vector<Error>, which represents a container for storing Error objects.

std::vector<Error> ErrorHandler::errors;