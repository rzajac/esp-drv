# Copyright 2017 Rafal Zajac <rzajac@gmail.com>.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.


project(esp_dht22 C)

find_package(esp_gpio REQUIRED)

add_library(esp_dht22 STATIC
    esp_dht22.c
    include/esp_dht22.h)

target_include_directories(esp_dht22 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${esp_gpio_INCLUDE_DIRS}
    ${ESP_USER_CONFIG_DIR})

target_link_libraries(esp_dht22 ${esp_gpio_LIBRARIES})

esp_gen_lib(esp_dht22)
