cmake_minimum_required(VERSION 3.7)
project(bit C CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(KS REQUIRED xcb-keysyms)

pkg_check_Modules(XCB REQUIRED xcb)

pkg_check_Modules(FT REQUIRED freetype2)

find_package(SDL2 REQUIRED)

pkg_check_Modules(SDL2_IMAGE REQUIRED SDL2_image)

pkg_check_Modules(SDL2_TTF REQUIRED SDL2_ttf)

find_package(GTK2 2.0 REQUIRED gtk)

pkg_check_Modules(GTKGL REQUIRED gtkglext-1.0)
if (GTKGL_FOUND)
message(STATUS ${GTKGL_INCLUDE_DIRS})
else()
message(STATUS GTKGL NOT FOUND)
endif()

pkg_check_Modules(PNG REQUIRED libpng)

pkg_check_Modules(JPEG REQUIRED libjpeg)

add_subdirectory(bit)

add_subdirectory(type)

add_subdirectory(sdl)

add_subdirectory(XCB)

add_subdirectory(gtk)

add_subdirectory(jsmn)

add_subdirectory(anim)
