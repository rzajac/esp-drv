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

#include <user_interface.h>
#include <esp_gpio.h>
#include <esp_dht22.h>
#include <mem.h>
#include <esp_sdo.h>

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
sys_init_done(void* arg)
{
  esp_dht22_err err;
  esp_dht22_dev *dev = esp_dht22_new_dev(GPIO2);

  // Get temperature.
  err = esp_dht22_get(dev);
  if (err != ESP_DHT22_OK) {
    os_free(dev);
    os_printf("DHT22 error code: %d\n", err);
    return;
  }

  os_printf("Temp: %s\n", ftoa(dev->temp, 2));
  os_printf("Hum: %s\n", ftoa(dev->hum, 2));
  os_printf("--------------------\n");
  os_free(dev);
}

void ICACHE_FLASH_ATTR
user_init()
{
  // No need for wifi for this example.
  wifi_station_disconnect();
  wifi_set_opmode_current(NULL_MODE);

  //system_init_done_cb(sys_init_done);
  stdout_init(BIT_RATE_74880);

  // Initialize DHT22 on GPIO 2.
  esp_dht22_init(GPIO2);

  // Wait before running main code.
  os_printf("Initialized.\n");
  os_timer_disarm(&timer);
  os_timer_setfn(&timer, sys_init_done, NULL);
  os_timer_arm(&timer, 1500, true);
}
