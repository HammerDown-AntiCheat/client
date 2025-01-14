cmake_minimum_required(VERSION 3.25)
include(cmake/vcpkg.cmake)



project(hdac_client)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(HD_AC_BUILD_AS_DLL "Will build HDAC as dll" ON)

vcpkg_install_if_not_found(boost-asio boost-beast boost-crc boost-thread openssl nlohmann-json zlib)

if (HD_AC_BUILD_AS_DLL)
    message(STATUS "HD:AC building into dll")
    add_library(hdac_client MODULE source/main.cpp)
    target_compile_definitions(hdac_client PRIVATE HDAC_AS_DL)
else ()
    add_executable(hdac_client source/main.cpp)
endif ()

add_subdirectory(extlibs)
add_subdirectory(source)

target_include_directories(hdac_client PRIVATE source)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(hdac_client PRIVATE /W4 /WX)
else()
    target_compile_options(hdac_client PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

find_package(boost_asio REQUIRED CONFIG)
find_package(boost_beast REQUIRED CONFIG)
find_package(boost_crc REQUIRED CONFIG)
find_package(boost_thread REQUIRED CONFIG)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

target_link_libraries(hdac_client PRIVATE xorstr cv-sdk lazy_importer Boost::asio Boost::beast Boost::crc Boost::thread nlohmann_json::nlohmann_json ZLIB::ZLIB)


target_compile_definitions(hdac_client PRIVATE WIN32_LEAN_AND_MEAN)