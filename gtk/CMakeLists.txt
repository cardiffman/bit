cmake_minimum_required(VERSION 3.6.2)
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")

find_package(GTK2 2.0 REQUIRED gtk)
if (GTK2_FOUND)

pkg_check_Modules(GTKGL REQUIRED gtkglext-1.0)
if (GTKGL_FOUND)
message(STATUS ${GTKGL_INCLUDE_DIRS})

add_executable(bitgtk bitgtk.cpp)
target_link_libraries(bitgtk anim pthread ${GTK2_LIBRARIES} ${GTKGL_LIBRARIES} ${FT_LIBRARIES})
target_include_directories(bitgtk PUBLIC ../anim ${GTK2_INCLUDE_DIRS} ${GTKGL_INCLUDE_DIRS})
if (USE_JSMN)
target_compile_definitions(bitgtk PRIVATE USE_JSMN=1)
elseif(USE_JSONCPP)
target_compile_definitions(bitgtk PRIVATE USE_JSONCPP=1)
else()
endif()
install(TARGETS bitgtk DESTINATION .)

else()
message(STATUS GTKGL NOT FOUND)
endif()

else()
message(STATUS GTK2 NOT FOUND)
endif()

endif()