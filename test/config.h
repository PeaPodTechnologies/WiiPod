#ifndef I2CIP_TESTS_TEST_H_
#define I2CIP_TESTS_TEST_H_

#define DEBUG 0
#include "../src/debug.h"

// TESTING PARAMETERS

#define I2CIP_TEST_BUFFERSIZE 100 // Need to limit this, or else crash; I think Unity takes up a lot of stack space

// // Check if we're on ESP32
// #ifdef ESP32
//   SET_LOOP_TASK_STACK_SIZE( 32*1024 ); // Thanks to: https://community.platformio.org/t/esp32-stack-configuration-reloaded/20994/8; https://github.com/espressif/arduino-esp32/pull/5173
// #endif

// #define WIIPOD_BUS_NUNCHUCK 0
#define WIIPOD_BUS_SHT45  1
#define WIIPOD_BUS_K30 1
#define WIIPOD_BUS_MCP 0
#define WIIPOD_BUS_ROTARY 0

#endif