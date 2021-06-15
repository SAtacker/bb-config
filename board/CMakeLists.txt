cmake_minimum_required (VERSION 3.11)
project(beagle-config VERSION 0.1.0)

# Adding Subdirectory for beagle-panel
add_subdirectory(ui/)
# Adding Subdirectory for command handler
add_subdirectory(command_handler/)
add_subdirectory(cli_parser/)
add_subdirectory(xdg_app_handler/)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} 
                main.cpp 
                ui/ui.cpp
)

# linking with beagle-panel
target_link_libraries(${PROJECT_NAME}
  PRIVATE beagle-panel
  PRIVATE cli_parser
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)