
include_directories(. TMC-API/)

add_executable(quarkcnc_quad_driver
        main.cpp
        TMC-API/tmc/helpers/CRC.c
        TMC-API/tmc/helpers/Functions.c
        TMC-API/tmc/ic/TMC2160/TMC2160.c
        ) 

# Need this to fix a bug in TMC libraries where a longer type is casted into a smaller type
# QuarkCnc-Quad-Driver-Firmware/src/TMC-API/tmc/ic/TMC2160/TMC2160.h:86:1: error: narrowing conversion of '3289120798' from 'unsigned int' to 'int32_t' {aka 'long int'} [-Wnarrowing]
set_source_files_properties(main.cpp PROPERTIES COMPILE_OPTIONS "-Wno-narrowing;")


# pull in common dependencies
target_link_libraries(quarkcnc_quad_driver pico_stdlib hardware_spi)

# enable usb output, disable uart output
pico_enable_stdio_usb(quarkcnc_quad_driver 1)
pico_enable_stdio_uart(quarkcnc_quad_driver 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(quarkcnc_quad_driver)



