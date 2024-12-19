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
#include "../src/debug.h"

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
  
  return DeviceGroup::create<SSD1306>(id);
}

bool WiiPod::initialize(void) {
  if(initialized) return true;

  #ifdef WIIPOD_DEBUG_SERIAL
  WIIPOD_DEBUG_SERIAL.println(F("[WIIPOD | INIT] START"));
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
      #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.println(F("[WIIPOD | INIT] FAIL EIO"));
      #endif
      return false;
    case I2CIP::I2CIP_ERR_SOFT:
      #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.println(F("[WIIPOD | INIT] FAIL EINVAL"));
      #endif
      return false;
    default:
      break;
  }

  bool r = I2CIP::MUX::pingMUX((this->eeprom->getFQA()));
  unsigned long delta = millis() - now;
  if(!r) {
    #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.println(F("[WIIPOD | INIT] FAIL EIO"));
    #endif
    return false;
  }
  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | INIT] PASS "));
    WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
    WIIPOD_DEBUG_SERIAL.println(F("s"));
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
    WIIPOD_DEBUG_SERIAL.println(F("[WIIPOD | CHECK] START"));
  #endif

  unsigned long now = millis();
  // i2cip_errorlevel_t errlev = modules[MODULE]->operator()();
  i2cip_errorlevel_t errlev = this->operator()();
  unsigned long delta = millis() - now;

  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
      #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | CHECK] FAIL EIO "));
      #endif
      initialized = false;
      break;
    case I2CIP::I2CIP_ERR_SOFT:
      #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | CHECK] FAIL EINVAL "));
      #endif
      break;
    default:
      #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | CHECK] PASS "));
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
  #ifdef WIIPOD_DEBUG_SERIAL
  i2cip_errorlevel_t errlev = this->operator()<EEPROM>(eeprom_fqa, true, _i2cip_args_io_default, WIIPOD_DEBUG_SERIAL); // Test failsafe get
  #else
  i2cip_errorlevel_t errlev = this->operator()<EEPROM>(eeprom_fqa, true); // Test failsafe get
  #endif
  unsigned long delta = millis() - now;

  #ifdef WIIPOD_DEBUG_SERIAL
  switch(errlev) {
    case I2CIP::I2CIP_ERR_HARD:
        WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | UPDATE] EEPROM FAIL EIO "));
      break;
    case I2CIP::I2CIP_ERR_SOFT:
      WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | UPDATE] EEPROM FAIL EINVAL "));
      break;
    default:
      WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | UPDATE] EEPROM PASS "));
      break;
  }
  WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
  WIIPOD_DEBUG_SERIAL.print(F("s"));
  
  if(errlev != I2CIP::I2CIP_ERR_NONE) {
    WIIPOD_DEBUG_SERIAL.println();
    return errlev;
  }

  // Bonus Points - Print EEPROM contents
  const char* cache = this->eeprom->getCache();
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

  #ifdef WIIPOD_BUS_NUNCHUCK
  #ifdef WIIPOD_DEBUG_SERIAL
  errlev = this->operator()<Nunchuck>(createFQA(WIIPOD_BUS_NUNCHUCK, I2CIP_NUNCHUCK_ADDRESS), true, _i2cip_args_io_default, WIIPOD_DEBUG_SERIAL);
  #else
  errlev = this->operator()<Nunchuck>(createFQA(WIIPOD_BUS_NUNCHUCK, I2CIP_NUNCHUCK_ADDRESS), true);
  #endif
  // errlev = this->updateNunchuck(WIIPOD_BUS_NUNCHUCK, true);
  // const wiipod_nunchuck_t* data = this->getNunchuckCache();
  // if(errlev == I2CIP_ERR_NONE && data != nullptr) {
  //   // Data & Fonts
  //   // bool use_c = data->c;
  //   // bool use_z = data->z;
  //   // bool border = data->z;
  //   // bool use_bold = use_c && use_z;
  //   // char c = use_bold ? ' ' : (use_z ? 'Z' : (use_c ? '+' : '~'));

  //   #ifdef WIIPOD_BUS_SCREEN
  //   if(this->screen == nullptr) {this->screen = new SSDD1306(I2CIP::createFQA(this->wirenum, this->modulenum, WIIPOD_BUS_SSDD1306, I2CIP_SSDD1306_ADDRESS));}
  //   #endif

  //   now = millis();
  //   errlev = this->renderNunchuck();
  //   if(errlev == I2CIP_ERR_HARD) { delete this->screen; this->screen = nullptr; }
  //   fps += 1000.0 / (millis() - now); fps /= 2.0; // Rolling average

  //   // RENDER UPDATE
  //   #ifdef WIIPOD_DEBUG_SERIAL
  //     this->printNunchuck(WIIPOD_DEBUG_SERIAL); // Render Screen
  //     WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | RENDER "));
  //     WIIPOD_DEBUG_SERIAL.print(cycle.get().toString());
  //     WIIPOD_DEBUG_SERIAL.print(F(" | "));
  //     WIIPOD_DEBUG_SERIAL.print(fps, 0);
  //     WIIPOD_DEBUG_SERIAL.print(F(" FPS | 0x"));
  //     WIIPOD_DEBUG_SERIAL.print(errlev, HEX);
  //     WIIPOD_DEBUG_SERIAL.println(F("]"));
  //   #endif
  // }
  #endif

  // #ifdef WIIPOD_BUS_SHT31
  // // Get SHT31 Data
  // errlev = this->updateSHT31(1, true);
  // if(errlev == I2CIP_ERR_NONE) {
  //   const state_sht31_t* sht31 = this->getSHT31Cache();
  //   if(sht31 != nullptr) {
  //     WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | SHT31] TEMPERATURE "));
  //     WIIPOD_DEBUG_SERIAL.print(sht31->temperature, 2);
  //     WIIPOD_DEBUG_SERIAL.print(F("C, HUMIDITY "));
  //     WIIPOD_DEBUG_SERIAL.print(sht31->humidity, 2);
  //     WIIPOD_DEBUG_SERIAL.println(F("% RH"));
  //   }
  // }
  // #endif

  #ifdef WIIPOD_BUS_SHT45
  // errlev = this->updateSHT45(WIIPOD_BUS_SHT45, true);
  i2cip_fqa_t fqa_sht45 = createFQA(WIIPOD_BUS_SHT45, I2CIP_SHT45_ADDRESS);
  #ifdef WIIPOD_DEBUG_SERIAL
  errlev = this->operator()<SHT45>(fqa_sht45, true, _i2cip_args_io_default, WIIPOD_DEBUG_SERIAL);
  #else
  errlev = this->operator()<SHT45>(fqa_sht45, true);
  #endif
  // if(errlev == I2CIP_ERR_NONE) {
  //   const state_sht45_t* sht45 = this->getSHT45Cache();
  //   if(sht45 != nullptr) {
  //     #ifdef WIIPOD_DEBUG_SERIAL
  //       WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | SHT45] TEMPERATURE "));
  //       WIIPOD_DEBUG_SERIAL.print(sht45->temperature, 1);
  //       WIIPOD_DEBUG_SERIAL.print(F("C, HUMIDITY "));
  //       WIIPOD_DEBUG_SERIAL.print(sht45->humidity, 1);
  //       WIIPOD_DEBUG_SERIAL.println(F("%"));
  //     #endif
  //   }
  // }
  #endif

  #ifdef WIIPOD_BUS_K30
  i2cip_fqa_t fqa_k30 = createFQA(WIIPOD_BUS_K30, I2CIP_K30_ADDRESS);
  #ifdef WIIPOD_DEBUG_SERIAL
  errlev = this->operator()<K30>(fqa_k30, true, _i2cip_args_io_default, WIIPOD_DEBUG_SERIAL);
  #else
  errlev = this->operator()<K30>(fqa_k30, true);
  #endif
  // if(errlev == I2CIP_ERR_NONE) {
  //   const uint16_t* k30 = this->getK30Cache();
  //   if(k30 != nullptr) {
  //     WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | K30] CO2 "));
  //     WIIPOD_DEBUG_SERIAL.print(*k30);
  //     WIIPOD_DEBUG_SERIAL.print(F(" PPM"));
  //   }
  // }
  #endif

  #ifdef WIIPOD_BUS_MCP
  i2cip_fqa_t fqa_mcp = createFQA(WIIPOD_BUS_MCP, I2CIP_MCP23017_ADDRESS);
  // errlev = this->updateMCP23017(WIIPOD_BUS_MCP, true);
  i2cip_mcp23017_bitmask_t a = 0x1; i2cip_mcp23017_bitmask_t b = 0x1; i2cip_mcp23017_t s = flash ? 0x1 : 0x0;
  i2cip_args_io_t args_mcp = {.a = &a, .s = &s, .b = &b};
  flash = !flash;
  #ifdef WIIPOD_DEBUG_SERIAL
  errlev = this->operator()<MCP23017>(fqa_mcp, true, args_mcp, WIIPOD_DEBUG_SERIAL);
  #else
  errlev = this->operator()<MCP23017>(fqa_mcp, true, args_mcp);
  #endif
  #endif

  #ifdef WIIPOD_BUS_ROTARY
  // errlev = this->updateRotaryEncoder(WIIPOD_BUS_ROTARY, true);
  #ifdef WIIPOD_DEBUG_SERIAL
  errlev = this->operator()<Seesaw_RotaryEncoder>(createFQA(WIIPOD_BUS_ROTARY, I2CIP_SEESAW_ADDRESS), true, _i2cip_args_io_default, WIIPOD_DEBUG_SERIAL);
  #else
  errlev = this->operator()<Seesaw_RotaryEncoder>(createFQA(WIIPOD_BUS_ROTARY, I2CIP_SEESAW_ADDRESS), true);
  #endif
  #endif

  // if(errlev == I2CIP_ERR_NONE) {
  //   const i2cip_rotaryencoder_t* seesaw = this->getSeesawCache();
  //   #ifdef WIIPOD_BUS_SCREEN
  //   if(seesaw != nullptr) {
  //     if(this->screen == nullptr) {this->screen = new SSD1306(I2CIP::createFQA(this->wirenum, this->modulenum, WIIPOD_BUS_SCREEN, I2CIP_SSD1306_ADDRESS));}
  //     if(this->screen != nullptr) {
  //       now = millis();
  //       errlev = this->renderRotary();
  //       if(errlev != I2CIP_ERR_NONE) { 
  //         #ifdef WIIPOD_DEBUG_SERIAL
  //           WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | RENDER] ROTARY -> SCREEN FAIL "));
  //           WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
  //         #endif
  //       }
  //     }
  //   }
  //   #endif
  // }

  #ifdef WIIPOD_BUS_PCA
  // errlev = this->updatePCA9685(WIIPOD_BUS_PCA, true);
  #ifdef WIIPOD_DEBUG_SERIAL
  errlev = this->operator()<PCA9685>(createFQA(WIIPOD_BUS_PCA, I2CIP_PCA9685_ADDRESS), true, _i2cip_args_io_default, WIIPOD_DEBUG_SERIAL);
  #else
  errlev = this->operator()<PCA9685>(createFQA(WIIPOD_BUS_PCA, I2CIP_PCA9685_ADDRESS), true);
  #endif
  #endif

  #ifdef WIIPOD_BUS_ADS
  // errlev = this->updateADS1115(WIIPOD_BUS_ADS, true);
  #ifdef WIIPOD_DEBUG_SERIAL
  errlev = this->operator()<ADS1115>(createFQA(WIIPOD_BUS_ADS, I2CIP_ADS1115_ADDRESS), true, _i2cip_args_io_default, WIIPOD_DEBUG_SERIAL);
  #else
  errlev = this->operator()<ADS1115>(createFQA(WIIPOD_BUS_ADS, I2CIP_ADS1115_ADDRESS), true);
  #endif
  #endif

// Uncomment to perform scan
  // #ifdef WIIPOD_DEBUG_SERIAL
  //   this->scanToPrint(WIIPOD_DEBUG_SERIAL, this->wirenum, this->modulenum);
  // #endif

  return errlev;
}

// i2cip_errorlevel_t WiiPod::updateNunchuck(uint8_t bus, bool update) {
//   i2cip_errorlevel_t errlev;
//   i2cip_fqa_t nunchuck_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, NUNCHUCK_ADDRESS);
//   if(nunchuck == nullptr) {
//     // nunchuck = (Nunchuck*)(modules[MODULE]->operator[]("NUNCHUCK")->operator()(nunchuck_fqa));
//     // I2CIP::DeviceGroup* nunchuck_dg = modules[MODULE]->deviceGroupFactory("NUNCHUCK");
//     // if(nunchuck_dg == nullptr) { delete nunchuck_dg; nunchuck_dg = nullptr; Serial.println("DG"); crashout(); }
//     // nunchuck = (Nunchuck*)(nunchuck_dg->operator()(nunchuck_fqa));

//     // if(nunchuck == nullptr) { delete nunchuck; nunchuck = nullptr; Serial.println("No"); crashout(); }
//     // if(String(nunchuck->getID()) != nunchuck_dg->key) { delete nunchuck; nunchuck = nullptr; Serial.println("Bad"); crashout(); }

//     // if(!modules[MODULE]->add(*Nunchuck::nunchuckFactory(nunchuck_fqa), true)) { Serial.println(F("ADD")); crashout(); }
//     // nunchuck = (Nunchuck*)modules[MODULE]->operator[](nunchuck_fqa);
//     // DeviceGroup* nunchuck_dg = modules[MODULE]->operator[]("NUNCHUCK");
//     DeviceGroup* nunchuck_dg = this->operator[](Nunchuck::getID(i2cip_nunchuck_id_progmem));
//     // DeviceGroup* nunchuck_dg = wiipod["NUNCHUCK"];
//     if(nunchuck_dg == nullptr) {
//       // delete nunchuck_dg; nunchuck_dg = nullptr; Serial.println("DG"); crashout(); }
//       Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }

//     nunchuck = new Nunchuck(nunchuck_fqa);
//     if(nunchuck == nullptr || nunchuck->getFQA() != nunchuck_fqa) {
//       // Serial.println(F("FQA")); crashout(); }
//       Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!modules[MODULE]->add(*nunchuck, true)) { Serial.println(F("ADD")); crashout(); }
//     if(!this->add(nunchuck, true)) {
//       // Serial.println(F("ADD")); crashout(); }
//       Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!wiipod.add(*nunchuck, true)) { Serial.println(F("ADD")); crashout(); }

//   }

//   #ifdef WIIPOD_DEBUG_SERIAL
//     WIIPOD_DEBUG_SERIAL.print("[");
//     WIIPOD_DEBUG_SERIAL.print(Nunchuck::getID());
//     WIIPOD_DEBUG_SERIAL.print(F("] "));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(nunchuck_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(nunchuck_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(nunchuck_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(nunchuck_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(" | "));
//   #endif

//   // errlev = nunchuck->get(data, nullptr);
//   // errlev = modules[MODULE]->operator()(nunchuck_fqa, update);
//   // errlev = wiipod(nunchuck_fqa, update);
//   unsigned long now = millis();
//   errlev = this->operator()(nunchuck_fqa, update);
//   unsigned long delta = millis() - now;
  
//   #ifdef WIIPOD_DEBUG_SERIAL
//   if(errlev != I2CIP::I2CIP_ERR_NONE) {
//     WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
//     WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
//   } else {
//     WIIPOD_DEBUG_SERIAL.print(F("PASS "));
//     WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
//     WIIPOD_DEBUG_SERIAL.print(F("s"));
//     if(!update) { WIIPOD_DEBUG_SERIAL.print('\n'); return errlev; }

//     WIIPOD_DEBUG_SERIAL.print(F(" | JOY: ("));
//     WIIPOD_DEBUG_SERIAL.print((double)nunchuck->getCache().joy_x / 255.0, 2);
//     WIIPOD_DEBUG_SERIAL.print(F(", "));
//     WIIPOD_DEBUG_SERIAL.print((double)nunchuck->getCache().joy_y / 255.0, 2);
//     // WIIPOD_DEBUG_SERIAL.print(F(") | ACCEL: ("));
//     // WIIPOD_DEBUG_SERIAL.print(nunchuck_data.accel_x);
//     // WIIPOD_DEBUG_SERIAL.print(F(", "));
//     // WIIPOD_DEBUG_SERIAL.print(nunchuck_data.accel_y);
//     // WIIPOD_DEBUG_SERIAL.print(F(", "));
//     // WIIPOD_DEBUG_SERIAL.print(nunchuck_data.accel_z);
//     WIIPOD_DEBUG_SERIAL.print(F(") | C: "));
//     WIIPOD_DEBUG_SERIAL.print(nunchuck->getCache().c ? F("Y") : F("N"));
//     WIIPOD_DEBUG_SERIAL.print(F(" | Z: "));
//     WIIPOD_DEBUG_SERIAL.println(nunchuck->getCache().z ? F("Y") : F("N"));
//   }
//   #endif

//   return errlev;
// }

// i2cip_errorlevel_t WiiPod::updateSHT45(uint8_t bus, bool update) {
//   i2cip_errorlevel_t errlev;
//   i2cip_fqa_t sht45_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, SHT45_ADDRESS);
//   if(sht45 == nullptr) {
//     DeviceGroup* sht45_dg = this->operator[](SHT45::getID(i2cip_sht45_id_progmem));
//     if(sht45_dg == nullptr) {
//       Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }

//     sht45 = new SHT45(sht45_fqa);
//     if(sht45 == nullptr || sht45->getFQA() != sht45_fqa) {
//       Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
//     if(!this->add(sht45, true)) {
//       Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }

//   }

//   #ifdef WIIPOD_DEBUG_SERIAL
//     WIIPOD_DEBUG_SERIAL.print("[");
//     WIIPOD_DEBUG_SERIAL.print(SHT45::getID());
//     WIIPOD_DEBUG_SERIAL.print(F("] "));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(sht45_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(sht45_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(sht45_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(sht45_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.println(F(" | UPDATE] START"));
//   #endif

//   // errlev = sht45->get(data, nullptr);
//   // errlev = modules[MODULE]->operator()(sht45_fqa, update);
//   // errlev = wiipod(sht45_fqa, update);
//   unsigned long now = millis();
//   errlev = this->operator()(sht45_fqa, update);
//   unsigned long delta = millis() - now;
  
//   #ifdef WIIPOD_DEBUG_SERIAL
//   WIIPOD_DEBUG_SERIAL.print("[");
//   WIIPOD_DEBUG_SERIAL.print(SHT45::getID());
//   WIIPOD_DEBUG_SERIAL.print(F("] "));
//   WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(sht45_fqa), HEX);
//   WIIPOD_DEBUG_SERIAL.print(F(":"));
//   WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(sht45_fqa), HEX);
//   WIIPOD_DEBUG_SERIAL.print(F(":"));
//   WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(sht45_fqa), HEX);
//   WIIPOD_DEBUG_SERIAL.print(F(":"));
//   WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(sht45_fqa), HEX);
//   WIIPOD_DEBUG_SERIAL.print(F(" | UPDATE] "));
//   if(errlev != I2CIP::I2CIP_ERR_NONE) {
//     WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
//     WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
//   } else {
//     WIIPOD_DEBUG_SERIAL.print(F("PASS"));
//     if(!update) { WIIPOD_DEBUG_SERIAL.print('\n'); return errlev; }
//     WIIPOD_DEBUG_SERIAL.print(' ');
//     WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
//     WIIPOD_DEBUG_SERIAL.print(F("s - [TEMPERATURE: "));
//     WIIPOD_DEBUG_SERIAL.print(sht45->getCache().temperature, 1);
//     WIIPOD_DEBUG_SERIAL.print(F("C, "));
//     WIIPOD_DEBUG_SERIAL.print(sht45->getCache().humidity, 1);
//     WIIPOD_DEBUG_SERIAL.println(F("%]"));
//   }
//   #endif

//   return errlev;
// }

// // i2cip_errorlevel_t WiiPod::updateSHT31(uint8_t bus, bool update) {
// //   SHT31::loadID();
// //   i2cip_errorlevel_t errlev;
// //   i2cip_fqa_t sht31_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, SHT31_ADDR);
// //   if(sht31 == nullptr) {
// //     // if(!modules[MODULE]->add(*SHT31::sht31Factory(sht31_fqa), true)) { Serial.println(F("ADD")); crashout(); }
// //     // sht31 = (SHT31*)modules[MODULE]->operator[](sht31_fqa);
// //     // if(sht31 == nullptr || sht31->getFQA() != sht31_fqa) { Serial.println(F("FQA")); crashout(); }
// //     // if(modules[MODULE]->operator[]("SHT31") == nullptr) { Serial.println(F("DG")); crashout(); }

// //     // DeviceGroup* sht31_dg = modules[MODULE]->operator[]("SHT31");
// //     DeviceGroup* sht31_dg = this->operator[](SHT31::getID());
// //     // DeviceGroup* sht31_dg = wiipod["SHT31"];
// //     if(sht31_dg == nullptr) {
// //       // delete sht31_dg; sht31_dg = nullptr; Serial.println("DG"); crashout(); }
// //       Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }
// //     sht31 = (SHT31*)(sht31_dg->operator()(sht31_fqa));
// //     if(sht31 == nullptr || sht31->getFQA() != sht31_fqa) {
// //       // Serial.println(F("FQA")); crashout(); }
// //       Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
// //     // if(!modules[MODULE]->add(*sht31, true)) { Serial.println(F("ADD")); crashout(); }
// //     if(!this->add(sht31, true)) {
// //       // Serial.println(F("ADD")); crashout(); }
// //       Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
// //     // if(!wiipod.add(*sht31, true)) { Serial.println(F("ADD")); crashout(); }

// //     // sht31 = (SHT31*)(modules[MODULE]->operator[]("SHT31")->operator()(sht31_fqa));

// //   }

// //   #ifdef WIIPOD_DEBUG_SERIAL
// //     WIIPOD_DEBUG_SERIAL.print("[");
// //     WIIPOD_DEBUG_SERIAL.print(SHT31::getID());
// //     WIIPOD_DEBUG_SERIAL.print(F("] "));
// //     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(sht31_fqa), HEX);
// //     WIIPOD_DEBUG_SERIAL.print(F(":"));
// //     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(sht31_fqa), HEX);
// //     WIIPOD_DEBUG_SERIAL.print(F(":"));
// //     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(sht31_fqa), HEX);
// //     WIIPOD_DEBUG_SERIAL.print(F(":"));
// //     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(sht31_fqa), HEX);
// //     WIIPOD_DEBUG_SERIAL.print(F(" | "));
// //   #endif

// //   // errlev = modules[MODULE]->operator()(sht31_fqa, update);
// //   errlev = this->operator()(sht31, update);
// //   // errlev = wiipod(sht31_fqa, update);
// //   #ifdef WIIPOD_DEBUG_SERIAL
// //   if(errlev == I2CIP::I2CIP_ERR_NONE) {
// //     if(!update) { WIIPOD_DEBUG_SERIAL.println(F("PASS")); return errlev; }

// //     WIIPOD_DEBUG_SERIAL.print(F("TEMPERATURE: "));
// //     WIIPOD_DEBUG_SERIAL.print(sht31->getCache().temperature, 2);
// //     WIIPOD_DEBUG_SERIAL.print(F("C, HUMIDITY: "));
// //     WIIPOD_DEBUG_SERIAL.print(sht31->getCache().humidity, 2);
// //     WIIPOD_DEBUG_SERIAL.println(F("%"));
// //   } else {
// //     WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
// //     WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
// //   }
// //   #endif

// //   return errlev;
// // }

// i2cip_errorlevel_t WiiPod::updateK30(uint8_t bus, bool update) {
//   i2cip_errorlevel_t errlev;
//   i2cip_fqa_t k30_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, K30_ADDRESS);
//   if(k30 == nullptr) {
//     // if(!modules[MODULE]->add(*K30::k30Factory(k30_fqa), true)) { Serial.println(F("ADD")); crashout(); }
//     // k30 = (K30*)modules[MODULE]->operator[](k30_fqa);
//     // if(k30 == nullptr || k30->getFQA() != k30_fqa) { Serial.println(F("FQA")); crashout(); }
//     // if(modules[MODULE]->operator[]("K30") == nullptr) { Serial.println(F("DG")); crashout(); }

//     // DeviceGroup* k30_dg = modules[MODULE]->operator[]("K30");
//     DeviceGroup* k30_dg = this->operator[](K30::getID(i2cip_k30_id_progmem));
//     // DeviceGroup* k30_dg = wiipod["K30"];
//     if(k30_dg == nullptr) {
//       // delete k30_dg; k30_dg = nullptr; Serial.println("DG"); crashout(); }
//       Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }
//     k30 = (K30*)(k30_dg->operator()(k30_fqa));
//     if(k30 == nullptr || k30->getFQA() != k30_fqa) {
//       // Serial.println(F("FQA")); crashout(); }
//       Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!modules[MODULE]->add(*k30, true)) { Serial.println(F("ADD")); crashout(); }
//     if(!this->add(k30, true)) {
//       // Serial.println(F("ADD")); crashout(); }
//       Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!wiipod.add(*k30, true)) { Serial.println(F("ADD")); crashout(); }

//     // k30 = (K30*)(modules[MODULE]->operator[]("K30")->operator()(k30_fqa));

//   }

//   #ifdef WIIPOD_DEBUG_SERIAL
//     WIIPOD_DEBUG_SERIAL.print("[");
//     WIIPOD_DEBUG_SERIAL.print(K30::getID());
//     WIIPOD_DEBUG_SERIAL.print(F("] "));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(k30_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(k30_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(k30_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(k30_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(" | "));
//   #endif

//   // errlev = modules[MODULE]->operator()(k30_fqa, update);
//   unsigned long now = millis();
//   errlev = this->operator()(k30, update);
//   unsigned long delta = millis() - now;
//   // errlev = wiipod(k30_fqa, update);
//   #ifdef WIIPOD_DEBUG_SERIAL
//   if(errlev == I2CIP::I2CIP_ERR_NONE) {
//     WIIPOD_DEBUG_SERIAL.print(F("PASS "));
//     WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
//     WIIPOD_DEBUG_SERIAL.print(F("s"));
//     if(!update) { WIIPOD_DEBUG_SERIAL.print('\n'); return errlev; }
//     // New cache!
//     WIIPOD_DEBUG_SERIAL.print(F(" CO2: "));
//     WIIPOD_DEBUG_SERIAL.print(k30->getCache());
//     WIIPOD_DEBUG_SERIAL.println(F("PPM"));
//   } else {
//     WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
//     WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
//   }
//   #endif

//   return errlev;
// }

// i2cip_errorlevel_t WiiPod::updateMCP23017(uint8_t bus, bool update) {
//   i2cip_errorlevel_t errlev;
//   i2cip_fqa_t mcp_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, I2CIP_MCP23017_ADDRESS);
//   if(mcp == nullptr) {
//     // if(!modules[MODULE]->add(*K30::k30Factory(k30_fqa), true)) { Serial.println(F("ADD")); crashout(); }
//     // k30 = (K30*)modules[MODULE]->operator[](k30_fqa);
//     // if(k30 == nullptr || k30->getFQA() != k30_fqa) { Serial.println(F("FQA")); crashout(); }
//     // if(modules[MODULE]->operator[]("K30") == nullptr) { Serial.println(F("DG")); crashout(); }

//     // DeviceGroup* k30_dg = modules[MODULE]->operator[]("K30");
//     DeviceGroup* mcp_dg = this->operator[](MCP23017::getID(i2cip_mcp23017_id_progmem));
//     // DeviceGroup* k30_dg = wiipod["K30"];
//     if(mcp_dg == nullptr) {
//       // delete k30_dg; k30_dg = nullptr; Serial.println("DG"); crashout(); }
//       Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }
//     mcp = (MCP23017*)(mcp_dg->operator()(mcp_fqa));
//     if(mcp == nullptr || mcp->getFQA() != mcp_fqa) {
//       // Serial.println(F("FQA")); crashout(); }
//       Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!modules[MODULE]->add(*k30, true)) { Serial.println(F("ADD")); crashout(); }
//     if(!this->add(mcp, true)) {
//       // Serial.println(F("ADD")); crashout(); }
//       Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!wiipod.add(*k30, true)) { Serial.println(F("ADD")); crashout(); }

//     // k30 = (K30*)(modules[MODULE]->operator[]("K30")->operator()(mcp_fqa));

//   }

//   #ifdef WIIPOD_DEBUG_SERIAL
//     WIIPOD_DEBUG_SERIAL.print("[");
//     WIIPOD_DEBUG_SERIAL.print(MCP23017::getID());
//     WIIPOD_DEBUG_SERIAL.print(F("] "));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(mcp_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(mcp_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(mcp_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(mcp_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(" | "));
//   #endif

//   // errlev = modules[MODULE]->operator()(mcp_fqa, update);
//   // i2cip_mcp23017_t, i2cip_mcp23017_bitmask_t, i2cip_mcp23017_t, i2cip_mcp23017_bitmask_t
//   i2cip_mcp23017_bitmask_t a = 0x1;
//   i2cip_mcp23017_bitmask_t b = 0x1;
//   i2cip_mcp23017_t s = 0x1;
//   i2cip_args_io_t args = {.a = &a, .s = &s, .b = &b};
//   unsigned long now = millis();
//   errlev = this->operator()(mcp, update, args);
//   unsigned long delta = millis() - now;
//   // errlev = wiipod(mcp_fqa, update);
//   #ifdef WIIPOD_DEBUG_SERIAL
//   if(errlev == I2CIP::I2CIP_ERR_NONE) {
//     WIIPOD_DEBUG_SERIAL.print(F("PASS "));
//     WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
//     WIIPOD_DEBUG_SERIAL.print(F("s"));

//     if(!update) { WIIPOD_DEBUG_SERIAL.println(); return errlev; }

//     WIIPOD_DEBUG_SERIAL.print(F(" | A: 0b"));
//     WIIPOD_DEBUG_SERIAL.print((uint8_t)(mcp->getCache()), BIN);
//     WIIPOD_DEBUG_SERIAL.print(F(" | B: 0b"));
//     WIIPOD_DEBUG_SERIAL.println((uint8_t)(mcp->getCache() >> 8), BIN);
//   //   WIIPOD_DEBUG_SERIAL.print(F("ppm"));
//   } else {
//     WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
//     WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
//   }
//   #endif

//   return errlev;
// }

// i2cip_errorlevel_t WiiPod::updateRotaryEncoder(uint8_t bus, bool update) {
//   i2cip_errorlevel_t errlev;
//   i2cip_fqa_t seesaw_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, I2CIP_SEESAW_ADDRESS);
//   if(seesaw == nullptr) {
//     // if(!modules[MODULE]->add(*K30::k30Factory(k30_fqa), true)) { Serial.println(F("ADD")); crashout(); }
//     // k30 = (K30*)modules[MODULE]->operator[](k30_fqa);
//     // if(k30 == nullptr || k30->getFQA() != k30_fqa) { Serial.println(F("FQA")); crashout(); }
//     // if(modules[MODULE]->operator[]("K30") == nullptr) { Serial.println(F("DG")); crashout(); }

//     // DeviceGroup* k30_dg = modules[MODULE]->operator[]("K30");
//     DeviceGroup* seesaw_dg = this->operator[](Seesaw_RotaryEncoder::getID(i2cip_seesaw_id_progmem));
//     // DeviceGroup* k30_dg = wiipod["K30"];
//     if(seesaw_dg == nullptr) {
//       // delete k30_dg; k30_dg = nullptr; Serial.println("DG"); crashout(); }
//       Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }
//     seesaw = (Seesaw_RotaryEncoder*)(seesaw_dg->operator()(seesaw_fqa));
//     if(seesaw == nullptr || seesaw->getFQA() != seesaw_fqa) {
//       // Serial.println(F("FQA")); crashout(); }
//       Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!modules[MODULE]->add(*k30, true)) { Serial.println(F("ADD")); crashout(); }
//     if(!this->add(seesaw, true)) {
//       // Serial.println(F("ADD")); crashout(); }
//       Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!wiipod.add(*k30, true)) { Serial.println(F("ADD")); crashout(); }

//     // k30 = (K30*)(modules[MODULE]->operator[]("K30")->operator()(seesaw_fqa));

//   }

//   #ifdef WIIPOD_DEBUG_SERIAL
//     WIIPOD_DEBUG_SERIAL.print("[");
//     WIIPOD_DEBUG_SERIAL.print(Seesaw_RotaryEncoder::getID());
//     WIIPOD_DEBUG_SERIAL.print(F("] "));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(seesaw_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(seesaw_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(seesaw_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(seesaw_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(" | "));
//   #endif

//   // errlev = modules[MODULE]->operator()(seesaw_fqa, update);
//   unsigned long now = millis();
//   errlev = this->operator()(seesaw, update);
//   unsigned long delta = millis() - now;
//   // errlev = wiipod(seesaw_fqa, update);
//   #ifdef WIIPOD_DEBUG_SERIAL
//   if(errlev == I2CIP::I2CIP_ERR_NONE) {
//     WIIPOD_DEBUG_SERIAL.print(F("PASS "));
//     WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
//     WIIPOD_DEBUG_SERIAL.print(F("s"));

//     if(!update) { WIIPOD_DEBUG_SERIAL.println(); return errlev; }

//     WIIPOD_DEBUG_SERIAL.print(F(" | ROTARY: 0x"));
//     WIIPOD_DEBUG_SERIAL.print((seesaw->getCache()).encoder, HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(" | BUTTON: "));
//     WIIPOD_DEBUG_SERIAL.println((seesaw->getCache()).button == PIN_OFF ? F("OFF") : ((seesaw->getCache()).button == PIN_ON ? F("ON") : F("???")));
//   //   WIIPOD_DEBUG_SERIAL.print(F("ppm"));
//   } else {
//     WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
//     WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
//   }
//   #endif

//   return errlev;
// }

// bool flash = true;

// i2cip_errorlevel_t WiiPod::updatePCA9685(uint8_t bus, bool update) {
//   i2cip_errorlevel_t errlev;
//   i2cip_fqa_t pca_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, I2CIP_PCA9685_ADDRESS);
//   if(pca == nullptr) {
//     // if(!modules[MODULE]->add(*K30::k30Factory(k30_fqa), true)) { Serial.println(F("ADD")); crashout(); }
//     // k30 = (K30*)modules[MODULE]->operator[](k30_fqa);
//     // if(k30 == nullptr || k30->getFQA() != k30_fqa) { Serial.println(F("FQA")); crashout(); }
//     // if(modules[MODULE]->operator[]("K30") == nullptr) { Serial.println(F("DG")); crashout(); }

//     // DeviceGroup* k30_dg = modules[MODULE]->operator[]("K30");
//     DeviceGroup* pca_dg = this->operator[](PCA9685::getID(i2cip_pca9685_id_progmem));
//     // DeviceGroup* k30_dg = wiipod["K30"];
//     if(pca_dg == nullptr) {
//       // delete k30_dg; k30_dg = nullptr; Serial.println("DG"); crashout(); }
//       Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }
//     pca = (PCA9685*)(pca_dg->operator()(pca_fqa));
//     if(pca == nullptr || pca->getFQA() != pca_fqa) {
//       // Serial.println(F("FQA")); crashout(); }
//       Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!modules[MODULE]->add(*k30, true)) { Serial.println(F("ADD")); crashout(); }
//     if(!this->add(pca, true)) {
//       // Serial.println(F("ADD")); crashout(); }
//       Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!wiipod.add(*k30, true)) { Serial.println(F("ADD")); crashout(); }

//     // k30 = (K30*)(modules[MODULE]->operator[]("K30")->operator()(pca_fqa));

//   }

//   #ifdef WIIPOD_DEBUG_SERIAL
//     WIIPOD_DEBUG_SERIAL.print("[");
//     WIIPOD_DEBUG_SERIAL.print(PCA9685::getID());
//     WIIPOD_DEBUG_SERIAL.print(F("] "));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(pca_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(pca_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(pca_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(pca_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(" | "));
//   #endif

//   i2cip_pca9685_chsel_t b = PCA9685_CH0;
//   // i2cip_pca9685_t s = 2048;
//   i2cip_pca9685_t s = flash ? 0x1000 : 0x0000;
//   flash = !flash;

//   i2cip_args_io_t args = {.s = &s, .b = &b}; // No input
//   unsigned long now = millis();
//   errlev = this->operator()(pca, update, args);
//   unsigned long delta = millis() - now;

//   #ifdef WIIPOD_DEBUG_SERIAL
//   if(errlev == I2CIP::I2CIP_ERR_NONE) {
//     WIIPOD_DEBUG_SERIAL.print(F("PASS "));
//     WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
//     WIIPOD_DEBUG_SERIAL.print(F("s"));

//     if(!update) { WIIPOD_DEBUG_SERIAL.println(); return errlev; }

//     WIIPOD_DEBUG_SERIAL.print(F(" | PWM CH #"));
//     WIIPOD_DEBUG_SERIAL.print(b, HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(" SET: "));
//     WIIPOD_DEBUG_SERIAL.println(s);
//   } else {
//     WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
//     WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
//   }
//   #endif

//   return errlev;
// }

// i2cip_errorlevel_t WiiPod::updateADS1115(uint8_t bus, bool update) {
//   i2cip_errorlevel_t errlev;
//   i2cip_fqa_t ads_fqa = I2CIP::createFQA(getWireNum(), getModuleNum(), bus, I2CIP_ADS1115_ADDRESS);
//   if(ads == nullptr) {
//     // if(!modules[MODULE]->add(*K30::k30Factory(k30_fqa), true)) { Serial.println(F("ADD")); crashout(); }
//     // k30 = (K30*)modules[MODULE]->operator[](k30_fqa);
//     // if(k30 == nullptr || k30->getFQA() != k30_fqa) { Serial.println(F("FQA")); crashout(); }
//     // if(modules[MODULE]->operator[]("K30") == nullptr) { Serial.println(F("DG")); crashout(); }

//     // DeviceGroup* k30_dg = modules[MODULE]->operator[]("K30");
//     DeviceGroup* ads_dg = this->operator[](ADS1115::getID(i2cip_ads1115_id_progmem));
//     // DeviceGroup* k30_dg = wiipod["K30"];
//     if(ads_dg == nullptr) {
//       // delete k30_dg; k30_dg = nullptr; Serial.println("DG"); crashout(); }
//       Serial.println("DG"); return I2CIP::I2CIP_ERR_SOFT; }
//     ads = (ADS1115*)(ads_dg->operator()(ads_fqa));
//     if(ads == nullptr || ads->getFQA() != ads_fqa) {
//       // Serial.println(F("FQA")); crashout(); }
//       Serial.println(F("FQA")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!modules[MODULE]->add(*k30, true)) { Serial.println(F("ADD")); crashout(); }
//     if(!this->add(ads, true)) {
//       // Serial.println(F("ADD")); crashout(); }
//       Serial.println(F("ADD")); return I2CIP::I2CIP_ERR_SOFT; }
//     // if(!wiipod.add(*k30, true)) { Serial.println(F("ADD")); crashout(); }

//     // k30 = (K30*)(modules[MODULE]->operator[]("K30")->operator()(ads_fqa));

//   }

//   #ifdef WIIPOD_DEBUG_SERIAL
//     WIIPOD_DEBUG_SERIAL.print("[");
//     WIIPOD_DEBUG_SERIAL.print(ADS1115::getID());
//     WIIPOD_DEBUG_SERIAL.print(F("] "));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(ads_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(ads_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(ads_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(":"));
//     WIIPOD_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(ads_fqa), HEX);
//     WIIPOD_DEBUG_SERIAL.print(F(" | "));
//   #endif

//   // errlev = modules[MODULE]->operator()(ads_fqa, update);
//   unsigned long now = millis();
//   errlev = this->operator()(ads, update);
//   unsigned long delta = millis() - now;
//   // errlev = wiipod(ads_fqa, update);
//   #ifdef WIIPOD_DEBUG_SERIAL
//   if(errlev == I2CIP::I2CIP_ERR_NONE) {
//     WIIPOD_DEBUG_SERIAL.print(F("PASS "));
//     WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
//     WIIPOD_DEBUG_SERIAL.print(F("s"));

//     if(!update) { WIIPOD_DEBUG_SERIAL.println(); return errlev; }

//     WIIPOD_DEBUG_SERIAL.print(F(" | CHANNEL 0: "));
//     WIIPOD_DEBUG_SERIAL.print((ads->getCache()), 3);
//     WIIPOD_DEBUG_SERIAL.println(F(" V"));
//   } else {
//     WIIPOD_DEBUG_SERIAL.print(F("FAIL "));
//     WIIPOD_DEBUG_SERIAL.println(errlev, HEX);
//   }
//   #endif

//   return errlev;
// }

void WiiPod::scanToPrint(uint8_t wire, uint8_t module, Stream& out) {
  out.print(F("[WIIPOD | SCAN] "));
  out.print(wire, HEX);
  out.print(F(":"));
  out.print(module, HEX);
  out.println(F(" START (~10s)"));
  for(uint8_t b = 0; b < 0b111; b++) // 3 bit i2c bus
  {
    out.print(F("[WIIPOD | SCAN] BUS "));
    out.print(b, HEX);
    out.println(F(" START"));
    bool first = true;
    for(uint8_t a = 0x08; a < 0b1111111; a++) // 7 bit i2c address; 0x00 - 0x07 reserved
    {
      if(a >= I2CIP_MUX_ADDR_MIN && a <= I2CIP_MUX_ADDR_MAX) { continue; }
      if(a >= 0x78 && a <= 0x7F) { continue; } // 0x78 - 0x7F reserved
      i2cip_fqa_t fqa = I2CIP::createFQA(wire, module, b, a);
      i2cip_errorlevel_t errlev = Device::pingTimeout(fqa, first, false, 10);
      first = false;
      if(errlev == I2CIP_ERR_NONE) {
        out.print(F("[WIIPOD | SCAN] "));
        out.print(wire, HEX);
        out.print(F(":"));
        out.print(module, HEX);
        out.print(F(":"));
        out.print(b, HEX);
        out.print(F(":"));
        out.print(a, HEX);
        out.println(F(" FOUND"));
      }
      delay(1);
    }
  }
}