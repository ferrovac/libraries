#include "LscPersistence.h"

volatile bool BasePersistent::initComplete = false;
volatile bool PersistentTracker::powerFailureImminent = false;