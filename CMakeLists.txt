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

target_link_libraries(sdlbit ${SDL2_LIBRARIES})
target_include_directories(sdlbit PUBLIC ${SDL2_INCLUDE_DIRS})

add_executable(rp respathtest.cpp)
target_link_libraries(rp ${SDL2_LIBRARIES})
target_include_directories(rp PUBLIC ${SDL2_INCLUDE_DIRS})

if (NOT ${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY "${PROJECT_SOURCE_DIR}/res" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()