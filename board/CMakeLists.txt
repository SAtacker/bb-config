add_executable(${PROJECT_NAME} 
                main.cpp 
                ui/ui.cpp
                )

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component # Not needed for this example.
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)