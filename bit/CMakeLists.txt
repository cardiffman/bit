cmake_minimum_required(VERSION 3.7)
project(bit C CXX)

add_executable(bit bit.cpp parse_json_utils.cpp parse_json_dom.cpp bitwindow.cpp)

set_property(TARGET bit PROPERTY CXX_STANDARD 14)

message(STATUS "XCB_LIBRARIES ${XCB_LIBRARIES}")
message(STATUS "xcb_LIBRARIES ${xcb_LIBRARIES}")
message(STATUS "XCBWINDOW_LIBRARIES ${XCBWINDOW_LIBRARIES}")
message(STATUS "xcbwindow_LIBRARIES ${xcbwindow_LIBRARIES}")
message(STATUS "PNG_LIBRARIES ${PNG_LIBRARIES}")
message(STATUS "JPEG_LIBRARIES ${JPEG_LIBRARIES}")

target_link_libraries(bit jsmn xcbwindow ${KS_LIBRARIES} ${XCB_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
target_include_directories(bit PUBLIC ../jsmn ../XCB ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS})
target_compile_options(bit PUBLIC ${SDL2_CFLAGS_OTHER} ${XCB_CFLAGS_OTHER})

