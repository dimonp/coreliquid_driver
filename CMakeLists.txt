cmake_minimum_required(VERSION 3.10)

if(NOT PROJECT_VERSION)
  set(PROJECT_VERSION "0.1.0")
endif()

option(USE_SYSTEMD_BUS "Build with systemd sd-bus support" ON)

project(my_msi_coreliquid_driver
    VERSION ${PROJECT_VERSION}
    LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

set(PROJECT_SOURCES
    src/my_msi_driver.c
    src/logger.c src/logger.h
    src/coreliquid_hid.c src/coreliquid_hid.h
    src/coreliquid.c src/coreliquid.h
    src/coreliquid_s.c src/coreliquid_s.h
    src/sensors_wrap.c src/sensors_wrap.h
)


find_package(hidapi REQUIRED QUIET)
if (NOT hidapi_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        hidapi
        GIT_REPOSITORY https://github.com/libusb/hidapi.git
        GIT_TAG        hidapi-0.15.0
    )
    FetchContent_MakeAvailable(hidapi)
endif()

find_library(SENSORS_LIBRARY NAMES sensors)

if(USE_SYSTEMD_BUS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)

    if(SYSTEMD_FOUND)
        message(STATUS "libsystemd found, enabling sd-bus support")
        add_definitions(-DHAVE_SYSTEMD_BUS)
        list(APPEND PROJECT_SOURCES src/sensors_dbus.c src/sensors_dbus.h)
    else()
        message(WARNING "libsystemd not found, sd-bus support disabled")
        set(USE_SYSTEMD_BUS OFF)
    endif()
endif()

add_executable(my_msi_coreliquid_driver ${PROJECT_SOURCES})

target_compile_definitions(my_msi_coreliquid_driver PRIVATE $<$<CONFIG:Debug>:_DEBUG=1>)
target_compile_options(my_msi_coreliquid_driver PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_options(my_msi_coreliquid_driver PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
target_compile_options(my_msi_coreliquid_driver PRIVATE $<$<CONFIG:Release>:-O3>)
target_link_options(my_msi_coreliquid_driver PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)

target_link_libraries(my_msi_coreliquid_driver
    PRIVATE ${SENSORS_LIBRARY}
    PRIVATE hidapi::hidapi)

if(USE_SYSTEMD_BUS AND SYSTEMD_FOUND)
    target_link_libraries(my_msi_coreliquid_driver PRIVATE PRIVATE PkgConfig::SYSTEMD)
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/service/my_msi_coreliquid_driver@.service.in"
    "${CMAKE_CURRENT_BINARY_DIR}/my_msi_coreliquid_driver@.service"
    @ONLY
)

install(TARGETS my_msi_coreliquid_driver DESTINATION bin)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/my_msi_coreliquid_driver@.service"
        DESTINATION "/usr/lib/systemd/system/")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/service/my_msi_coreliquid_driver_sleep.sh"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        DESTINATION "/usr/lib/systemd/system-sleep/")

