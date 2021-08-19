cmake_minimum_required (VERSION 3.11)
project(beagle-config VERSION 0.1.0)

option(BEAGLE_CONFIG_SANITIZE "Use sanitize debug options" OFF)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG 69b0c9e53e523ac43a303964fc9c5bc0da7d5d61
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(beagle-config 
  src/main.cpp 
  src/ui/panel/emmc/emmc_impl.cpp
  src/ui/panel/gpio/gpio_impl.cpp
  src/ui/panel/ics/ics_impl.cpp
  src/ui/panel/led/led_impl.cpp
  src/ui/panel/about/about_impl.cpp
  src/ui/panel/panel.hpp
  src/ui/panel/placeholder/placeholder_impl.cpp
  src/ui/panel/pru/pru_impl.cpp
  src/ui/panel/wifi/wifi_impl.cpp
  src/ui/panel/passwd/passwd.cpp
  src/ui/panel/display/display.cpp
  src/ui/panel/ssh/ssh.cpp
  src/ui/ui.cpp
  src/ui/ui.hpp
  src/utils.hpp
  src/utils.cpp
)

target_link_libraries(beagle-config
  PRIVATE ftxui::component
  PRIVATE stdc++fs
)

target_include_directories(beagle-config
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src/
)

set_property(TARGET beagle-config PROPERTY CXX_STANDARD 17)

target_compile_options(beagle-config
  PRIVATE "-Wall"
  PRIVATE "-Wextra"
  PRIVATE "-pedantic"
  PRIVATE "-Werror"
  PRIVATE "-Wdeprecated"
  PRIVATE "-Wshadow"
)

# Useful for debugging and find 
if (BEAGLE_CONFIG_SANITIZE)
  target_compile_options(beagle-config PRIVATE -fsanitize=address,leak,undefined)
  target_link_libraries(beagle-config PRIVATE -fsanitize=address,leak,undefined)
endif()

set(git_version "unknown")
set(git_hash "unknown")
find_package(Git QUIET)
if (Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE git_version
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE git_hash
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/environment.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/environment.hpp
)
