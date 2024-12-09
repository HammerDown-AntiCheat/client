cmake_minimum_required(VERSION 3.25)
include(cmake/vcpkg.cmake)


project(hdac_client)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
option(BUILD_AS_DLL "Will build HDAC as dll" OFF)

vcpkg_install_if_not_found(boost-asio boost-beast openssl)

if (BUILD_AS_DLL)
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

target_link_libraries(hdac_client PRIVATE xorstr cv-sdk lazy_importer Boost::asio Boost::beast)


target_compile_definitions(hdac_client PRIVATE WIN32_LEAN_AND_MEAN)