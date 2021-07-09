# Designed to be included from the app's CMakeLists.txt file
cmake_minimum_required(VERSION 3.5)

# These paths are used in other components
set(BOARD_PATH ${BOARD_NAME} CACHE INTERNAL "")
set(LED_DRIVER_PATH "vled" CACHE INTERNAL "")
set(BUTTON_DRIVER_PATH "hollow" CACHE INTERNAL "")
