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
#include <osapi.h>
#include <esp_ds18b20.h>
#include <esp_gpio.h>
#include <esp_sdo.h>

// List of found devices on the OneWire bus.
static esp_ow_device *root = NULL;

static void ICACHE_FLASH_ATTR
sys_init_done()
{
  esp_ow_err err;

  if (esp_ds18b20_init(GPIO2) == false) {
    os_printf("Error initializing DS18B20.\n");
    return;
  }

  err = esp_ds18b20_search(GPIO2, false, &root);
  if (err != ESP_OW_OK) {
    os_printf("Search error: %d\n", err);
    return;
  }

  // Log found devices to serial.
  esp_ow_dump_found(root);
}

void ICACHE_FLASH_ATTR
user_init()
{
  // No need for wifi for this examples.
  wifi_station_disconnect();
  wifi_set_opmode_current(NULL_MODE);

  stdout_init(BIT_RATE_74880);
  system_init_done_cb(sys_init_done);
}
