cmake_minimum_required(VERSION 3.6.2)
project(bit C CXX)

set(CMAKE_CXX_STANDARD 14)

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

if (USE_JSONCPP)
add_subdirectory(jsoncpp)
endif()

if (NOT USE_JSMN AND NOT USE_JSONCPP)
message(STATUS "Using CMJSON")
add_subdirectory(cmjson)
endif()

add_subdirectory(anim)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_subdirectory(opengl)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
add_subdirectory(win64)
endif()

### This crap is needlessly difficult. 
### First make the lists and then use them with FILES.
### dont use wildcards directly: FILE NOT FOUND.
### dont use DIRECTORY/FILES_MATCHING: Can't specify DESTINATION.
FILE(GLOB GFX_FILES1 CONFIGURE_DEPENDS res/Lesson1/*.bmp)
FILE(GLOB GFX_FILES2 CONFIGURE_DEPENDS res/Lesson2/*.bmp)
FILE(GLOB GFX_FILES3 CONFIGURE_DEPENDS res/Lesson3/*.bmp res/Lesson3/*.png)
FILE(GLOB GFX_FILES4 CONFIGURE_DEPENDS res/Lesson4/*.bmp res/Lesson4/*.png)
FILE(GLOB GFX_FILES5 CONFIGURE_DEPENDS res/Lesson5/*.bmp res/Lesson5/*.png res/Lesson5/*.jpg)
FILE(GLOB GFX_FILES6 CONFIGURE_DEPENDS res/Lesson6/*.bmp res/Lesson6/*.png res/Lesson6/*.jpg res/Lesson6/*.ttf)
message(STATUS "The gfx files are [${GFX_FILES1} ${GFX_FILES2} ${GFX_FILES3} ${GFX_FILES4} ${GFX_FILES5} ${GFX_FILES6}")
#install(FILES ${GFX_FILES1} ${GFX_FILES2} ${GFX_FILES3} ${GFX_FILES4} ${GFX_FILES5} ${GFX_FILES6} DESTINATION res)
install(FILES ${GFX_FILES1} DESTINATION res/Lesson1)
install(FILES ${GFX_FILES2} DESTINATION res/Lesson2)
install(FILES ${GFX_FILES3} DESTINATION res/Lesson3)
install(FILES ${GFX_FILES4} DESTINATION res/Lesson4)
install(FILES ${GFX_FILES5} DESTINATION res/Lesson5)
install(FILES ${GFX_FILES6} DESTINATION res/Lesson6)

