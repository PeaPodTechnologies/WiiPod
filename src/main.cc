#ifndef UNIT_TEST
#define UNIT_TEST 1
#define IS_MAIN 1

// INCLUDES & MACROS

#include <Arduino.h>

#include "../test/config.h"

// Uncomment to enable debug
// #define DEBUG 0
#include <debug.h>
#define WIIPOD_DEBUG_SERIAL Serial // This file only

#include <chrono.h>
#include <I2CIP.h>
#include <wiipod.h>
#include <HT16K33.h>

// #include <avr/wdt.h>

#define TIMESTAMP_LOG_DELTA_MS 5000 // Default
#define FSM_CYCLE_DELTA_MS     3000  // 1 FPS
#define WIRENUM 0
#define MODULE  0

// DECLARATIONS
extern Chronograph Chronos;

using namespace I2CIP;

// Module* m;  // to be initialized in setup()
// Module* modules[I2CIP_MUX_COUNT] = { nullptr };
char idbuffer[10];

Variable cycle(Number(0, false, false), "Cycle");
void logCycle(bool _, const Number& cycle);

WiiPod* wiipod = nullptr;
HT16K33 *ht16k33 = nullptr;
i2cip_fqa_t fqa_sht45 = I2CIP::createFQA(WIRENUM, MODULE, 0, I2CIP_SHT45_ADDRESS);
i2cip_fqa_t fqa_pca9685 = I2CIP::createFQA(WIRENUM, MODULE, 1, I2CIP_PCA9685_ADDRESS);
i2cip_fqa_t fqa_pca9632 = I2CIP::createFQA(WIRENUM, MODULE, 1, I2CIP_PCA9632_ADDRESS);

// HELPERS

// int freeRam() {
//   extern int __heap_start,*__brkval;
//   int v;
//   return (int)&v - (__brkval == 0  
//     ? (int)&__heap_start : (int) __brkval);  
// }

unsigned long lastCycle = 0;
void logCycle(bool _, const Number& cycle) {
  unsigned long delta = millis() - lastCycle;
  #ifdef FSM_CYCLE_DELTA_MS
  if(delta < FSM_CYCLE_DELTA_MS) {
    delay(FSM_CYCLE_DELTA_MS - delta);
  }
  #endif
  lastCycle = millis();

  String m = _F("[ WIIPOD ");
  m += cycle.toString();
  m += _F(" | ");
  m += timestampToString(millis());
  // if(delta < FSM_CYCLE_DELTA_MS) { 
    m += " | ";
    // m += (FSM_CYCLE_DELTA_MS - delta);
    m += 1000.0 / delta;
    m += " FPS";
  // }
  // m += " | SRAM: ";
  // m += freeRam();
  m += _F(" ]");

  WIIPOD_DEBUG_SERIAL.println(m);
}

void crashout(void) {
  while(true) { // Blink
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void setup(void) {
  Serial.begin(115200);
  cycle.addLoggerCallback(logCycle);

  // Module
  wiipod = new WiiPod(WIRENUM, MODULE);
  i2cip_errorlevel_t errlev = wiipod->check(); // Discover, Ping
  if(errlev != I2CIP_ERR_NONE) crashout();

  // Status Display
  ht16k33 = new HT16K33(WIRENUM, I2CIP_MUX_NUM_FAKE, I2CIP_MUX_BUS_FAKE, "SEVENSEG");
  errlev = wiipod->operator()<HT16K33>(ht16k33, false, _i2cip_args_io_default, DebugJsonBreakpoints);
  if(errlev != I2CIP_ERR_NONE) crashout();
}

i2cip_errorlevel_t errlev;
double fps = 0.0;
void loop(void) {

  // 0. Clock, Cycle, Delay
  unsigned long now = millis(); Chronos.set(now); cycle.set(cycle.get()++);

  // 1. Update WiiPod: SHT45 -> Temperature Cache -> 7-Seg, PWM, LCD
  // errlev = wiipod->update();
  errlev = wiipod->operator()<SHT45>(fqa_sht45, true, _i2cip_args_io_default, DebugJsonBreakpoints);

  if(errlev != I2CIP_ERR_NONE) {
    // FAIL
    errlev = wiipod->operator()<HT16K33>(ht16k33, true, _i2cip_args_io_default, DebugJsonBreakpoints);
  } else {
    // DISPLAY TEMPERATURE AND DO PWM
    DeviceGroup* sht45 = wiipod->getDeviceGroup("SHT45");
    if(sht45 != nullptr && sht45->numdevices > 0) {
      // AVERAGES
      state_sht45_t state = {0.0f, 0.0f};
      for(uint8_t i = 0; i < sht45->numdevices; i++) {
        state.temperature += ((SHT45*)(sht45->devices[i]))->getCache().temperature;
        state.humidity += ((SHT45*)(sht45->devices[i]))->getCache().humidity;
      }
      state.temperature /= sht45->numdevices; state.humidity /= sht45->numdevices;

      // SEVENSEG
      i2cip_ht16k33_mode_t mode = SEG_1F;
      // i2cip_ht16k33_data_t data = { .f = temphum ? state.temperature : state.humidity };
      i2cip_ht16k33_data_t data = { .f = state.temperature };
      i2cip_args_io_t hargs = { .a = nullptr, .s = &data.f, .b = &mode };
      // errlev = 
      errlev = wiipod->operator()<HT16K33>(ht16k33, true, hargs, DebugJsonBreakpoints);
      // temphum = !temphum;

      // PWM
      uint16_t pwm12 = (uint16_t)(0xFFF * max(0.f, state.temperature) / 100.f); // Celsius to PWM
      i2cip_pca9685_chsel_t c = PCA9685_CH0;
      i2cip_args_io_t pargs = { .a = nullptr, .s = &pwm12, .b = &c };
      errlev = modules[MODULE]->operator()<PCA9685>(fqa_pca9685, true, pargs, DebugJsonBreakpoints);

      // LCD
      String msg = "HI PEAPOD :D\nT: " + String(state.temperature, 1) + "C PWM: " + String((pwm12 / (float)0xFFF) * 100, 1) + "%";
      i2cip_args_io_t largs = { .a = nullptr, .s = &msg, .b = nullptr };
      errlev = modules[MODULE]->operator()<PCA9632>(fqa_pca9632, true, largs, DebugJsonBreakpoints);
    }
  }

  unsigned long delta = millis() - now;
  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print(F("[ WIIPOD "));
    WIIPOD_DEBUG_SERIAL.print(cycle.get().toString());
    WIIPOD_DEBUG_SERIAL.print(F(" | "));
    WIIPOD_DEBUG_SERIAL.print(1000.0 / delta, 0);
    WIIPOD_DEBUG_SERIAL.print(F(" FPS | 0x"));
    WIIPOD_DEBUG_SERIAL.print(errlev, HEX);
    WIIPOD_DEBUG_SERIAL.println(F("]"));
  #endif

  // delay(1000 / 30); // 30 FPS
}

#endif