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


#ifndef ESP_DHT22_H
#define ESP_DHT22_H

#include <c_types.h>

// Structure representing DHT22 device.
typedef struct {
  float hum;             // Humidity.
  float temp;            // Temperature in Celsius.
  uint8_t gpio_num;      // The GPIO this device is connected to.
  uint32_t last_measure; // Last measure time. Uses system_get_time().
} esp_dht22_dev;

// Error codes.
typedef enum {
  ESP_DHT22_OK,
  ESP_DHT22_ERR_MEM,
  ESP_DHT22_ERR_DEV_NULL,
  ESP_DHT22_ERR_BAD_RESP_SIGNAL,
  ESP_DHT22_ERR_PARITY,
} esp_dht22_err;

// Espressif SDK missing includes.
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);


/**
 * Initialize DHT22.
 *
 * @param gpio_num The GPIO number DHT22 is connected to.
 */
void ICACHE_FLASH_ATTR
esp_dht22_init(uint8_t gpio_num);

/**
 * Get DHT22 structure.
 *
 * It's up to a caller to release the memory at some point.
 *
 * @param gpio_num The GPIO number connected to the data bus.
 *
 * @return Device.
 */
esp_dht22_dev *ICACHE_FLASH_ATTR
esp_dht22_new_dev(uint8_t gpio_num);

/**
 * Get temperature and humidity.
 *
 * NOTE: You must keep calls to this function at least 2s apart.
 *
 * @param device The device structure to set values on.
 *
 * @return Error code.
 */
esp_dht22_err ICACHE_FLASH_ATTR
esp_dht22_get(esp_dht22_dev *device);

#endif //ESP_DHT22_H
