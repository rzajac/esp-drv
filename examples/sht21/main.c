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
#include <esp_util.h>
#include <user_interface.h>

#define SCL GPIO0
#define SDA GPIO2

os_timer_t timer;

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

  os_printf("Humidity: %s%%\n", esp_util_ftoa(value, 2));

  err = esp_sht21_get_temp_last(&value);
  if (err != ESP_I2C_OK) os_printf("Get TEMP error: %d\n", err);

  os_printf("Temperature: %s deg. C\n", esp_util_ftoa(value, 2));
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
