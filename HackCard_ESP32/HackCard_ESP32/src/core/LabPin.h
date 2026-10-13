#pragma once

#include <stddef.h>

bool labPinProvided(const char* pin);
bool labPinMatches(const char* pin);
bool labPinCheck(const char* pin, char* error, size_t errorLen);
