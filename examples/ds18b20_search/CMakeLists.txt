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


find_package(esp_sdo REQUIRED)
find_package(esp_ow REQUIRED)
find_package(esp_eb REQUIRED)
find_package(esp_util REQUIRED)

add_executable(ds18b20_search main.c ${ESP_USER_CONFIG})

target_include_directories(ds18b20_search PUBLIC
    ${ESP_USER_CONFIG_DIR}
    ${esp_sdo_INCLUDE_DIRS}
    ${esp_ow_INCLUDE_DIRS}
    ${esp_eb_INCLUDE_DIRS}
    ${esp_util_INCLUDE_DIRS})

target_link_libraries(ds18b20_search
    ${esp_sdo_LIBRARIES}
    ${esp_ow_LIBRARIES}
    ${esp_eb_LIBRARIES}
    ${esp_util_LIBRARIES}
    esp_ds18b20)

esp_gen_exec_targets(ds18b20_search)
