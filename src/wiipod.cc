#include <wiipod.hpp>

#include <nunchuck.hpp>
#include <sht31.h>
#include "../src/debug.h"

using namespace I2CIP;

WiiPod::WiiPod(const uint8_t& wire, const uint8_t& module) : Module(wire, module) { }

bool refresh = false;
DeviceGroup* WiiPod::deviceGroupFactory(const i2cip_id_t& id) {
  // Pre-Load IDs
  Nunchuck::loadID();
  SHT31::loadID();
  refresh = true;

  if(id == EEPROM::getStaticIDBuffer() || strcmp(id, EEPROM::getStaticIDBuffer()) == 0) {
    return new DeviceGroup(EEPROM::getStaticIDBuffer(), EEPROM::eepromFactory);
  }
  if(id == Nunchuck::getStaticIDBuffer() || strcmp(id, Nunchuck::getStaticIDBuffer()) == 0) {
    return new DeviceGroup(Nunchuck::getStaticIDBuffer(), Nunchuck::nunchuckFactory);
  }
  if(id == SHT31::getStaticIDBuffer() || SHT31::getStaticIDBuffer()) {// strcmp(id, "SHT31") == 0) {
    return new DeviceGroup(SHT31::getStaticIDBuffer(), SHT31::sht31Factory);
  }

  return nullptr;
}

bool WiiPod::initialize(void) {
  if(initialized) return true;

  #ifdef WIIPOD_DEBUG_SERIAL
  WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD] INIT: "));
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
  bool r = this->discover(); // Discovery adds eeprom to data tables, reads and parses contents
  if(!r) {
    #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.println(F("FAIL EINVAL"));
    #endif
    return false;
  }
  unsigned long now = millis();
  r = I2CIP::MUX::pingMUX((this->eeprom->getFQA()));
  unsigned long delta = millis() - now;
  if(!r) {
    #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.println(F("FAIL EIO"));
    #endif
    return false;
  }
  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print(F("PASS "));
    WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
    WIIPOD_DEBUG_SERIAL.println(F("s"));
  #endif

  // if(modules[modulenum] == nullptr) { 
  //   Serial.println(F("FAIL UNREACH"));
  //   return false;
  // } else if((I2CIP::EEPROM*)(modules[modulenum]) == nullptr) {
  //   Serial.println(F("FAIL EEPROM"));
  //   delete modules[modulenum];
  //   modules[modulenum] = nullptr;
  //   return false;
  // }
  

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
    WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD] CHECK: "));
  #endif
  // Serial.print(F("[I2CIP] MODULE "));
  // Serial.print(wirenum, HEX);
  // Serial.print(":");
  // Serial.print(modulenum, HEX);
  // Serial.print(F(":.:. | CHECK: "));

  // if(modules[modulenum] == nullptr) {
  //   Serial.println(F("FAIL ENOENT"));
  //   return I2CIP::I2CIP_ERR_SOFT; // ENOENT
  // }

  unsigned long now = millis();
  // i2cip_errorlevel_t errlev = modules[MODULE]->operator()();
  i2cip_errorlevel_t errlev = this->operator()();
  unsigned long delta = millis() - now;

  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
      #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.print(F("FAIL EIO "));
      #endif
      initialized = false;
      break;
    case I2CIP::I2CIP_ERR_SOFT:
      #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.print(F("FAIL EINVAL "));
      #endif
      break;
    default:
      #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.print(F("PASS "));
      #endif
      break;
  }
  #ifdef WIIPOD_DEBUG_SERIAL
  WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
  WIIPOD_DEBUG_SERIAL.println(F("s"));
  #endif
  return errlev;
}

i2cip_errorlevel_t WiiPod::update() {
  if(!initialized) {
    if(!initialize()) {
      return I2CIP::I2CIP_ERR_HARD;
    }
  }

  #ifdef WIIPOD_DEBUG_SERIAL
  WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD] UPDATE: "));
  #endif
  // Serial.print(wirenum);
  // Serial.print(":");
  // Serial.print(modulenum);
  // Serial.print(F(":.:. | UPDATE: "));

  // if(modules[modulenum] == nullptr) {
  //   Serial.println(F("FAIL ENOENT"));
  //   return I2CIP::I2CIP_ERR_SOFT; // ENOENT
  // }

  const I2CIP::EEPROM& eeprom = *(this->eeprom);

  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print(F("EEPROM "));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(eeprom.getFQA()), HEX);
    WIIPOD_DEBUG_SERIAL.print(":");
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(eeprom.getFQA()), HEX);
    WIIPOD_DEBUG_SERIAL.print(":");
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(eeprom.getFQA()), HEX);
    WIIPOD_DEBUG_SERIAL.print(":");
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(eeprom.getFQA()), HEX);
    WIIPOD_DEBUG_SERIAL.print(" ");
  #endif

  unsigned long now = millis();
  // i2cip_errorlevel_t errlev = modules[modulenum]->operator()(eeprom.getFQA(), true, true); // Test failsafe get
  i2cip_errorlevel_t errlev = this->operator()(eeprom.getFQA(), true, true); // Test failsafe get
  unsigned long delta = millis() - now;

  #ifdef WIIPOD_DEBUG_SERIAL
  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
        WIIPOD_DEBUG_SERIAL.print(F("FAIL EIO "));
      break;
    case I2CIP::I2CIP_ERR_SOFT:
      WIIPOD_DEBUG_SERIAL.print(F("FAIL EINVAL "));
      break;
    default:
      WIIPOD_DEBUG_SERIAL.print(F("PASS "));
      break;
  }
  WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
  WIIPOD_DEBUG_SERIAL.print(F("s"));
  
  if(errlev != I2CIP::I2CIP_ERR_NONE) {
    WIIPOD_DEBUG_SERIAL.println();
    return errlev;
  }

  // Bonus Points - Print EEPROM contents
  const char* cache = eeprom.getCache();
  if(cache == nullptr || cache[0] == '\0') {
    WIIPOD_DEBUG_SERIAL.println(F(" EMPTY"));
    return errlev;
  }

  WIIPOD_DEBUG_SERIAL.print(F(" \""));
  for(size_t i = 0; i < I2CIP_EEPROM_SIZE; i++) {
    if(cache[i] == '\0') break;
    WIIPOD_DEBUG_SERIAL.print(cache[i]);
  }
  WIIPOD_DEBUG_SERIAL.println(F("\""));

  #endif

  return errlev;
}

i2cip_errorlevel_t WiiPod::updateNunchuck(uint8_t bus, bool update) {
  i2cip_errorlevel_t errlev;
  i2cip_fqa_t nunchuck_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, NUNCHUCK_ADDRESS);
  if(nunchuck == nullptr) {
    // nunchuck = (Nunchuck*)(modules[MODULE]->operator[]("NUNCHUCK")->operator()(nunchuck_fqa));
    // I2CIP::DeviceGroup* nunchuck_dg = modules[MODULE]->deviceGroupFactory("NUNCHUCK");
    // if(nunchuck_dg == nullptr) { delete nunchuck_dg; nunchuck_dg = nullptr; Serial.println("DG"); crashout(); }
    // nunchuck = (Nunchuck*)(nunchuck_dg->operator()(nunchuck_fqa));

    // if(nunchuck == nullptr) { delete nunchuck; nunchuck = nullptr; Serial.println("No"); crashout(); }
    // if(String(nunchuck->getID()) != nunchuck_dg->key) { delete nunchuck; nunchuck = nullptr; Serial.println("Bad"); crashout(); }

    // if(!modules[MODULE]->add(*Nunchuck::nunchuckFactory(nunchuck_fqa), true)) { Serial.println(F("ADD")); crashout(); }
    // nunchuck = (Nunchuck*)modules[MODULE]->operator[](nunchuck_fqa);
    // DeviceGroup* nunchuck_dg = modules[MODULE]->operator[]("NUNCHUCK");
    DeviceGroup* nunchuck_dg = this->operator[]("NUNCHUCK");
    // DeviceGroup* nunchuck_dg = wiipod["NUNCHUCK"];
    if(nunchuck_dg == nullptr) {
      // delete nunchuck_dg; nunchuck_dg = nullptr; Serial.println("DG"); crashout(); }
      Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }

    nunchuck = (Nunchuck*)(nunchuck_dg->operator()(nunchuck_fqa));
    if(nunchuck == nullptr || nunchuck->getFQA() != nunchuck_fqa) {
      // Serial.println(F("FQA")); crashout(); }
      Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
    // if(!modules[MODULE]->add(*nunchuck, true)) { Serial.println(F("ADD")); crashout(); }
    if(!this->add(*nunchuck, true)) {
      // Serial.println(F("ADD")); crashout(); }
      Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
    // if(!wiipod.add(*nunchuck, true)) { Serial.println(F("ADD")); crashout(); }

  }

  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print("[");
    WIIPOD_DEBUG_SERIAL.print(Nunchuck::getStaticIDBuffer());
    WIIPOD_DEBUG_SERIAL.print(F("] "));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(nunchuck_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(F(":"));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(nunchuck_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(F(":"));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(nunchuck_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(F(":"));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(nunchuck_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(F(" | "));
  #endif

  // errlev = nunchuck->get(data, nullptr);
  // errlev = modules[MODULE]->operator()(nunchuck_fqa, update);
  // errlev = wiipod(nunchuck_fqa, update);
  errlev = this->operator()(nunchuck_fqa, update);
  
  #ifdef WIIPOD_DEBUG_SERIAL
  if(errlev != I2CIP::I2CIP_ERR_NONE) {
    WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
    WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
  } else {
    if(!update) { WIIPOD_DEBUG_SERIAL.println(F("PASS")); return errlev; }

    WIIPOD_DEBUG_SERIAL.print(F("JOY: ("));
    WIIPOD_DEBUG_SERIAL.print((double)nunchuck->getCache().joy_x / 255.0, 2);
    WIIPOD_DEBUG_SERIAL.print(F(", "));
    WIIPOD_DEBUG_SERIAL.print((double)nunchuck->getCache().joy_y / 255.0, 2);
    // WIIPOD_DEBUG_SERIAL.print(F(") | ACCEL: ("));
    // WIIPOD_DEBUG_SERIAL.print(nunchuck_data.accel_x);
    // WIIPOD_DEBUG_SERIAL.print(F(", "));
    // WIIPOD_DEBUG_SERIAL.print(nunchuck_data.accel_y);
    // WIIPOD_DEBUG_SERIAL.print(F(", "));
    // WIIPOD_DEBUG_SERIAL.print(nunchuck_data.accel_z);
    WIIPOD_DEBUG_SERIAL.print(F(") | C: "));
    WIIPOD_DEBUG_SERIAL.print(nunchuck->getCache().c ? F("Y") : F("N"));
    WIIPOD_DEBUG_SERIAL.print(F(" | Z: "));
    WIIPOD_DEBUG_SERIAL.println(nunchuck->getCache().z ? F("Y") : F("N"));
  }
  #endif

  return errlev;
}

i2cip_errorlevel_t WiiPod::updateSHT31(uint8_t bus, bool update) {
  i2cip_errorlevel_t errlev;
  i2cip_fqa_t sht31_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, SHT31_ADDR);
  if(sht31 == nullptr) {
    // if(!modules[MODULE]->add(*SHT31::sht31Factory(sht31_fqa), true)) { Serial.println(F("ADD")); crashout(); }
    // sht31 = (SHT31*)modules[MODULE]->operator[](sht31_fqa);
    // if(sht31 == nullptr || sht31->getFQA() != sht31_fqa) { Serial.println(F("FQA")); crashout(); }
    // if(modules[MODULE]->operator[]("SHT31") == nullptr) { Serial.println(F("DG")); crashout(); }

    // DeviceGroup* sht31_dg = modules[MODULE]->operator[]("SHT31");
    DeviceGroup* sht31_dg = this->operator[]("SHT31");
    // DeviceGroup* sht31_dg = wiipod["SHT31"];
    if(sht31_dg == nullptr) {
      // delete sht31_dg; sht31_dg = nullptr; Serial.println("DG"); crashout(); }
      Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }
    sht31 = (SHT31*)(sht31_dg->operator()(sht31_fqa));
    if(sht31 == nullptr || sht31->getFQA() != sht31_fqa) {
      // Serial.println(F("FQA")); crashout(); }
      Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
    // if(!modules[MODULE]->add(*sht31, true)) { Serial.println(F("ADD")); crashout(); }
    if(!this->add(*sht31, true)) {
      // Serial.println(F("ADD")); crashout(); }
      Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
    // if(!wiipod.add(*sht31, true)) { Serial.println(F("ADD")); crashout(); }

    // sht31 = (SHT31*)(modules[MODULE]->operator[]("SHT31")->operator()(sht31_fqa));

  }

  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print("[");
    WIIPOD_DEBUG_SERIAL.print(SHT31::getStaticIDBuffer());
    WIIPOD_DEBUG_SERIAL.print(F("] "));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(sht31_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(F(":"));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(sht31_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(F(":"));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(sht31_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(F(":"));
    WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(sht31_fqa), HEX);
    WIIPOD_DEBUG_SERIAL.print(F(" | "));
  #endif

  // errlev = modules[MODULE]->operator()(sht31_fqa, update);
  errlev = this->operator()(sht31_fqa, update);
  // errlev = wiipod(sht31_fqa, update);
  #ifdef WIIPOD_DEBUG_SERIAL
  if(errlev == I2CIP::I2CIP_ERR_NONE) {
    if(!update) { WIIPOD_DEBUG_SERIAL.println(F("PASS")); return errlev; }

    WIIPOD_DEBUG_SERIAL.print(F("TEMPERATURE: "));
    WIIPOD_DEBUG_SERIAL.print(sht31->getCache().temperature, 2);
    WIIPOD_DEBUG_SERIAL.print(F("C, HUMIDITY: "));
    WIIPOD_DEBUG_SERIAL.print(sht31->getCache().humidity, 2);
    WIIPOD_DEBUG_SERIAL.println(F("%"));
  } else {
    WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
    WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
  }
  #endif

  return errlev;
}