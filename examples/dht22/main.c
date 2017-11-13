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


#include <esp_gpio.h>
#include <esp_dht22.h>
#include <esp_sdo.h>
#include <esp_util.h>
#include <user_interface.h>
#include <mem.h>

os_timer_t timer;


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

  os_printf("Temp: %s\n", esp_util_ftoa(dev->temp, 3));
  os_printf("Hum: %s\n", esp_util_ftoa(dev->hum, 3));
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
