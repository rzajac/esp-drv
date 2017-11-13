## SHT21 example.

Demonstrates how to:
- get serial number,
- get firmware version,
- get humidity and temperature.

## Flashing

```
$ cd build
$ cmake ..
$ make sht21_ex_flash
$ miniterm.py /dev/ttyUSB0 74880
```
