cmake_minimum_required(VERSION 3.7)

add_library(xcbwindow STATIC XCBWindow.cpp)

set_property(TARGET xcbwindow PROPERTY CXX_STANDARD 14)

#find_package(PkgConfig REQUIRED)

#pkg_check_modules(KS REQUIRED xcb-keysyms)

#pkg_check_Modules(XCB REQUIRED xcb)

target_link_libraries(xcbwindow ${KS_LIBRARIES} ${XCB_LIBRARIES})
target_include_directories(xcbwindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS})
target_include_directories(xcbwindow INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )
