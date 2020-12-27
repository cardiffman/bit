project (2d CXX)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_executable (2d twodee.cpp)

target_include_directories(2d PUBLIC ../anim ../jsmn)

target_link_libraries(2d GL GLU glut anim ${FT_LIBRARIES})

#target_include_directories(2d PUBLIC ../anim ../jsmn ../XCB ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS})

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_link_libraries(2d GL GLU glut anim ${FT_LIBRARIES})
else()
target_include_directories(2d PUBLIC ../jsmn)
target_link_libraries(2d opengl32 glu32 png16 glut anim ${FT_LIBRARIES} ${PNG_LIBRARIES})
endif()
if (USE_JSMN)
target_compile_definitions(2d PRIVATE USE_JSMN=1)
elseif(USE_JSONCPP)
target_compile_definitions(2d PRIVATE USE_JSONCPP=1)
else()
endif()
install(TARGETS 2d DESTINATION .)

endif()
