cmake_minimum_required (VERSION 3.11)
project(beagle-config VERSION 0.1.0)

# Adding Subdirectory for beagle-panel
add_subdirectory(ui/)

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
  PRIVATE beagle-panel
)
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
