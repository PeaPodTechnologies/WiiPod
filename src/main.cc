#ifndef UNIT_TEST

#include <Arduino.h>

#include <debug.h>
#include <chrono.h>
#include <I2CIP.h>
#include <WiiChuck.h>

#include <avr/wdt.h>

#define TIMESTAMP_LOG_DELTA_MS 5000 // Default
#define FSM_CYCLE_DELTA_MS     100
#define WIRENUM 0
#define MODULE  0

I2CIP::i2cip_errorlevel_t errlev;
Variable* cycle;
void logCycle(bool _, const Number& cycle);
void logTimestamp(bool _, const fsm_timestamp_t& timestamp);
// void logDisco(bool _, const bool& on);

I2CIP::Module* modules[I2CIP_MUX_COUNT] = { nullptr };
char idbuffer[10];

Accessory nunchuck1;

void crashout(void) {
  while(true) { // Blink
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

bool initializeModule(uint8_t wirenum, uint8_t modulenum);
i2cip_errorlevel_t checkModule(uint8_t wirenum, uint8_t modulenum);
i2cip_errorlevel_t updateModule(uint8_t wirenum, uint8_t modulenum);

void setup() {
  // Serial
  Serial.begin(115200);
  while(!Serial);

  // Construct States
  cycle = new Variable(Number(0, false, false), "Cycle");
  cycle->addLoggerCallback(logCycle);
  nunchuck1.begin();

  // Chronos.addInterval(TIMESTAMP_LOG_DELTA_MS, &logTimestamp);

  // DebugJson::debugSRAM();
  
  #ifdef _AVR_WDT_H_
    wdt_enable(WDTO_4S);
  #endif

  // OPTIONAL: FIRST TIME INIT
  bool r = initializeModule(WIRENUM, MODULE);
  if (!r) { delete modules[MODULE]; modules[MODULE] = nullptr; crashout(); }
}

void loop() {
  #ifdef _AVR_WDT_H_
    wdt_reset();
  #endif

  cycle->set(cycle->get() + Number(1, false, false));

  // Watchdog Timer Kickout
  // if(now > TWENTYFOURHRS_MILLIS) { while(true) { delay(1); } }

  Chronos.set(millis());

  unsigned long i2cip_start = millis();
  switch(checkModule(WIRENUM, MODULE)) {
    case I2CIP::I2CIP_ERR_HARD:
      delete modules[MODULE];
      modules[MODULE] = nullptr;
      return;
    case I2CIP::I2CIP_ERR_SOFT:
      if (!initializeModule(WIRENUM, MODULE)) { delete modules[MODULE]; modules[MODULE] = nullptr; return; }
      break;
    default:
      errlev = updateModule(WIRENUM, MODULE);
      break;
  }
  unsigned long i2cip_end = millis();

  // DEBUG PRINT: CYCLE COUNT, FPS, and ERRLEV
  Serial.print(F("[I2CIP | CYCLE "));
  Serial.print(cycle->get());
  Serial.print(F(" | "));
  Serial.print(1000.0 / (i2cip_end - i2cip_start), 0);
  Serial.print(F(" FPS | 0x"));
  Serial.print(errlev, HEX);
  Serial.println(F("]"));
}

unsigned long lastCycle = 0;

void logCycle(bool _, const Number& cycle) {
  String m = _F("==== [ Cycle: ");
  m += cycle.toString();
  m += _F(" @ ");
  m += timestampToString(millis());

  unsigned delta = millis() - lastCycle;
  unsigned d = 0;
  if(delta < FSM_CYCLE_DELTA_MS) {
    d = FSM_CYCLE_DELTA_MS - delta;
    delay(d);
  }
  if(d > 0) {
    m += _F(" + ");
    m += d;
    m += _F("ms");
  }
  m += _F(" ] ====");

  Serial.println(m);
}

bool initializeModule(uint8_t wirenum, uint8_t modulenum) {
  Serial.print(F("[I2CIP] MODULE "));
  Serial.print(wirenum);
  Serial.print(":");
  Serial.print(modulenum);
  Serial.print(F(":.:. | INIT: "));

  if(modules[modulenum] != nullptr) {
    Serial.print(F("(DELETING) "));
    delete modules[modulenum];
  }

  // Initialize module
  unsigned long now = millis();
  modules[modulenum] = new I2CIP::Module(WIRENUM, MODULE);
  unsigned long delta = millis() - now;

  if(modules[modulenum] == nullptr) { 
    Serial.println(F("FAIL UNREACH"));
    return false;
  } else if((I2CIP::EEPROM*)(modules[modulenum]) == nullptr) {
    Serial.println(F("FAIL EEPROM"));
    delete modules[modulenum];
    modules[modulenum] = nullptr;
    return false;
  }

  Serial.print(delta / 1000.0, 3);
  Serial.println("s");

  return true;
}

i2cip_errorlevel_t checkModule(uint8_t wirenum, uint8_t modulenum) {
  Serial.print(F("[I2CIP] MODULE "));
  Serial.print(wirenum, HEX);
  Serial.print(":");
  Serial.print(modulenum, HEX);
  Serial.print(F(":.:. | CHECK: "));

  if(modules[modulenum] == nullptr) {
    Serial.println(F("FAIL ENOENT"));
    return I2CIP::I2CIP_ERR_SOFT; // ENOENT
  }

  unsigned long now = millis();
  i2cip_errorlevel_t errlev = modules[MODULE]->operator()();
  unsigned long delta = millis() - now;

  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
      Serial.print(F("FAIL EIO "));
      break;
    case I2CIP::I2CIP_ERR_SOFT:
      Serial.print(F("FAIL EINVAL "));
      break;
    default:
      Serial.print(F("PASS "));
      break;
  }
  Serial.print(delta / 1000.0, 3);
  Serial.println(F("s"));
  // I2CIP_ERR_BREAK(errlev);

  // // Continue - EEPROM check
  // i2cip_fqa_t eeprom = ((EEPROM*)modules[MODULE])->getFQA();

  // Serial.print(F("[I2CIP] EEPROM "));
  // Serial.print(wirenum);
  // Serial.print(":");
  // Serial.print(modulenum);
  // Serial.print(":");
  // Serial.print(I2CIP_FQA_SEG_MUXBUS(eeprom));
  // Serial.print(":");
  // Serial.print(I2CIP_FQA_SEG_DEVADR(eeprom));
  // Serial.print(F(" - CHECK: "));

  // now = millis();
  // errlev = modules[MODULE]->operator()(eeprom);
  // delta = millis() - now;

  // switch(errlev) {
  //   case I2CIP_ERR_HARD:
  //     Serial.print(F("FAIL EIO "));
  //     break;
  //   case I2CIP_ERR_SOFT:
  //     Serial.print(F("FAIL EINVAL "));
  //     break;
  //   case I2CIP_ERR_NONE:
  //   default:
  //     Serial.print(F("PASS "));
  //     break;
  // }
  // Serial.print(delta / 1000.0, 3);
  // Serial.println(F("s"));
  return errlev;
}

i2cip_errorlevel_t updateModule(uint8_t wirenum, uint8_t modulenum) {
  Serial.print(F("[I2CIP] MODULE "));
  Serial.print(wirenum);
  Serial.print(":");
  Serial.print(modulenum);
  Serial.print(F(":.:. | UPDATE: "));

  if(modules[modulenum] == nullptr) {
    Serial.println(F("FAIL ENOENT"));
    return I2CIP::I2CIP_ERR_SOFT; // ENOENT
  }

  const I2CIP::EEPROM& eeprom = modules[modulenum]->operator const I2CIP::EEPROM &();

  Serial.print(F("EEPROM "));
  Serial.print(I2CIP_FQA_SEG_I2CBUS(eeprom.getFQA()), HEX);
  Serial.print(":");
  Serial.print(I2CIP_FQA_SEG_MODULE(eeprom.getFQA()), HEX);
  Serial.print(":");
  Serial.print(I2CIP_FQA_SEG_MUXBUS(eeprom.getFQA()), HEX);
  Serial.print(":");
  Serial.print(I2CIP_FQA_SEG_DEVADR(eeprom.getFQA()), HEX);
  Serial.print(" ");

  unsigned long now = millis();
  i2cip_errorlevel_t errlev = modules[modulenum]->operator()(eeprom.getFQA(), true);
  unsigned long delta = millis() - now;

  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
      Serial.print(F("FAIL EIO "));
      break;
    case I2CIP::I2CIP_ERR_SOFT:
      Serial.print(F("FAIL EINVAL "));
      break;
    default:
      Serial.print(F("PASS "));
      break;
  }
  Serial.print(delta / 1000.0, 3);
  Serial.print(F("s"));
  if(errlev != I2CIP::I2CIP_ERR_NONE) {
    Serial.println();
    return errlev;
  }

  // Bonus Points - Print EEPROM contents
  const char* cache = eeprom.getCache();
  if(cache == nullptr || cache[0] == '\0') {
    Serial.println(F(" EMPTY"));
    return errlev;
  }

  Serial.print(F(" \""));
  Serial.print(cache);
  Serial.println(F("\""));

  return errlev;
}

#endif