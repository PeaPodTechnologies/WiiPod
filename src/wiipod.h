#ifndef WIIPOD_WIIPOD_H
#define WIIPOD_WIIPOD_H

#include <Arduino.h>
#include <I2CIP.h>

class WiiPod;
#define MAIN_CLASS_NAME WiiPod

#include <Nunchuck.h>
// #include <sht31.h>
#include <SHT45.h>
#include <K30.h>
#include <MCP23017.h>
#include <Seesaw.h>
#include <SSD1306.h>
#include <PCA9685.h>
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
    MCP23017* mcp = nullptr;
    Seesaw_RotaryEncoder* seesaw = nullptr;
    PCA9685* pca = nullptr;

    uint8_t wirenum, modulenum;

    friend i2cip_errorlevel_t overwriteEEPROM(const char* eeprom_contents);
  public:
    WiiPod(const uint8_t& wire, const uint8_t& module);
    virtual ~WiiPod() { } // { delete nunchuck; delete sht31; }
    
    I2CIP::DeviceGroup* deviceGroupFactory(const i2cip_id_t& id) override;

    // Future
    // bool parseEEPROMContents(const char* eeprom_contents) override;

    SSD1306* screen = nullptr;

    bool initialize(); // Does Discover
    i2cip_errorlevel_t check();
    i2cip_errorlevel_t update();

    // i2cip_errorlevel_t updateSHT31(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updateSHT45(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updateNunchuck(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updateK30(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updateMCP23017(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updateRotaryEncoder(uint8_t bus, bool update = false);
    i2cip_errorlevel_t updatePCA9685(uint8_t bus, bool update = false);

    // i2cip_errorlevel_t updateSHT31(bool update = false) { return updateSHT31(0, update); }
    // i2cip_errorlevel_t updateNunchuck(bool update = false) { return updateNunchuck(0, update); }
    // i2cip_errorlevel_t updateK30(bool update = false) { return updateK30(0, update); }

    const wiipod_nunchuck_t* getNunchuckCache(void) { return nunchuck == nullptr ? (wiipod_nunchuck_t*)nullptr : &nunchuck->getCache();}
    // const state_sht31_t* getSHT31Cache(void) { return sht31 == nullptr ? nullptr : &sht31->getCache(); }
    const state_sht45_t* getSHT45Cache(void) { return sht45 == nullptr ? nullptr : &sht45->getCache(); }
    const uint16_t* getK30Cache(void) { return k30 == nullptr ? nullptr : &k30->getCache(); }
    const uint16_t* getMCP23017Cache(void) { return mcp == nullptr ? nullptr : &mcp->getCache(); }
    const i2cip_rotaryencoder_t* getSeesawCache(void) { return seesaw == nullptr ? nullptr : &seesaw->getCache(); }

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

    i2cip_errorlevel_t renderNunchuck() { return (nunchuck == nullptr || screen == nullptr) ? I2CIP::I2CIP_ERR_HARD : nunchuck->printToScreen(screen, I2CIP_SSD1306_WIDTH, I2CIP_SSD1306_HEIGHT); }
    i2cip_errorlevel_t renderRotary() { return (seesaw == nullptr || screen == nullptr) ? I2CIP::I2CIP_ERR_HARD : seesaw->printToScreen(screen, I2CIP_SSD1306_WIDTH, I2CIP_SSD1306_HEIGHT); }

    #ifdef DEBUG_SERIAL
    void scanToPrint(Stream& out = DEBUG_SERIAL, uint8_t wire, uint8_t module);
    #else
    void scanToPrint(Stream& out, uint8_t wire, uint8_t module);
    #endif
};

#endif