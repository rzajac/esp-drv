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


#include <esp_ds18b20.h>
#include <esp_tim.h>
#include <esp_eb.h>
#include <mem.h>


/**
 * Decode temperature.
 *
 * @param sp The address for first scrachpad byte.
 *
 * @return The temperature.
 */
float ICACHE_FLASH_ATTR
decode_temp(const uint8_t *sp)
{
  bool minus;
  uint8_t integer = 0;
  float decimal = 0;

  // Get only bits responsible for temperature and sign.
  int16_t temp = sp[0] | (sp[1] << 8);
  minus = (temp & 0x8000) > 0;

  // Detect negative number.
  if (minus) {
    temp = (int16_t) (~temp + 1); // 2's complement.
  }

  // Remove fraction.
  integer = (uint8_t) (temp >> 4);
  // take only 3 LSB form MSB.
  integer |= ((temp >> 8) & 0x07) << 4;

  // Calculate fraction accounting for resolution.
  switch ((sp[4] & 0x60) >> 5) {
    case ESP_DS18B20_RES_12:
      decimal = (temp & 0x0F) * ((float) ESP_DS18B20_STEP_12);
      break;

    case ESP_DS18B20_RES_11:
      decimal = ((temp >> 1) & 0x07) * ((float) ESP_DS18B20_STEP_11);
      break;

    case ESP_DS18B20_RES_10:
      decimal = ((temp >> 2) & 0x03) * ((float) ESP_DS18B20_STEP_10);
      break;

    case ESP_DS18B20_RES_9:
      decimal = ((temp >> 3) & 0x01) * ((float) ESP_DS18B20_STEP_9);
      break;

    default:
      decimal = 0;
  }

  decimal = integer + decimal;
  if (minus) decimal = 0 - decimal;

  return decimal;
}

esp_ow_err ICACHE_FLASH_ATTR
esp_d18b20_read_sp(esp_ow_device *device)
{
  uint8_t idx;
  uint8_t crc = 0;

  esp_ds18b20_st *st = device->custom;

  if (esp_ow_reset(device->gpio_num) == false) {
    st->retries = -1;
    return ESP_OW_ERR_NO_DEV;
  }

  esp_ow_match_dev(device);
  esp_ow_write(device->gpio_num, ESP_DS18B20_CMD_READ_SP);
  esp_ow_read_bytes(device->gpio_num, st->sp, 9);

  for (idx = 0; idx < 9; idx++) {
    crc = esp_ow_crc8(crc, st->sp[idx]);
  }

  if (crc != 0) {
    memset(st->sp, 0, 9);
    return ESP_OW_ERR_BAD_CRC;
  }

  return ESP_OW_OK;
}

esp_ow_err ICACHE_FLASH_ATTR
esp_ds18b20_write_sp(esp_ow_device *device)
{
  uint8_t *start;
  esp_ds18b20_st *st = device->custom;

  if (esp_ow_reset(device->gpio_num) == false) {
    return ESP_OW_ERR_NO_DEV;
  }

  esp_ow_match_dev(device);
  esp_ow_write(device->gpio_num, ESP_DS18B20_CMD_WRITE_SP);

  // We transmit only 3 bytes (Th, Tl, cfg).
  start = &st->sp[2];

  esp_ow_write_bytes(device->gpio_num, start, 3);

  return ESP_OW_OK;
}

esp_ow_err ICACHE_FLASH_ATTR
read_temp(esp_ow_device *device)
{
  esp_ds18b20_st *st = device->custom;
  esp_ow_err err = esp_d18b20_read_sp(device);

  st->retries = -1;
  if (err != ESP_OW_OK) return err;

  st->last_temp = decode_temp(st->sp);

  return ESP_OW_OK;
}

static void ICACHE_FLASH_ATTR
start_conversion(void *arg)
{
  esp_tim_timer *timer = arg;

  bool done = false;
  esp_ow_device *dev = timer->payload;
  esp_ds18b20_st *st = dev->custom;
  uint32_t sample_count = 200;

  st->retries++;

  do {
    if (esp_ow_read_bit(dev->gpio_num)) {
      done = true;
      break;
    }
    os_delay_us(5);
    sample_count--;
  } while (sample_count > 0);

  if (done == true) {
    if (read_temp(dev) != ESP_OW_OK) {
      esp_eb_trigger(ESP_DS18B20_EV_TEMP_ERROR, dev);
    } else {
      esp_eb_trigger(ESP_DS18B20_EV_TEMP_READY, dev);
    }
  } else {
    // The worst case from datasheet is 750ms when
    // 12 bits of resolution is set.
    // Each call to this function takes about 200*5us (1ms).
    // If temperature is not available we call ourselves again
    // after 10ms. That gives 11ms per call. 68 calls * 11ms = 748ms.
    // This condition basically makes sure we are not
    // calling ourselves forever.
    if (st->retries > 68) {
      st->retries = -1;
      esp_eb_trigger(ESP_DS18B20_EV_TEMP_ERROR, dev);
    } else {
      // Try again.
      esp_tim_continue(timer);
    }
  }
}

bool ICACHE_FLASH_ATTR
esp_ds18b20_init(uint8_t gpio_num)
{
  esp_ow_init(gpio_num);

  return true;
}

esp_ow_err ICACHE_FLASH_ATTR
esp_ds18b20_search(uint8_t gpio_num, bool in_alert, esp_ow_device **list)
{
  esp_ow_device *curr;

  esp_ow_cmd cmd = in_alert ? ESP_OW_CMD_SEARCH_ROM_ALERT : ESP_OW_CMD_SEARCH_ROM;
  esp_ow_err err = esp_ow_search_family(gpio_num, cmd, ESP_DS18B20_FAMILY_CODE, list);
  if (err != ESP_OW_OK) return err;

  curr = *list;
  while (curr) {
    curr->custom = os_zalloc(sizeof(esp_ds18b20_st));
    ((esp_ds18b20_st *) curr->custom)->last_temp = ESP_DS18B20_TEMP_ERR;
    ((esp_ds18b20_st *) curr->custom)->retries = -1;
    curr = curr->next;
  }

  return err;
}

esp_ow_device *ICACHE_FLASH_ATTR
esp_ds18b20_new_dev(uint8_t *rom)
{
  esp_ow_device *device = os_zalloc(sizeof(esp_ow_device));
  esp_ds18b20_st *st = os_zalloc(sizeof(esp_ds18b20_st));

  os_memcpy(device->rom, rom, 8);
  st->last_temp = ESP_DS18B20_TEMP_ERR;
  st->retries = -1;
  device->custom = st;

  return device;
}


esp_ow_device *ICACHE_FLASH_ATTR
esp_ds18b20_get(uint8_t gpio_num)
{
  return esp_ow_read_rom_dev(gpio_num);
}

esp_ow_err ICACHE_FLASH_ATTR
esp_ds18b20_get_alarm(esp_ow_device *dev, int8_t *low, int8_t *high)
{
  esp_ds18b20_st *st = dev->custom;
  esp_ow_err err = esp_d18b20_read_sp(dev);
  if (err != ESP_OW_OK) return err;

  *low = st->sp[3];
  *high = st->sp[2];

  return ESP_OW_OK;
}

esp_ow_err ICACHE_FLASH_ATTR
esp_ds18b20_set_alarm(esp_ow_device *dev, int8_t low, int8_t high)
{
  esp_ds18b20_st *st = dev->custom;
  esp_ow_err err = esp_d18b20_read_sp(dev);
  if (err != ESP_OW_OK) return err;

  st->sp[3] = low;
  st->sp[2] = high;

  return esp_ds18b20_write_sp(dev);
}

void ICACHE_FLASH_ATTR
esp_ds18b20_free_list(esp_ow_device *list)
{
  esp_ow_free_device_list(list, true);
}

esp_ds18b20_err ICACHE_FLASH_ATTR
esp_ds18b20_convert(esp_ow_device *device)
{
  esp_ds18b20_st *st = device->custom;

  // We are already waiting for the conversion.
  if (st->retries >= 0) return ESP_DS18B20_ERR_CONV_IN_PROG;

  if (esp_ow_reset(device->gpio_num) == false) return ESP_DS18B20_NO_DEV;

  // Send conversion command.
  esp_ow_match_dev(device);
  esp_ow_write(device->gpio_num, ESP_DS18B20_CMD_CONVERT);

  if (esp_tim_start(start_conversion, device)) {
    st->retries = 0;
  }

  return ESP_DS18B20_OK;
}

bool ICACHE_FLASH_ATTR
esp_ds18b20_has_parasite(uint8_t gpio_num)
{
  bool has_parasite;

  if (esp_ow_reset(gpio_num) == false) {
    return false; // No devices.
  }

  esp_ow_write(gpio_num, ESP_OW_CMD_SKIP_ROM);
  esp_ow_write(gpio_num, ESP_DS18B20_CMD_READ_PWR);
  has_parasite = !esp_ow_read_bit(gpio_num);

  return has_parasite;
}
