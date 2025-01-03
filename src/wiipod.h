#ifndef WIIPOD_WIIPOD_H
#define WIIPOD_WIIPOD_H

#include <Arduino.h>
#include <I2CIP.h>

class WiiPod;
#define MAIN_CLASS_NAME WiiPod

// #include <sht31.h>
#include <ADS1115.h>
#include <K30.h>
#include <MCP23017.h>
#include <Nunchuck.h>
#include <PCA9685.h>
#include <Seesaw.h>
#include <SHT45.h>
#include <SSD1306.h>
#include <PCA9632.h>
#include "../src/debug.h"

// #define WIIPOD_BUS_NUNCHUCK 0
#define WIIPOD_BUS_SHT45  0
// #define WIIPOD_BUS_K30 1
// #define WIIPOD_BUS_MCP 0
#define WIIPOD_BUS_ROTARY 0
// #define WIIPOD_BUS_SCREEN 1
// #define WIIPOD_BUS_ADS 0
#define WIIPOD_BUS_PCA 1
#define WIIPOD_BUS_LCD 1 // I2CIP_MUX_BUS_FAKE

#define WIIPOD_RENDER_X  64
#define WIIPOD_RENDER_Y  36

#define WIIPOD_SERIAL_BAUD 115200

// initialize, check, update
class WiiPod : public I2CIP::Module {
  private:
    bool initialized = false; // MUX last ping success?

    bool flash = false;

    // Nunchuck* nunchuck = nullptr;
    // // SHT31* sht31 = nullptr;
    // SHT45* sht45 = nullptr;
    // K30* k30 = nullptr;
    // MCP23017* mcp = nullptr;
    // Seesaw_RotaryEncoder* seesaw = nullptr;
    // PCA9685* pca = nullptr;
    // ADS1115* ads = nullptr;

    friend i2cip_errorlevel_t overwriteEEPROM(const char* eeprom_contents);
  
    i2cip_fqa_t createFQA(uint8_t bus, uint8_t addr) { return I2CIP::createFQA(getWireNum(), getModuleNum(), bus, addr); }
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
    // i2cip_errorlevel_t updateSHT45(uint8_t bus, bool update = false);
    // i2cip_errorlevel_t updateNunchuck(uint8_t bus, bool update = false);
    // i2cip_errorlevel_t updateK30(uint8_t bus, bool update = false);
    // i2cip_errorlevel_t updateMCP23017(uint8_t bus, bool update = false);
    // i2cip_errorlevel_t updateRotaryEncoder(uint8_t bus, bool update = false);
    // i2cip_errorlevel_t updatePCA9685(uint8_t bus, bool update = false);
    // i2cip_errorlevel_t updateADS1115(uint8_t bus, bool update = false);


    // i2cip_errorlevel_t updateSHT31(bool update = false) { return updateSHT31(0, update); }
    // i2cip_errorlevel_t updateNunchuck(bool update = false) { return updateNunchuck(0, update); }
    // i2cip_errorlevel_t updateK30(bool update = false) { return updateK30(0, update); }

    // const wiipod_nunchuck_t* getNunchuckCache(void) { return nunchuck == nullptr ? (wiipod_nunchuck_t*)nullptr : &nunchuck->getCache();}
    // // const state_sht31_t* getSHT31Cache(void) { return sht31 == nullptr ? nullptr : &sht31->getCache(); }
    // const state_sht45_t* getSHT45Cache(void) { return sht45 == nullptr ? nullptr : &sht45->getCache(); }
    // const uint16_t* getK30Cache(void) { return k30 == nullptr ? nullptr : &k30->getCache(); }
    // const uint16_t* getMCP23017Cache(void) { return mcp == nullptr ? nullptr : &mcp->getCache(); }
    // const i2cip_rotaryencoder_t* getSeesawCache(void) { return seesaw == nullptr ? nullptr : &seesaw->getCache(); }
    // const float* getADS1115Cache(void) { return ads == nullptr ? nullptr : &ads->getCache(); }

    // #ifdef DEBUG_SERIAL
    // void printNunchuck(Stream& out = DEBUG_SERIAL)
    // #else
    // void printNunchuck(Stream& out)
    // #endif
    // {
    //   if(nunchuck == nullptr) { return; }
    //   nunchuck->printToScreen(out, WIIPOD_RENDER_X, WIIPOD_RENDER_Y, nunchuck->getCache().z, nunchuck->getCache().c);
    //   // nunchuck->printToScreen(out, WIIPOD_RENDER_X, WIIPOD_RENDER_Y);
    // }

    // i2cip_errorlevel_t renderNunchuck() { return (nunchuck == nullptr || screen == nullptr) ? I2CIP::I2CIP_ERR_HARD : nunchuck->printToScreen(screen, I2CIP_SSD1306_WIDTH, I2CIP_SSD1306_HEIGHT); }
    // i2cip_errorlevel_t renderRotary() { return (seesaw == nullptr || screen == nullptr) ? I2CIP::I2CIP_ERR_HARD : seesaw->printToScreen(screen, I2CIP_SSD1306_WIDTH, I2CIP_SSD1306_HEIGHT); }

    #ifdef DEBUG_SERIAL
    void scanToPrint(Stream& out = DEBUG_SERIAL);
    #else
    void scanToPrint(Stream& out);
    #endif
};

// Emulating what was done in I2CIP::Module with EEPROM* const eeprom
// template <class C> class WiiPod : public WiiPod, public DeviceGroup {
//   private:
//     C* c = nullptr; 
//   public:
//     WiiPod(const uint8_t& wire, const uint8_t& module) : WiiPod(wire, module) { }
//     virtual ~WiiPod() { delete c; }

//     operator[](const i2cip_fqa_t& fqa, bool update = false, bool recurse = true) {
//       if(contains(fqa)) {
//         return operator()(fqa, update);
//       } else {
//         if(add(*C::factory(fqa), true)) {
//           return operator()(fqa, update);
//         } else {
//           return I2CIP_ERR_SOFT;
//         }
//       }
//     }
// };

#endif