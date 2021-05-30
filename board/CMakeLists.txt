# Adding Subdirectory for beagle-panel
add_subdirectory(ui/)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} 
                main.cpp 
                ui/ui.cpp
)

# linking with beagle-panel
target_link_libraries(${PROJECT_NAME}
  PRIVATE beagle-panel
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)