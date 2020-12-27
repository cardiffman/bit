cmake_minimum_required(VERSION 3.6.2)
#if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
project(type C CXX)

add_executable(type type.cpp typewindow.cpp)
#set_property(TARGET type PROPERTY CXX_STANDARD 20)
pkg_check_Modules(FT REQUIRED freetype2)
target_link_libraries(type xcbwindow ${FT_LIBRARIES} ${XCB_LIBRARIES} ${KS_LIBRARIES})
target_include_directories(type PUBLIC ${FT_INCLUDE_DIRS} ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS})
target_compile_options(type PUBLIC ${XCB_CFLAGS_OTHER})

endif()