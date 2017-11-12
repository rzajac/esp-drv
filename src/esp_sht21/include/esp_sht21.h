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


#ifndef ESP_SHT21_H
#define ESP_SHT21_H

#include <c_types.h>
#include <esp_i2c.h>

#define ESP_SHT21_ADDRESS 0x40
// Measure relative humidity. Hold Master Mode.
#define ESP_SHT21_RH_HM 0xE5
// Measure relative humidity. No Hold Master Mode.
#define ESP_SHT21_RH_NHM 0xF5
// Measure temperature. Hold Master Mode.
#define ESP_SHT21_TEMP_HM 0xE3
// Measure temperature. No Hold Master Mode.
#define ESP_SHT21_TEMP_NHM 0xF3
// Read temperature from previous RH measurement.
#define ESP_SHT21_TEMP_LAST 0xE0
// Read User Register 1.
#define ESP_SHT21_UR1_READ 0xE7
// Write User Register 1.
#define ESP_SHT21_UR1_WRITE 0xE6
// Read Heater Control Register
#define ESP_SHT21_HCR_READ 0x11
// Write Heater Control Register
#define ESP_SHT21_HCR_WRITE 0x51

// Invalid humidity.
#define ESP_SHT21_BAD_RH ((float)-1.00)
// Invalid temperature.
#define ESP_SHT21_BAD_TEMP ((float)-273)

// SHT21 humidity and temperature resolutions.
// RH: 12bit TEMP: 14bit
#define ESP_SHT21_RES3 0x0
// RH: 8bit TEMP: 12bit
#define ESP_SHT21_RES2 0x1
// RH: 10bit TEMP: 13bit
#define ESP_SHT21_RES1 0x2
// RH: 11bit TEMP: 11bit
#define ESP_SHT21_RES0 0x3

/**
 * Initialize SHT21.
 *
 * @param gpio_scl The GPIO pin used for clock.
 * @param gpio_sda The GPIO pin used for data.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_init(uint8_t gpio_scl, uint8_t gpio_sda);

/**
 * Measure humidity.
 *
 * It takes approximately around 20ms to return the value.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_rh(float *humidity);

/**
 * Measure temperature.
 *
 * This function slow. Faster method is to use esp_sht21_get_temp_last
 * function which will return temperature value from previous
 * humidity measurement.
 *
 * @param temp The measured temperature.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_temp(float *temp);

/**
 * Get temperature from previous humidity measurement.
 *
 * This function is faster then esp_sht21_get_temp.
 *
 * @param temp The measured temperature.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_temp_last(float *temp);

/**
 * Get 64 bit unique SHT21 serial number.
 *
 * @param sn The pointer to 8 byte array.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_sn(uint8_t *sn);

/**
 * Get firmware revision.
 *
 * @param rev The revision to set.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_rev(uint8_t *rev);

/**
 * Get humidity and temperature measurement resolution.
 *
 * @param res The measurement resolution. One of the ESP_SHT21_RES* defines.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_res_get(uint8_t *res);

/**
 * Set SHT21 humidity and temperature measurement resolution.
 *
 * @param res The resolution. One of the ESP_SHT21_RES* defines.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_res_set(uint8_t res);

/**
 * Get on-board heater status.
 *
 * @param on_off The on/off status.
 * @param level  The heater level value 0-15.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_heater_get(bool *on_off, uint8_t *level);

/**
 * Set on-board heater status.
 *
 * @param on_off The on/off status.
 * @param level  The heater level value 0-15.
 *
 * @return The I2C error code.
 */
esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_heater_set(bool on_off, uint8_t level);

#endif //ESP_SHT21_H
