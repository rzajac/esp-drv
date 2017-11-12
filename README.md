## ESP8266 drivers.

The collection of device drivers for ESP8266.

- [DS18B20](src/esp_ds18b20) OneWire temperature sensor.
- [DHT22 (AM2302)](src/esp_dht22) temperature and humidity sensor.
- [SHT21 (Si7021)](src/esp_sht21) temperature and humidity sensor.

## Build environment.

This library is part of my build system for ESP8266 based on CMake.
To compile / flash examples you will have to have the ESP development 
environment setup as described at https://github.com/rzajac/esp-dev-env.

## Development environment installation.

There are two ways to install device drivers in the development environment:

```
$ wget -O - https://raw.githubusercontent.com/rzajac/esp-drv/master/install.sh | bash
```

or if you already cloned this repository you can do:

```
$ cd build
$ cmake ..
$ make
$ make install
```

## Examples.

- [DHT22 get temperature and humidity](examples/dht22_temp_hum)
- [DS18B20 get temperature](examples/ds18b20_temp)
- [Search for DS18B20](examples/ds18b20_search)

# Dependencies.

This library depends on:

- https://github.com/rzajac/esp-prot

to install dependency run:

```
$ wget -O - https://raw.githubusercontent.com/rzajac/esp-prot/master/install.sh | bash
```

## License.

[Apache License Version 2.0](LICENSE) unless stated otherwise.
