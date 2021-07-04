cmake_minimum_required (VERSION 3.11)
project(beagle-config VERSION 0.1.0)

option(BEAGLE_CONFIG_SANITIZE "Use sanitize debug options" OFF)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG bd21cac2b6212945c1d682f6c3001696e9976e98
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(beagle-config 
  src/main.cpp 
  src/ui/focusable.hpp
  src/ui/panel/background_worker/background_worker_impl.cpp
  src/ui/panel/emmc/emmc_impl.cpp
  src/ui/panel/gpio/gpio_impl.cpp
  src/ui/panel/ics/ics_impl.cpp
  src/ui/panel/panel.hpp
  src/ui/panel/placeholder/placeholder_impl.cpp
  src/ui/panel/pru/pru_impl.cpp
  src/ui/panel/wifi/wifi_impl.cpp
  src/connman_handler/connman_handler
  src/ui/ui.cpp
  src/ui/ui.hpp
)

target_link_libraries(beagle-config
  PRIVATE ftxui::component
)

target_include_directories(beagle-config
  PRIVATE src/
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
