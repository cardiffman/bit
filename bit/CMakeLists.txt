cmake_minimum_required(VERSION 3.6.2)
#if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
project(bit C CXX)
set(CMAKE_CXX_STANDARD 11)

pkg_check_Modules(XCB_SHM REQUIRED xcb-shm)


add_executable(bit
	bit.cpp
	#parse_json_dom.cpp
	bitwindow.cpp
	)

message(STATUS "XCB_LIBRARIES ${XCB_LIBRARIES}")
message(STATUS "xcb_LIBRARIES ${xcb_LIBRARIES}")
message(STATUS "XCBWINDOW_LIBRARIES ${XCBWINDOW_LIBRARIES}")
message(STATUS "xcbwindow_LIBRARIES ${xcbwindow_LIBRARIES}")
message(STATUS "PNG_LIBRARIES ${PNG_LIBRARIES}")
message(STATUS "JPEG_LIBRARIES ${JPEG_LIBRARIES}")

set(BIT_INCLUDE_DIRS ../anim ../XCB ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS} ${FT_INCLUDE_DIRS})
set(BIT_LINK_DIRS anim xcbwindow ${KS_LIBRARIES} ${XCB_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${FT_LIBRARIES})
if (USE_JSMN)
	#list(append BIT_INCLUDE_DIRS ../jsmn)
	list(APPEND BIT_LINK_LIBS jsmn)
	target_compile_definitions(bit PRIVATE USE_JSMN=1)
endif()

if (NOT USE_JSMN AND NOT USE_JSONCPP)
	message(STATUS "Adding cmjson library")
	list(APPEND BIT_INCLUDE_DIRS ../cmjson)
	list(APPEND BIT_LINK_LIBS cmjson)
endif()
target_link_libraries(bit ${BIT_LINK_DIRS})
target_include_directories(bit PUBLIC ${BIT_INCLUDE_DIRS})
target_compile_options(bit PUBLIC ${SDL2_CFLAGS_OTHER} ${XCB_CFLAGS_OTHER})

add_executable(shmtrial
	shmtrial.cpp
	shmwindow.cpp
	)
target_include_directories(shmtrial PUBLIC ${BIT_INCLUDE_DIRS})
target_link_libraries(shmtrial ${BIT_LINK_DIRS} ${XCB_SHM_LIBRARIES})

if (NOT USE_JSMN AND NOT USE_JSONCPP)
add_executable(jstest jstest.cpp)
target_include_directories(jstest PRIVATE ${BIT_INCLUDE_DIRS})
target_link_libraries(jstest cmjson)
endif()

endif()