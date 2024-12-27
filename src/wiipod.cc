#include <wiipod.h>

#include <Nunchuck.h>
// #include <sht31.h>
#include <SHT45.h>
#include <K30.h>
#include <MCP23017.h>
#include <Seesaw.h>
#include <PCA9685.h>
#include <ADS1115.h>
#include <SSD1306.h>
#include <PCA9632.h>
#include "./debug.h"

#ifdef WIIPOD_DEBUG_SERIAL
#ifndef WIIPOD_ERROR_SERIAL
#define WIIPOD_ERROR_SERIAL WIIPOD_DEBUG_SERIAL
#endif
#ifndef WIIPOD_WARNING_SERIAL
#define WIIPOD_WARNING_SERIAL WIIPOD_DEBUG_SERIAL
#endif
#endif

#define WIIPOD_DEVICEGROUP_BREAK(dg) if(dg != nullptr) { return dg; }

using namespace I2CIP;

WiiPod::WiiPod(const uint8_t& wire, const uint8_t& module) : Module(wire, module) { }

DeviceGroup* WiiPod::deviceGroupFactory(const i2cip_id_t& id) {
  DeviceGroup* dg = nullptr;

  dg = DeviceGroup::create<EEPROM>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);

  dg = DeviceGroup::create<Nunchuck>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);

  // dg = DeviceGroup::create<SHT31>(id);
  // WIIPOD_DEVICEGROUP_BREAK(dg);

  dg = DeviceGroup::create<SHT45>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);

  dg = DeviceGroup::create<K30>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);

  dg = DeviceGroup::create<MCP23017>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);

  dg = DeviceGroup::create<Seesaw_RotaryEncoder>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);

  dg = DeviceGroup::create<PCA9685>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);

  dg = DeviceGroup::create<ADS1115>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);

  dg = DeviceGroup::create<PCA9632>(id);
  WIIPOD_DEVICEGROUP_BREAK(dg);
  
  return DeviceGroup::create<SSD1306>(id);
}

bool WiiPod::initialize(void) {
  if(initialized) return true;

  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.println(F("[MODULE | INIT] DISCOVERY START"));
  #endif
  // Serial.print(F("[I2CIP] MODULE "));
  // Serial.print(wirenum);
  // Serial.print(":");
  // Serial.print(modulenum);
  // Serial.print(F(":.:. | INIT: "));

  // if(modules[modulenum] != nullptr) {
  //   // Serial.print(F("(DELETING) "));
  //   // delete modules[modulenum];
  //   return;
  // }

  // Initialize module
  // modules[modulenum] = &WiiPod(WIRENUM, MODULE);
  unsigned long now = millis();
  i2cip_errorlevel_t errlev = this->discoverEEPROM(); // Discovery adds eeprom to data tables, reads and parses contents
  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
      #ifdef WIIPOD_ERROR_SERIAL
      WIIPOD_ERROR_SERIAL.println(F("[MODULE | INIT] FAIL EIO"));
      #endif
      return false;
    case I2CIP::I2CIP_ERR_SOFT:
      #ifdef WIIPOD_WARNING_SERIAL
      WIIPOD_WARNING_SERIAL.println(F("[MODULE | INIT] FAIL EINVAL"));
      #endif
      return false;
    default:
      break;
  }

  bool r = I2CIP::MUX::pingMUX((this->eeprom->getFQA()));
  unsigned long delta = millis() - now;
  if(!r) {
    #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.println(F("[MODULE | INIT] MUX FAIL EIO"));
    #endif
    return false;
  }
  #ifdef WIIPOD_DEBUG_SERIAL
    String m = _F("[MODULE | INIT] DISCOVERY PASS ");
    m += String(delta / 1000.0, 3);
    m += _F("s");
    WIIPOD_DEBUG_SERIAL.println(m);
  #endif

  initialized = true;

  return true;
}

i2cip_errorlevel_t WiiPod::check() {
  if(!initialized) {
    if(!initialize()) {
      return I2CIP::I2CIP_ERR_HARD;
    }
  }
  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.println(F("[MODULE | CHECK] START"));
  #endif

  unsigned long now = millis();
  // i2cip_errorlevel_t errlev = modules[MODULE]->operator()();
  i2cip_errorlevel_t errlev = this->operator()();
  unsigned long delta = millis() - now;

  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
      #ifdef WIIPOD_ERROR_SERIAL
        WIIPOD_ERROR_SERIAL.println(F("[MODULE | CHECK] FAIL EIO"));
      #endif
      initialized = false;
      break;
    case I2CIP::I2CIP_ERR_SOFT:
      #ifdef WIIPOD_WARNING_SERIAL
        WIIPOD_WARNING_SERIAL.println(F("[MODULE | CHECK] FAIL EINVAL"));
      #endif
      break;
    default:
      #ifdef WIIPOD_DEBUG_SERIAL
        String m = _F("[MODULE | CHECK] PASS ");
        m += String(delta / 1000.0, 3);
        m += _F("s");
        WIIPOD_DEBUG_SERIAL.println(m);
      #endif
      break;
  }
  return errlev;
}

i2cip_errorlevel_t WiiPod::update() {
  if(!initialized) {
    if(!initialize()) {
      return I2CIP::I2CIP_ERR_HARD;
    }
  }

  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.println(F("[WIIPOD | UPDATE] START"));
  #endif
  // Serial.print(wirenum);
  // Serial.print(":");
  // Serial.print(modulenum);
  // Serial.print(F(":.:. | UPDATE: "));

  // if(modules[modulenum] == nullptr) {
  //   Serial.println(F("FAIL ENOENT"));
  //   return I2CIP::I2CIP_ERR_SOFT; // ENOENT
  // }

  // const I2CIP::EEPROM& eeprom = *(this->eeprom);
  i2cip_fqa_t eeprom_fqa = this->eeprom->getFQA();

  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | UPDATE] EEPROM "));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(eeprom_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(":");
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(eeprom_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(":");
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(eeprom_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(":");
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(eeprom_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.println(" START");
  #endif

  unsigned long now = millis();
  // i2cip_errorlevel_t errlev = modules[modulenum]->operator()(eeprom.getFQA(), true, true); // Test failsafe get
  #ifdef DEBUG_JSON
  i2cip_errorlevel_t errlev = this->operator()<EEPROM>(eeprom_fqa, true, _i2cip_args_io_default, DebugJsonBreakpoints); // Test failsafe get
  #else
  i2cip_errorlevel_t errlev = this->operator()<EEPROM>(eeprom_fqa, true); // Test failsafe get
  #endif
  unsigned long delta = millis() - now;

  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
      #ifdef WIIPOD_ERROR_SERIAL
        WIIPOD_ERROR_SERIAL.println(F("[MODULE | UPDATE] EEPROM FAIL EIO"));
      #endif
      return I2CIP::I2CIP_ERR_HARD;
    case I2CIP::I2CIP_ERR_SOFT:
      #ifdef WIIPOD_WARNING_SERIAL
        WIIPOD_WARNING_SERIAL.println(F("[MODULE | UPDATE] EEPROM FAIL EINVAL"));
      #endif
      return I2CIP::I2CIP_ERR_SOFT;
    default:
      #ifdef WIIPOD_DEBUG_SERIAL
        String m = _F("[MODULE | UPDATE] PASS ");
        m += String(delta / 1000.0, 3);
        m += _F("s");
        WIIPOD_DEBUG_SERIAL.println(m);
      #endif
      break;
  }

  const char* cache = this->eeprom->getCache();
  if(cache == nullptr || cache[0] == '\0') {
    return errlev;
  }

  #ifdef DEBUG_JSON
  DebugJson::telemetry(now + delta, cache, "eeprom", Serial); // Send data: {eeprom: $CACHE}
  #endif

// Uncomment to perform scan
  // #ifdef WIIPOD_DEBUG_SERIAL
  //   this->scanToPrint(WIIPOD_DEBUG_SERIAL);
  // #endif

  return errlev;
}

void WiiPod::scanToPrint(Print& out) {
  String m = F("[MODULE | SCAN] ");
  m += String(getWireNum(), HEX);
  m += ':';
  m += String(getModuleNum(), HEX);
  m += F(" START (~10s)");
  out.println(m);
  for(uint8_t b = 0; b < 0b111; b++) {
    m = F("[MODULE | SCAN] BUS ");
    m += String(b, HEX);
    m += F(" START");
    out.println(m);
    bool first = true;
    for(uint8_t a = 0x08; a < 0b1111111; a++) {
      // 7 bit i2c address; 0x00 - 0x07 reserved
      if(a >= I2CIP_MUX_ADDR_MIN && a <= I2CIP_MUX_ADDR_MAX) { continue; }
      if(a >= 0x78 && a <= 0x7F) { continue; } // 0x78 - 0x7F reserved
      i2cip_fqa_t fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), b, a);
      i2cip_errorlevel_t errlev = Device::pingTimeout(fqa, first, false, 10);
      first = false;
      if(errlev == I2CIP_ERR_NONE) {
        m = F("[MODULE | SCAN] FOUND ");
        m += fqaToString(fqa);
        out.println(m);
      }
      delay(1);
    }
  }
}