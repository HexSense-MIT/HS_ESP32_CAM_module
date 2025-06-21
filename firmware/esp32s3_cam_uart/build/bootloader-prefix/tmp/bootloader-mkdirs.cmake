# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/liufangzheng/Downloads/software/IDF_IDE/esp_idf/esp-idf-v5.4.1/components/bootloader/subproject")
  file(MAKE_DIRECTORY "/Users/liufangzheng/Downloads/software/IDF_IDE/esp_idf/esp-idf-v5.4.1/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "/Users/liufangzheng/Documents/Lab/github/HexSense_MIT/HS_ESP32_CAM_module/firmware/esp32s3_cam_uart/build/bootloader"
  "/Users/liufangzheng/Documents/Lab/github/HexSense_MIT/HS_ESP32_CAM_module/firmware/esp32s3_cam_uart/build/bootloader-prefix"
  "/Users/liufangzheng/Documents/Lab/github/HexSense_MIT/HS_ESP32_CAM_module/firmware/esp32s3_cam_uart/build/bootloader-prefix/tmp"
  "/Users/liufangzheng/Documents/Lab/github/HexSense_MIT/HS_ESP32_CAM_module/firmware/esp32s3_cam_uart/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/liufangzheng/Documents/Lab/github/HexSense_MIT/HS_ESP32_CAM_module/firmware/esp32s3_cam_uart/build/bootloader-prefix/src"
  "/Users/liufangzheng/Documents/Lab/github/HexSense_MIT/HS_ESP32_CAM_module/firmware/esp32s3_cam_uart/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/liufangzheng/Documents/Lab/github/HexSense_MIT/HS_ESP32_CAM_module/firmware/esp32s3_cam_uart/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/liufangzheng/Documents/Lab/github/HexSense_MIT/HS_ESP32_CAM_module/firmware/esp32s3_cam_uart/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
