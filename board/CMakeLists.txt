cmake_minimum_required (VERSION 3.11)
project(beagle-config VERSION 0.1.0)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/SAtacker/ftxui
  GIT_TAG 760e4885babcac4d744926a48023bf88b0d4b207

)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# ------------------------------------------------------------------------------

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} 
  main.cpp 
  ui/panel/emmc/emmc_impl.cpp
  ui/panel/gpio/gpio_impl.cpp
  ui/panel/ics/ics_impl.cpp
  ui/panel/panel.hpp
  ui/panel/placeholder/placeholder_impl.cpp
  ui/panel/pru/pru_impl.cpp
  ui/ui.cpp
  ui/ui.hpp
  ui/focusable.hpp
)

# linking with beagle-panel
target_link_libraries(${PROJECT_NAME}
  PRIVATE ftxui::screen 
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
