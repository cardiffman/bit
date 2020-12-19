add_library(anim STATIC anim.cpp
	scene_builder.cpp
	parse_json_utils.cpp
	read_png_file.cpp
	read_JPEG_file.cpp
	engine.cpp)

if (USE_JSMN)
	target_include_directories(anim PUBLIC ../jsmn  ${FT_INCLUDE_DIRS})
endif()

if (USE_JSONCPP)
	target_compile_definitions(anim PRIVATE USE_JSONCPP=1)
	target_include_directories(anim PUBLIC ../jsoncpp/include  ${FT_INCLUDE_DIRS})
endif()

if (USE_JSMN)
	target_link_libraries(anim jsmn ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
endif()

if (USE_JSONCPP)
	target_link_libraries(anim jsoncpp_lib ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
endif()
