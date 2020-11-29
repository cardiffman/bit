add_library(anim STATIC anim.cpp
	scene_builder.cpp
	parse_json_utils.cpp
	read_png_file.cpp
	read_JPEG_file.cpp
	engine.cpp)

target_include_directories(anim PUBLIC ../jsmn  ${FT_INCLUDE_DIRS})

target_link_libraries(anim jsmn ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
