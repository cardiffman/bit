cmake_minimum_required(VERSION 3.7)

add_executable(gtkbit gtkbit.cpp)
target_link_libraries(gtkbit ${GTK2_LIBRARIES})
target_include_directories(gtkbit PUBLIC ${GTK2_INCLUDE_DIRS})

add_executable(glgtkbit glgtkbit.cpp)
target_link_libraries(glgtkbit ${GTK2_LIBRARIES} ${GTKGL_LIBRARIES})
target_include_directories(glgtkbit PUBLIC ${GTK2_INCLUDE_DIRS} ${GTKGL_INCLUDE_DIRS})
