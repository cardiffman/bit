project (2d CXX)

add_executable (2d twodee.cpp)

#target_include_directories(2d PUBLIC ../anim ../jsmn ../XCB ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS})
target_include_directories(2d PUBLIC ../anim)

target_link_libraries(2d GL GLU glut anim ${FT_LIBRARIES})
