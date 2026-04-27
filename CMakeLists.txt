# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

set(EXTRA_COMPONENT_DIRS spiffs_usr)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(WiFi_provisioning)

spiffs_create_partition_image(spiffs spiffs_usr FLASH_IN_PROJECT)

