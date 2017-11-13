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

static uint8_t ICACHE_FLASH_ATTR
calc_crc(uint8_t init, const uint8_t *data, uint8_t len)
{
  uint8_t idx;
  uint8_t bit;

  // Calculates 8 bit checksum with polynomial 0x131 (10011001).
  for (idx = 0; idx < len; idx++) {
    init ^= (data[idx]);
    for (bit = 0; bit < 8; bit++) {
      if (init & 0x80) init = (uint8_t) ((init << 1) ^ 0x131);
      else init = (init << 1);
    }
  }

  return init;
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_init(uint8_t gpio_scl, uint8_t gpio_sda)
{
  return esp_i2c_init(gpio_scl, gpio_sda);
}

static float ICACHE_FLASH_ATTR
calc_rh(const uint8_t *data)
{
  float hu;
  hu = (float) (125.0 / 65536.0);
  hu = hu * (((data[0] << 8) | data[1]) & ~0x3);

  return hu - 6;
}

static float ICACHE_FLASH_ATTR
calc_temp(const uint8_t *data)
{
  float temp;
  temp = (float) (175.72 / 65536.0);
  temp = temp * (((data[0] << 8) | data[1]) & ~0x3);

  return (float) (temp - 46.85);
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_rh(float *humidity)
{
  esp_i2c_err err;
  uint8_t data[3];

  *humidity = ESP_SHT21_BAD_RH;

  err = esp_i2c_start_read(ESP_SHT21_ADDRESS, ESP_SHT21_RH_HM);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_read_bytes(data, 3);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_stop();

  if (calc_crc(0x0, data, 2) == data[2]) {
    *humidity = calc_rh(data);
  } else if (err == ESP_I2C_OK) {
    err = ESP_I2C_ERR_DATA_CORRUPTED;
  }

  return err;
}

/**
 * Get temperature.
 *
 * @param temp The temperature.
 * @param cmd  The temperature command.
 *
 * @return The I2C error code.
 */
static esp_i2c_err ICACHE_FLASH_ATTR
get_temp(float *temp, uint8_t cmd)
{
  esp_i2c_err err;
  uint8_t data[3];

  // When getting temperature from previous humidity measurement
  // the CRC checksum is not available.
  uint8_t data_len = (uint8_t) (cmd == ESP_SHT21_TEMP_HM ? 3 : 2);

  *temp = ESP_SHT21_BAD_TEMP;

  err = esp_i2c_start_read(ESP_SHT21_ADDRESS, cmd);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_read_bytes(data, data_len);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_stop();

  if (data_len == 2 || calc_crc(0x0, data, 2) == data[2]) {
    *temp = calc_temp(data);
  } else if (err == ESP_I2C_OK) {
    err = ESP_I2C_ERR_DATA_CORRUPTED;
  }

  return err;
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_temp(float *temp)
{
  return get_temp(temp, ESP_SHT21_TEMP_HM);
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_temp_last(float *temp)
{
  return get_temp(temp, ESP_SHT21_TEMP_LAST);
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_sn(uint8_t *sn)
{
  uint8_t idx;
  esp_i2c_err err;
  uint8_t sn_idx = 0;
  uint8_t crc;
  uint8_t cmd1[2] = {0xFA, 0x0F};
  uint8_t cmd2[2] = {0xFC, 0xC9};
  uint8_t data[14];

  err = esp_i2c_start_read_write(ESP_I2C_ADDR_WRITE(ESP_SHT21_ADDRESS), true);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_write_bytes(cmd1, 2);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_start_read_write(ESP_I2C_ADDR_READ(ESP_SHT21_ADDRESS), true);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_read_bytes(data, 8);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_stop();
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_start_read_write(ESP_I2C_ADDR_WRITE(ESP_SHT21_ADDRESS), true);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_write_bytes(cmd2, 2);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_start_read_write(ESP_I2C_ADDR_READ(ESP_SHT21_ADDRESS), true);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_read_bytes((data + 8), 6);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_stop();

  // Validate data.
  crc = 0x0;
  for (idx = 0; idx < 8; idx += 2) {
    crc = calc_crc(crc, &data[idx], 1);
    if (data[idx + 1] != crc) {
      return ESP_I2C_ERR_DATA_CORRUPTED;
    }
    sn[sn_idx++] = data[idx];
  }

  crc = 0x0;
  for (idx = 8; idx < 14; idx += 3) {
    crc = calc_crc(crc, &data[idx], 2);
    if (data[idx + 2] != crc) {
      return ESP_I2C_ERR_DATA_CORRUPTED;
    }
    sn[sn_idx++] = data[idx];
    sn[sn_idx++] = data[idx + 1];
  }

  return err;
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_get_rev(uint8_t *rev)
{
  esp_i2c_err err;
  uint8_t cmd[2] = {0x84, 0xB8};

  err = esp_i2c_start_read_write(ESP_I2C_ADDR_WRITE(ESP_SHT21_ADDRESS), true);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_write_bytes(cmd, 2);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_start_read_write(ESP_I2C_ADDR_READ(ESP_SHT21_ADDRESS), true);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_read_bytes(rev, 1);
  if (err != ESP_I2C_OK) return err;

  return esp_i2c_stop();
}

static esp_i2c_err ICACHE_FLASH_ATTR
register_get(uint8_t address, uint8_t reg_adr, uint8_t *reg)
{
  esp_i2c_err err;

  // Read the register.
  err = esp_i2c_start_read(address, reg_adr);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_read_bytes(reg, 1);
  if (err != ESP_I2C_OK) return err;

  return esp_i2c_stop();
}

static esp_i2c_err ICACHE_FLASH_ATTR
register_set(uint8_t address, uint8_t reg_adr, uint8_t value)
{
  esp_i2c_err err;

  err = esp_i2c_start_write(address, reg_adr);
  if (err != ESP_I2C_OK) return err;

  err = esp_i2c_write_bytes(&value, 1);
  if (err != ESP_I2C_OK) return err;

  return esp_i2c_stop();
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_res_get(uint8_t *res)
{
  esp_i2c_err err;
  uint8_t reg1 = 0;

  err = register_get(ESP_SHT21_ADDRESS, ESP_SHT21_UR1_READ, &reg1);
  if (err != ESP_I2C_OK) return err;

  *res = (uint8_t) (((reg1 >> 6) & 0x2) | (reg1 & 0x1));

  return ESP_I2C_OK;
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_res_set(uint8_t res)
{
  esp_i2c_err err;
  uint8_t reg1 = 0;

  err = register_get(ESP_SHT21_ADDRESS, ESP_SHT21_UR1_READ, &reg1);
  if (err != ESP_I2C_OK) return err;

  // Clear bits 7 and 0
  reg1 = (uint8_t) (reg1 & 0x7E);
  // Logical or after some bit manipulation.
  reg1 |= ((((res & 0x3) != 0) << 7) | ((res & 0x1) != 0));

  return register_set(ESP_SHT21_ADDRESS, ESP_SHT21_UR1_WRITE, reg1);
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_heater_get(bool *on_off, uint8_t *level)
{
  esp_i2c_err err;
  uint8_t reg1 = 0;
  uint8_t hcr = 0;

  err = register_get(ESP_SHT21_ADDRESS, ESP_SHT21_UR1_READ, &reg1);
  if (err != ESP_I2C_OK) return err;
  err = register_get(ESP_SHT21_ADDRESS, ESP_SHT21_HCR_READ, &hcr);
  if (err != ESP_I2C_OK) return err;

  *on_off = (reg1 & 0x4) != 0;
  *level = (uint8_t) (hcr & 0xF);

  return ESP_I2C_OK;
}

esp_i2c_err ICACHE_FLASH_ATTR
esp_sht21_heater_set(bool on_off, uint8_t level)
{
  esp_i2c_err err;
  uint8_t reg1 = 0;
  uint8_t hcr = 0;

  // Read both registers.
  err = register_get(ESP_SHT21_ADDRESS, ESP_SHT21_UR1_READ, &reg1);
  if (err != ESP_I2C_OK) return err;
  err = register_get(ESP_SHT21_ADDRESS, ESP_SHT21_HCR_READ, &hcr);
  if (err != ESP_I2C_OK) return err;

  // Write new values.
  reg1 = (uint8_t) (reg1 & 0xFB);
  reg1 = reg1 | (on_off << 2);
  err = register_set(ESP_SHT21_ADDRESS, ESP_SHT21_UR1_WRITE, reg1);
  if (err != ESP_I2C_OK) return err;

  hcr = (uint8_t) ((hcr & 0xF0) | (level & 0x0F));
  err = register_set(ESP_SHT21_ADDRESS, ESP_SHT21_HCR_WRITE, 0x0);
  if (err != ESP_I2C_OK) return err;

  return register_set(ESP_SHT21_ADDRESS, ESP_SHT21_HCR_WRITE, hcr);
}
