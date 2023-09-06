/*
Metadata in
*/

#include "./LscError.h"

// This line defines the static member variable 'errors' of the ErrorHandler class.
// The static member variable is defined outside of any class or function definition.
// It is associated with the ErrorHandler class and shared among all instances of the class.
// Here, it is of type std::vector<Error>, which represents a container for storing Error objects.

std::vector<Error> ErrorHandler::errors;