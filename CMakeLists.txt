cmake_minimum_required (VERSION 3.11)
project(bb-config VERSION 0.1)

option(BEAGLE_CONFIG_SANITIZE "Use sanitize debug options" OFF)
option(ARMHF_DEB "Debian Package for armhf" OFF)
option(VERSION_FILE "Create a version file" OFF)
option(BEAGLE_CONFIG_WITH_FETCH_FTXUI "Use FetchContent to fetch FTXUI" ON)

if(BEAGLE_CONFIG_WITH_FETCH_FTXUI)

	include(FetchContent)
	set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

	FetchContent_Declare(ftxui
		GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
		GIT_TAG v3.0.0
	)

	FetchContent_GetProperties(ftxui)
	if(NOT ftxui_POPULATED)
		FetchContent_Populate(ftxui)
		add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
	endif()

else()

	find_package(ftxui 3.0.0 REQUIRED)

endif()

find_package(PkgConfig)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0 libnm)

add_executable(${PROJECT_NAME} 
  src/main.cpp 
  src/ui/panel/emmc/emmc_impl.cpp
  src/ui/panel/gpio/gpio_impl.cpp
  src/ui/panel/ics/ics_impl.cpp
  src/ui/panel/led/led_impl.cpp
  src/ui/panel/about/about_impl.cpp
  src/ui/panel/adc/adc_impl.cpp
  src/ui/panel/dac/dac_impl.cpp
  src/ui/panel/uEnv/uEnv_impl.cpp
  src/ui/panel/panel.hpp
  src/ui/panel/placeholder/placeholder_impl.cpp
  src/ui/panel/pru/pru_impl.cpp
  src/ui/panel/pinmux/pinmux_impl.cpp
  src/ui/panel/wifi/wifi_impl.cpp
  src/ui/panel/passwd/passwd.cpp
  src/ui/panel/ssh/ssh.cpp
  src/ui/panel/service/service_impl.cpp
  src/ui/ui.cpp
  src/ui/ui.hpp
  src/utils.hpp
  src/utils.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE ftxui::component
  PRIVATE stdc++fs
  PkgConfig::deps
)

target_include_directories(${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src/
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_compile_options(${PROJECT_NAME}
  PRIVATE "-Wall"
  PRIVATE "-Wextra"
  PRIVATE "-pedantic"
  PRIVATE "-Werror"
  PRIVATE "-Wdeprecated"
  PRIVATE "-Wshadow"
)

# Useful for debugging and find 
if (BEAGLE_CONFIG_SANITIZE)
  target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address,leak,undefined)
  target_link_libraries(${PROJECT_NAME} PRIVATE -fsanitize=address,leak,undefined)
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

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/sbin)

if(GIT_VERSION)
  set(git_version ${GIT_VERSION})
endif()

if(ARMHF_DEB)
  set(CMAKE_CXX_COMPILER_TARGET arm-linux-gnueabihf)
  set(CMAKE_C_COMPILER_TARGET arm-linux-gnueabihf)
  if(ARCHITECTURE)
  else()
    EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE)
  endif()
  set(CPACK_GENERATOR "DEB")
  set(CPACK_SOURCE_GENERATOR "TGZ")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Shreyas Atre")
  set(CPACK_PACKAGE_CONTACT "Shreyas Atre shreyasatre16@gmail.com")
  set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}-${git_version}")
  set(CPACK_DEBIAN_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCHITECTURE})
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${git_version}-${ARCHITECTURE}")
  set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Configure your beaglebone device")
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "standard")
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/SAtacker/bb-config")
  set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
  set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/README.md")
  set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_SOURCE_DIR}/build/;${CMAKE_SOURCE_DIR}/.git/") 
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/script/postinst")

  include(CPack)
  
endif(ARMHF_DEB)
