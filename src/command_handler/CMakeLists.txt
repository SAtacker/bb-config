add_library(command_handler STATIC command_handler)

target_link_libraries(command_handler
 PUBLIC xdg_handler
 PUBLIC stdc++fs
)

set_target_properties(command_handler PROPERTIES CXX_STANDARD 17)
