cmake_minimum_required(VERSION 3.10)
project(my_msi_driver C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
    hidapi
    GIT_REPOSITORY https://github.com/libusb/hidapi.git
    GIT_TAG        hidapi-0.15.0
)
FetchContent_MakeAvailable(hidapi)

find_library(SENSORS_LIBRARY NAMES sensors)

add_executable(my_msi_driver 
    src/my_msi_driver.c
    src/coreliquid_hid.c src/coreliquid_hid.h
    src/coreliquid.c src/coreliquid.h
    src/coreliquid_s.c src/coreliquid_s.h
    src/sensors_wrap.c src/sensors_wrap.h
)

target_compile_definitions(my_msi_driver PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>)
add_compile_options($<$<CONFIG:Release>:-O3>)

target_link_libraries(my_msi_driver
    PRIVATE ${SENSORS_LIBRARY}
    PRIVATE hidapi::hidapi
)

install(TARGETS my_msi_driver DESTINATION /usr/local/bin)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/my_msi_driver.service"
        DESTINATION "/etc/systemd/system/")
