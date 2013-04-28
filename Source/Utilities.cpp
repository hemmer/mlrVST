/*
  ==============================================================================

    Utilities.cpp
    Created: 24 Apr 2013 10:48:01pm
    Author:  hemmer

  ==============================================================================
*/

#include "Utilities.h"
#include <cmath>

int round(float x)
{
    return static_cast<int>(floor(x + 0.5f));
}