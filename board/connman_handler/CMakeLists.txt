add_library(connman_handler STATIC connman_handler)

set(TEST ON)

if(TEST)
    message("Test for connman_handler...")
    add_executable(test_connman test_connman.cpp)
    target_link_libraries(test_connman PRIVATE connman_handler pthread)
endif()