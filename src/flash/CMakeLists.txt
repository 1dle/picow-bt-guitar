add_library(flash INTERFACE)
target_include_directories(flash INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(flash INTERFACE pico_stdlib hardware_flash hardware_sync)
target_sources(flash INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pico_flash.c)