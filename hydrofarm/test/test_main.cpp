#include <Arduino.h>
#include <unity.h>

#ifdef UNIT_TEST

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_led_builtin_pin_number(void) {
    TEST_ASSERT_EQUAL(LED_BUILTIN, 13);
}

void test_led_state_high(void) {
    digitalWrite(LED_BUILTIN, HIGH);
    TEST_ASSERT_EQUAL(digitalRead(LED_BUILTIN), HIGH);
}

void test_led_state_low(void) {
    digitalWrite(LED_BUILTIN, LOW);
    TEST_ASSERT_EQUAL(digitalRead(LED_BUILTIN), LOW);
}

void test_eeprom(void)
{
  #include "config.h"
  config_type config_bkp;
  config_type config_in_eeprom EEMEM;
  eeprom_read_block( &config_bkp, &config_in_eeprom, sizeof(config_bkp) );
}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // IMPORTANT LINE!
    RUN_TEST(test_led_builtin_pin_number);

    pinMode(LED_BUILTIN, OUTPUT);
}

uint8_t i = 0;
uint8_t max_blinks = 5;

void loop() {
    if (i < max_blinks)
    {
        //RUN_TEST(test_led_state_high);
        //delay(500);
        //RUN_TEST(test_led_state_low);
        delay(500);
        i++;
    }
    else if (i == max_blinks) {

      test_eeprom(;
      UNITY_END(); // stop unit testing
    }
}

#endif
