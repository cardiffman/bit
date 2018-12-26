cmake_minimum_required(VERSION 3.7)
project(bit C CXX)

add_executable(bit bit.cpp jsoncpp.cpp bitwindow.cpp XCBWindow.cpp)

set_property(TARGET bit PROPERTY CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)

pkg_check_modules(KS REQUIRED xcb-keysyms)

pkg_check_Modules(XCB REQUIRED xcb)

target_link_libraries(bit ${KS_LIBRARIES} ${XCB_LIBRARIES})
target_include_directories(bit PUBLIC ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS})
target_compile_options(bit PUBLIC ${SDL2_CFLAGS_OTHER} ${XCB_CFLAGS_OTHER})

add_executable(type type.cpp typewindow.cpp XCBWindow.cpp)
set_property(TARGET type PROPERTY CXX_STANDARD 14)
pkg_check_Modules(FT REQUIRED freetype2)
target_link_libraries(type ${FT_LIBRARIES} ${XCB_LIBRARIES} ${KS_LIBRARIES})
target_include_directories(type PUBLIC ${FT_INCLUDE_DIRS} ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS})
target_compile_options(type PUBLIC ${XCB_CFLAGS_OTHER})

add_executable(sdlbit sdlbit.cpp sdlaux.cpp)
set_property(TARGET bit PROPERTY CXX_STANDARD 14)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
#find_package(SDL2_image REQUIRED)
#include_directories(${SDL2_IMAGE_INCLUDE_DIR})
pkg_check_Modules(SDL2_IMAGE REQUIRED SDL2_image)
#find_package(SDL2_ttf REQUIRED)
#include_directories(${SDL2_TTF_INCLUDE_DIR})
pkg_check_Modules(SDL2_TTF REQUIRED SDL2_ttf)

target_link_libraries(sdlbit ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})
target_include_directories(sdlbit PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

add_executable(rp respathtest.cpp)
target_link_libraries(rp ${SDL2_LIBRARIES})
target_include_directories(rp PUBLIC ${SDL2_INCLUDE_DIRS})

add_executable(layerguy layerguy.cpp)
target_link_libraries(layerguy ${SDL2_LIBRARIES}  ${SDL2_IMAGE_LIBRARIES})
target_include_directories(rp PUBLIC ${SDL2_INCLUDE_DIRS})

if (NOT ${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY "${PROJECT_SOURCE_DIR}/res" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

find_package(GTK2 2.0 REQUIRED gtk)
add_executable(gtkbit gtkbit.cpp)
target_link_libraries(gtkbit ${GTK2_LIBRARIES})
target_include_directories(gtkbit PUBLIC ${GTK2_INCLUDE_DIRS})
