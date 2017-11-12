/*
 * Copyright 2017 Rafal Zajac <rzajac@gmail.com>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include <esp_sht21.h>
#include <esp_sdo.h>
#include <user_interface.h>
#include <osapi.h>

#define SCL GPIO0
#define SDA GPIO2

os_timer_t timer;

/**
 * Rise base to power of.
 *
 * From: http://bbs.espressif.com/viewtopic.php?t=246
 *
 * @param base The number.
 * @param exp  The exponent.
 *
 * @return Product.
 */
static int ICACHE_FLASH_ATTR
power(int base, int exp)
{
  int result = 1;
  while (exp) {
    result *= base;
    exp--;
  }

  return result;
}

/**
 * Get string representation of float.
 *
 * From: http://bbs.espressif.com/viewtopic.php?t=246
 *
 * Warning: limited to 15 chars & non-reentrant.
 *          e.g., don't use more than once per os_printf call.
 *
 * @param num       The float to convert to string.
 * @param decimals  The number of decimal places.
 *
 * @return The float string representation.
 */
static char *ICACHE_FLASH_ATTR
ftoa(float num, uint8_t decimals)
{
  static char *buf[16];

  int whole = (int) num;
  int decimal = (int) ((num - whole) * power(10, decimals));
  if (decimal < 0) {
    // get rid of sign on decimal portion
    decimal -= 2 * decimal;
  }

  char *pattern[10]; // setup printf pattern for decimal portion
  os_sprintf((char *) pattern, "%%d.%%0%dd", decimals);
  os_sprintf((char *) buf, (const char *) pattern, whole, decimal);

  return (char *) buf;
}

void ICACHE_FLASH_ATTR
run_sht21()
{
  uint8_t rev;
  float value;
  uint8_t sn[8];
  esp_i2c_err err;

  err = esp_sht21_init(SCL, SDA);
  if (err != ESP_I2C_OK) {
    os_printf("SHT21 init error.\n");
    return;
  }

  err = esp_sht21_get_sn(sn);
  if (err != ESP_I2C_OK) os_printf("SHT21 sn error %d.\n", err);

  os_printf("SHT21 SN: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
            sn[0], sn[1], sn[2], sn[3],
            sn[4], sn[5], sn[6], sn[7]);

  err = esp_sht21_get_rev(&rev);
  if (err != ESP_I2C_OK) os_printf("SHT21 sn error %d.\n", err);

  os_printf("Firmware rev: 0x%02X\n", rev);

  err = esp_sht21_get_rh(&value);
  if (err != ESP_I2C_OK) os_printf("Get RH error: %d\n", err);

  os_printf("Humidity: %s%%\n", ftoa(value, 2));

  err = esp_sht21_get_temp_last(&value);
  if (err != ESP_I2C_OK) os_printf("Get TEMP error: %d\n", err);

  os_printf("Temperature: %s deg. C\n", ftoa(value, 2));
}

void ICACHE_FLASH_ATTR
user_init()
{
  // We don't need WiFi for this example.
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE);

  stdout_init(BIT_RATE_74880);
  os_printf("Starting...\n");

  os_timer_disarm(&timer);
  os_timer_setfn(&timer, (os_timer_func_t *) run_sht21, NULL);
  os_timer_arm(&timer, 1500, false);
}
