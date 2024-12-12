#include <Arduino.h>
#include <unity.h>

#include <wiipod.h>
#include "../config.h"

#define WIRENUM 0
#define MODULE  0

i2cip_fqa_t eeprom_fqa = I2CIP::createFQA(WIRENUM, MODULE, I2CIP_MUX_BUS_DEFAULT, I2CIP_EEPROM_ADDR);
char buffer[I2CIP_TEST_BUFFERSIZE] = { '\0' };
uint16_t bufferlen = 0;

const char* eeprom_contents = "[{\"24LC32\":[80],\"SHT31\":[68],\"NUNCHUCK\":[82]}]";

WiiPod* wiipod = nullptr;

// ALL MACROS

void test_wiipod_ping(void) {
  bool r = wiipod->initialize();
  TEST_ASSERT_TRUE_MESSAGE(r, "WiiPod Ping");
}

void test_wiipod_update(void) {
  I2CIP::i2cip_errorlevel_t result = wiipod->update();
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(I2CIP::I2CIP_ERR_NONE, result, "WiiPod Update");
}

void test_wiipod_nunchuck(void) {
  I2CIP::i2cip_errorlevel_t result = wiipod->updateNunchuck(0, true);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(I2CIP::I2CIP_ERR_NONE, result, "WiiPod Nunchuck");
}
void test_wiipod_k30(void) {
  I2CIP::i2cip_errorlevel_t result = wiipod->updateK30(1, true);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(I2CIP::I2CIP_ERR_NONE, result, "WiiPod K30");
}

i2cip_errorlevel_t overwriteEEPROM(const char* eeprom_contents) {
  OutputSetter* setter = wiipod->operator const I2CIP::EEPROM &().getOutput();
  if(setter == nullptr) {
    // Output removed; Read-only
    TEST_ASSERT_TRUE_MESSAGE(false, "EEPROM Read-Only (OK!)");
  }
  return setter->set(eeprom_contents);
}

void test_eeprom_overwrite_contents(void) {
  i2cip_errorlevel_t result = overwriteEEPROM(eeprom_contents);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(I2CIP::I2CIP_ERR_NONE, result, "EEPROM Overwrite Contents");
}

void setup() {
  delay(2000);

  UNITY_BEGIN();

  wiipod = new WiiPod(WIRENUM, MODULE);

  RUN_TEST(test_wiipod_ping);
  delay(1000);

  RUN_TEST(test_eeprom_overwrite_contents);
  delay(1000);
}

void loop() {

  RUN_TEST(test_wiipod_update);
  delay(1000);

  RUN_TEST(test_wiipod_nunchuck);
  delay(1000);

  RUN_TEST(test_wiipod_k30);
  delay(1000);
}