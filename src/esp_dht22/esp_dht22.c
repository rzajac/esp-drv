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


#include <esp_dht22.h>
#include <esp_gpio.h>
#include <mem.h>
#include <user_interface.h>

#define BUS_LOW(gpio_num) (GPIO_OUT_EN_S = (0x1 << (gpio_num)))
#define BUS_RELEASE(gpio_num) (GPIO_OUT_EN_C = (0x1 << (gpio_num)))
#define BUS_READ(gpio_num) ((GPIO_IN & (0x1 << (gpio_num))) != 0)


/**
 * Measure number of 10us slots the state was kept.
 *
 * The maximum wait time for GPIO to change state is 500us.
 *
 * @param gpio_num  The GPIO number.
 * @param exp_state The state.
 *
 * @return The number time slots.
 */
static uint8_t ICACHE_FLASH_ATTR
state_lenght(uint8_t gpio_num, bool exp_state)
{
  uint8_t slot = 0;

  while (BUS_READ(gpio_num) == exp_state) {
    if (slot++ > 50) break;
    os_delay_us(8);
  }

  return slot;
}

/**
 * Calculate temperature.
 *
 * @param data The pointer to 5 bytes read from the bus.
 *
 * @return Temperature in Celsius.
 */
static float calc_temp(const uint8_t *data)
{
  // Remove negative temp indicator bit.
  float temp = data[2] & 0x7F;
  temp *= 0x100;
  temp += data[3];
  temp /= 10;
  if ((data[2] & 0x80) > 0) temp *= -1;

  return temp;
}

void ICACHE_FLASH_ATTR
esp_dht22_init(uint8_t gpio_num)
{
  esp_gpio_setup(gpio_num, GPIO_MODE_INPUT_PULLUP);
}

esp_dht22_dev *ICACHE_FLASH_ATTR
esp_dht22_new_dev(uint8_t gpio_num)
{
  esp_dht22_dev *dev = os_zalloc(sizeof(esp_dht22_dev));
  if (dev == NULL) return NULL;

  dev->gpio_num = gpio_num;

  return dev;
}

esp_dht22_err ICACHE_FLASH_ATTR
esp_dht22_get(esp_dht22_dev *device)
{
  uint16_t cnt = 0;
  // The humidity, temperature and parity data.
  uint8_t *data;
  // Current byte index in data (0-4).
  int8_t data_byte_idx = 0;
  // Current mask for data byte. We start with MSB.
  uint8_t data_byte_mask = 0x80;
  // The bit from the bus.
  bool bit;
  // Previous bit.
  bool prev_bit = false;
  // Number of high states.
  uint8_t high_cnt = 0;

  if (device == NULL) return ESP_DHT22_ERR_DEV_NULL;

  data = os_zalloc(5);
  if (data == NULL) return ESP_DHT22_ERR_MEM;

  // Emmit start signal.
  BUS_LOW(device->gpio_num);
  os_delay_us(820);

  // End start signal.
  BUS_RELEASE(device->gpio_num);
  os_delay_us(25);

  // Entering time critical code.
  ETS_GPIO_INTR_DISABLE();

  // Device responds with 80us low followed by 80us high.
  cnt = state_lenght(device->gpio_num, 0);
  if (cnt < 8 || cnt > 10) {
    ETS_GPIO_INTR_ENABLE();
    return ESP_DHT22_ERR_BAD_RESP_SIGNAL;
  }

  cnt = state_lenght(device->gpio_num, 1);
  if (cnt < 8 || cnt > 10) {
    ETS_GPIO_INTR_ENABLE();
    return ESP_DHT22_ERR_BAD_RESP_SIGNAL;
  }

  // Sample data bus for 40 bits.
  //
  // Device transmits
  //  - 0 as 50us low followed by 25us high (total 75us).
  //  - 1 as 50us low followed by 70us high (total 120us).
  //
  // Data is transmitted MSB first.
  //
  // This means 40 bits transfer takes between 3000us and 4800us.
  // The code below samples the bus no more then 4800us (10us * 480).
  // During that time we count falling edges and count number of
  // high states seen since last falling edge. The number of seen
  // high states tells us if this was o or 1.
  do {
    bit = BUS_READ(device->gpio_num);
    if (bit) high_cnt++;
    if (bit == false && prev_bit == true) {
      if (high_cnt >= 6) data[data_byte_idx] |= data_byte_mask;
      high_cnt = 0;
      data_byte_mask >>= 1;
      if (data_byte_mask == 0) {
        data_byte_idx++;
        data_byte_mask = 0x80;
      }
    }
    prev_bit = bit;
    cnt++;
    os_delay_us(10);
  } while (data_byte_idx < 5 && cnt <= 480);

  ETS_GPIO_INTR_ENABLE();

  device->last_measure = system_get_time();

  if (((uint8_t) (data[0] + data[1] + data[2] + data[3])) != data[4]) {
    return ESP_DHT22_ERR_PARITY;
  }

  device->temp = calc_temp(data);
  device->hum = data[0] * 0x100;
  device->hum += data[1];
  device->hum /= 10;

  //os_free(data);
  return ESP_DHT22_OK;
}

