#ifndef WIIPOD_DEBUG_SERIAL

#include <Arduino.h>

#ifndef DEBUG
#define DEBUG 1 // Uncomment to enable debug
#endif

// BASIC DEBUG MACRO
#ifdef DEBUG
#if DEBUG == true
#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif
#endif
#endif

#include <DebugJson.h>

// CROSS-LIBRARY DEBUG COMPATIBILITY
// #ifdef DEBUG_SERIAL
#define WIIPOD_DEBUG_SERIAL DebugJsonOut
#define WIIPOD_WARNING_SERIAL DebugJsonWarning
#define WIIPOD_ERROR_SERIAL DebugJsonError
// #define WIIPOD_DEBUG_SERIAL Serial
// #endif

// DEBUG DELAY MACRO FOR SERIAL OUTPUT STABILITY (OPTIONAL)
#ifdef WIIPOD_DEBUG_SERIAL
// #include <DebugJson.h>
#ifndef DEBUG_DELAY
#define DEBUG_DELAY() {delay(1);}
// #define DEBUG_DELAY() {delayMicroseconds(10);}
#endif
#endif

#ifndef _F
#ifdef DEBUG_DISABLE_FSTRINGS
#if DEBUG_DISABLE_FSTRINGS == 1
#define _F(x) x
#endif
#endif
#define _F(x) F(x)
#endif

#endif

#include "../lib/I2CIP/src/debug.h"
#include "../lib/FiniteStateMachine/src/debug.h"
