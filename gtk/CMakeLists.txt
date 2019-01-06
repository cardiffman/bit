cmake_minimum_required(VERSION 3.7)

add_executable(gtkbit gtkbit.cpp)
target_link_libraries(gtkbit ${GTK2_LIBRARIES})
target_include_directories(gtkbit PUBLIC ${GTK2_INCLUDE_DIRS})

add_executable(bitgtk bitgtk.cpp)
target_link_libraries(bitgtk anim pthread ${GTK2_LIBRARIES} ${GTKGL_LIBRARIES})
target_include_directories(bitgtk PUBLIC ../anim ${GTK2_INCLUDE_DIRS} ${GTKGL_INCLUDE_DIRS})
