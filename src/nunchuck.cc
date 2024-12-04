#include <nunchuck.hpp>

bool Nunchuck::_id_set = false;
char Nunchuck::_id[I2CIP_ID_SIZE];

using namespace I2CIP;

void Nunchuck::loadID(void) {
  uint8_t idlen = strlen_P(wiipod_nunchuck_id_progmem);

  // Read in PROGMEM
  for (uint8_t k = 0; k < idlen; k++) {
    char c = pgm_read_byte_near(wiipod_nunchuck_id_progmem + k);
    Nunchuck::_id[k] = c;
  }

  Nunchuck::_id[idlen] = '\0';
  Nunchuck::_id_set = true;

  #ifdef WIIPOD_DEBUG_SERIAL
    DEBUG_DELAY();
    WIIPOD_DEBUG_SERIAL.print(F("Nunchuck ID Loaded: '"));
    WIIPOD_DEBUG_SERIAL.print(Nunchuck::_id);
    WIIPOD_DEBUG_SERIAL.print(F("' @"));
    WIIPOD_DEBUG_SERIAL.println((uint16_t)Nunchuck::_id, HEX);
    DEBUG_DELAY();
  #endif
}

// Handles ID pointer assignment too
// NEVER returns nullptr, unless out of memory
Device* Nunchuck::nunchuckFactory(const i2cip_fqa_t& fqa, const i2cip_id_t& id) {
  if(!Nunchuck::_id_set || id == nullptr) {
    loadID();

    (Device*)(new Nunchuck(fqa, id == nullptr ? _id : id));
  }

  return (Device*)(new Nunchuck(fqa, id));
}

Device* Nunchuck::nunchuckFactory(const i2cip_fqa_t& fqa) { return nunchuckFactory(fqa, Nunchuck::getStaticIDBuffer()); }

Nunchuck::Nunchuck(const i2cip_fqa_t& fqa, const i2cip_id_t& id) : Device(fqa, id), InputInterface<wiipod_nunchuck_t, void*>((Device*)this) { }
Nunchuck::Nunchuck(const i2cip_fqa_t& fqa) : Nunchuck(fqa, Nunchuck::_id) { }

// Nunchuck::Nunchuck(const uint8_t& wire, const uint8_t& module, const uint8_t& addr) : Nunchuck(I2CIP_FQA_CREATE(wire, module, I2CIP_MUX_BUS_DEFAULT, addr)) { }

i2cip_errorlevel_t Nunchuck::readContents(wiipod_nunchuck_t& dest) {
  i2cip_errorlevel_t errlev = writeByte((uint8_t)0x00);
  I2CIP_ERR_BREAK(errlev);

  uint8_t temp[NUNCHUCK_READLEN];
  size_t len = NUNCHUCK_READLEN;
  errlev = read((uint8_t*)temp, len, false); // No null-terminator; we want raw data
  I2CIP_ERR_BREAK(errlev);

  if(len < NUNCHUCK_READLEN) {
    return I2CIP_ERR_SOFT;
  }

  // Serial.print("0b");
  // Serial.println(temp[0], BIN);
  // Serial.print("0b");
  // Serial.println(temp[1], BIN);
  // Serial.print("0b");
  // Serial.println(temp[2], BIN);
  // Serial.print("0b");
  // Serial.println(temp[3], BIN);
  // Serial.print("0b");
  // Serial.println(temp[4], BIN);
  // Serial.print("0b");
  // Serial.println(temp[5], BIN);

  dest.joy_x = temp[0];
  dest.joy_y = temp[1];
  dest.accel_x = (temp[2] << 2) | ((temp[5] & 0b11000000) >> 6);
  dest.accel_y = (temp[3] << 2) | ((temp[5] & 0b00110000) >> 4);
  dest.accel_z = (temp[4] << 2) | ((temp[5] & 0b00001100) >> 2);
  dest.c = !(temp[5] & 0b00000010);
  dest.z = !(temp[5] & 0b00000001);

  return errlev;
}

i2cip_errorlevel_t Nunchuck::get(wiipod_nunchuck_t& dest, void* const& args) {
  // 0. Check args
  if(args != nullptr) {
    return I2CIP_ERR_SOFT;
  }

  if(!initialized) {
    i2cip_errorlevel_t errlev = writeRegister((uint8_t)0xF0, (uint8_t)0x55, true);
    I2CIP_ERR_BREAK(errlev);
    errlev = writeRegister((uint8_t)0xFB, (uint8_t)0x00, true);
    I2CIP_ERR_BREAK(errlev);
    initialized = true;
  }

  #ifdef I2CIP_DEBUG_SERIAL
    DEBUG_DELAY();
    I2CIP_DEBUG_SERIAL.print(F("Nunchuck Initialized\n"));
    DEBUG_DELAY();
  #endif

  delay(10); // temporary

  return readContents(dest);
}

// G - Getter type: char* (null-terminated; writable heap)
void Nunchuck::clearCache(void) {
  this->setCache({ 0, 0, 0, 0, 0, false, false });

  #ifdef I2CIP_DEBUG_SERIAL
    DEBUG_DELAY();
    I2CIP_DEBUG_SERIAL.print(F("Nunchuck Cache Cleared (Zeroed)\n"));
    DEBUG_DELAY();
  #endif
}