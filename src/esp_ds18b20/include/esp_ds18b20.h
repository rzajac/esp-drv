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

#ifndef ESP_DS18B20_H
#define ESP_DS18B20_H

#include <esp_ow.h>
#include <c_types.h>

// The DS18B20 family code from datasheet.
#define ESP_DS18B20_FAMILY_CODE 0x28

// The absolute zero temperature is returned as error.
#define ESP_DS18B20_TEMP_ERR (-273)

// Temperature conversion ready.
#define ESP_DS18B20_EV_TEMP_READY "ds18b20tReady"
// Temperature conversion error.
#define ESP_DS18B20_EV_TEMP_ERROR "ds18b20tError"

// Temperature resolutions.
#define ESP_DS18B20_RES_9 0x0
#define ESP_DS18B20_RES_10 0x1
#define ESP_DS18B20_RES_11 0x2
#define ESP_DS18B20_RES_12 0x3

// Temperature steps.
#define ESP_DS18B20_STEP_9 0.5
#define ESP_DS18B20_STEP_10 0.25
#define ESP_DS18B20_STEP_11 0.125
#define ESP_DS18B20_STEP_12 0.0625

// OneWire commands.
typedef enum {
  ESP_DS18B20_CMD_READ_PWR = 0xB4,
  ESP_DS18B20_CMD_CONVERT = 0x44,
  ESP_DS18B20_CMD_READ_SP = 0xBE,
  ESP_DS18B20_CMD_WRITE_SP = 0x4E,
} esp_ds18b20_cmd;

typedef enum {
  ESP_DS18B20_OK,
  ESP_DS18B20_NO_DEV,
  ESP_DS18B20_ERR_CONV_IN_PROG, // Conversion in progress.
} esp_ds18b20_err;

// DS18B20 status.
typedef struct {
  uint8_t sp[9];
  int8_t retries;  // Is greater then zero when conversion in progress.
  float last_temp; // Last successful temperature read.
} esp_ds18b20_st;


/**
 * Initialize OneWire bus where DS18B20 is.
 *
 * To use many OneWire buses you have to initialize all of them.
 *
 * @param gpio_num The GPIO where OneWire bus is connected.
 *
 * @return Returns true on success, false otherwise.
 */
bool ICACHE_FLASH_ATTR
esp_ds18b20_init(uint8_t gpio_num);

/**
 * Find devices on OneWire bus.
 *
 * @param in_alert Find only devices in alert mode.
 * @param list     The list of found devices or NULL.
 *
 * @return OneWire error code.
 */
esp_ow_err ICACHE_FLASH_ATTR
esp_ds18b20_search(uint8_t gpio_num, bool in_alert, esp_ow_device **list);

/**
 * Construct DS18B20 device.
 *
 * It's up to a caller to release the memory at some point.
 *
 * @param rom The pointer to 8 byte ROM address
 *
 * @return The device.
 */
esp_ow_device *ICACHE_FLASH_ATTR
esp_ds18b20_new_dev(uint8_t *rom);

/**
 * Get the only device on the bus.
 *
 * Can be used only when there is only one device on the OneWire bus!
 *
 * @param gpio_num The GPIO where OneWire bus is connected.
 *
 * @return The OneWire device. On error ROM address has all zeros.
 */
esp_ow_device *ICACHE_FLASH_ATTR
esp_ds18b20_get(uint8_t gpio_num);

/**
 * Get DS18B20 alarm thresholds.
 *
 * @param dev  The device to get alarm thresholds for.
 * @param low  The low threshold in Celsius.
 * @param high The high threshold in Celsius.
 *
 * @return OneWire error code.
 */
esp_ow_err ICACHE_FLASH_ATTR
esp_ds18b20_get_alarm(esp_ow_device *dev, int8_t *low, int8_t *high);

/**
 * Set DS18B20 alarm thresholds.
 *
 * @param dev  The device to get alarm thresholds for.
 * @param low  The low threshold in Celsius.
 * @param high The high threshold in Celsius.
 *
 * @return OneWire error code.
 */
esp_ow_err ICACHE_FLASH_ATTR
esp_ds18b20_set_alarm(esp_ow_device *dev, int8_t low, int8_t high);

/**
 * Read scrachpad.
 *
 * @param device The device to read scratchpad for.
 *
 * @return OneWire error code.
 */
esp_ow_err ICACHE_FLASH_ATTR
esp_d18b20_read_sp(esp_ow_device *device);

/**
 * Write scratchpad to the device.
 *
 * @param device The device to write scratchpad to.
 *
 * @return OneWire error code.
 */
esp_ow_err ICACHE_FLASH_ATTR
esp_ds18b20_write_sp(esp_ow_device *device);

/**
 * Free memory allocated by devices found on OneWire bus.
 *
 * @param list
 */
void ICACHE_FLASH_ATTR
esp_ds18b20_free_list(esp_ow_device *list);

/**
 * Start temperature conversion.
 *
 * @param device The device to start conversion on.
 *
 * @return Error code.
 */
esp_ds18b20_err ICACHE_FLASH_ATTR
esp_ds18b20_convert(esp_ow_device *device);

/**
 * Check if OneWire bus has device with parasite power supply.
 *
 * @param gpio_num The GPIO where OneWire bus is connected.
 *
 * @return Returns true if parasite present, false otherwise.
 */
bool ICACHE_FLASH_ATTR
esp_ds18b20_has_parasite(uint8_t gpio_num);

#endif //ESP_DS18B20_H
