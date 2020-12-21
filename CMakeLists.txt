cmake_minimum_required(VERSION 3.6.2)
project(bit C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)

MESSAGE(STATUS "CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")

#if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
if (NOT CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
pkg_check_modules(KS REQUIRED xcb-keysyms)
endif()

#if (false)
pkg_check_Modules(XCB REQUIRED xcb)

find_package(SDL2 REQUIRED)

pkg_check_Modules(SDL2_IMAGE REQUIRED SDL2_image)

pkg_check_Modules(SDL2_TTF REQUIRED SDL2_ttf)

pkg_check_Modules(FT REQUIRED freetype2)


pkg_check_Modules(PNG REQUIRED libpng)

pkg_check_Modules(JPEG REQUIRED libjpeg)

add_subdirectory(bit)

add_subdirectory(type)

add_subdirectory(sdl)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
message(STATUS " Doing XCB because system name ${CMAKE_SYSTEM_NAME}")
add_subdirectory(XCB)
endif()

add_subdirectory(gtk)

if (USE_JSMN)
add_subdirectory(jsmn)
endif()

add_subdirectory(anim)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_subdirectory(opengl)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
add_subdirectory(win64)
endif()

if (USE_JSONCPP)
add_subdirectory(jsoncpp)
endif()

