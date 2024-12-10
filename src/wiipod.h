#ifndef WIIPOD_WIIPOD_H
#define WIIPOD_WIIPOD_H

#include <Arduino.h>
#include <I2CIP.h>

class WiiPod;
#define MAIN_CLASS_NAME WiiPod

#include <nunchuck.h>
// #include <sht31.h>
#include <sht45.h>
#include <k30.h>
#include "../src/debug.h"

#define WIIPOD_RENDER_X  64
#define WIIPOD_RENDER_Y  36

#define WIIPOD_SERIAL_BAUD 115200

class WiiPod : private I2CIP::Module {
  private:
    bool initialized = false; // MUX last ping success?

    Nunchuck* nunchuck = nullptr;
    // SHT31* sht31 = nullptr;
    SHT45* sht45 = nullptr;
    K30* k30 = nullptr;

    uint8_t wirenum, modulenum;

    friend i2cip_errorlevel_t overwriteEEPROM(const char* eeprom_contents);
  public:
    WiiPod(const uint8_t& wire, const uint8_t& module);
    virtual ~WiiPod() { } // { delete nunchuck; delete sht31; }
    
    I2CIP::DeviceGroup* deviceGroupFactory(const i2cip_id_t& id) override;

    // Future
    // bool parseEEPROMContents(const char* eeprom_contents) override;

    bool initialize(); // Does Discover
    i2cip_errorlevel_t check();
    i2cip_errorlevel_t update();

    // i2cip_errorlevel_t updateSHT31(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updateSHT45(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updateNunchuck(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updateK30(uint8_t bus, bool update = false);
    // i2cip_errorlevel_t updateSHT31(bool update = false) { return updateSHT31(0, update); }
    // i2cip_errorlevel_t updateNunchuck(bool update = false) { return updateNunchuck(0, update); }
    // i2cip_errorlevel_t updateK30(bool update = false) { return updateK30(0, update); }

    const wiipod_nunchuck_t* getNunchuckCache(void) { return nunchuck == nullptr ? (wiipod_nunchuck_t*)nullptr : &nunchuck->getCache();}
    // const state_sht31_t* getSHT31Cache(void) { return sht31 == nullptr ? nullptr : &sht31->getCache(); }
    const state_sht45_t* getSHT45Cache(void) { return sht45 == nullptr ? nullptr : &sht45->getCache(); }
    const uint16_t* getK30Cache(void) { return k30 == nullptr ? nullptr : &k30->getCache(); }

    #ifdef DEBUG_SERIAL
    void printNunchuck(Stream& out = DEBUG_SERIAL)
    #else
    void printNunchuck(Stream& out)
    #endif
    {
      if(nunchuck == nullptr) { return; }
      nunchuck->printToScreen(out, WIIPOD_RENDER_X, WIIPOD_RENDER_Y, nunchuck->getCache().z, nunchuck->getCache().c);
      // nunchuck->printToScreen(out, WIIPOD_RENDER_X, WIIPOD_RENDER_Y);
    }
};

#endif