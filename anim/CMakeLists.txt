add_library(anim STATIC anim.cpp
	scene_builder.cpp
	parse_json_utils.cpp
	read_png_file.cpp
	read_JPEG_file.cpp
	engine.cpp)

set(ANIM_INCLUDE_DIRS ${FT_INCLUDE_DIRS})
if (USE_JSMN)
target_compile_definitions(anim PRIVATE USE_JSMN=1)
list(APPEND ANIM_INCLUDE_DIRS ../jsmn)
endif()
if (USE_JSONCPP)
target_compile_definitions(anim PRIVATE USE_JSONCPP=1)
list(APPEND ANIM_INCLUDE_DIRS ../jsoncpp/include)
endif()

target_include_directories(anim PRIVATE ${ANIM_INCLUDE_DIRS})
set(ANIM_LINK_DIRS ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
if (USE_JSONCPP)
#	target_link_libraries(anim jsoncpp_lib ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
	list(APPEND ANIM_LINK_DIRS jsoncpp_lib)
endif()
if (USE_JSMN)
	list(APPEND ANIM_LINK_DIRS jsmn)
endif()
target_link_libraries(anim ${ANIM_LINK_DIRS})
