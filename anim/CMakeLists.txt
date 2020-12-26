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
target_compile_definitions(anim PUBLIC USE_JSONCPP=1)
list(APPEND ANIM_INCLUDE_DIRS ../jsoncpp/include)
endif()

if (NOT USE_JSONCPP AND NOT USE_JSMN)
list(APPEND ANIM_INCLUDE_DIRS ../cmjson)
endif()

target_include_directories(anim PUBLIC ${ANIM_INCLUDE_DIRS})
set(ANIM_LINK_LIBS ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${FT_LIBRARIES})
if (USE_JSONCPP)
	list(APPEND ANIM_LINK_LIBS jsoncpp_lib)
endif()
if (USE_JSMN)
	list(APPEND ANIM_LINK_LIBS jsmn)
endif()
if (NOT USE_JSONCPP AND NOT USE_JSMN)
	list(APPEND ANIM_LINK_LIBS cmjson)
endif()
target_link_libraries(anim PUBLIC ${ANIM_LINK_LIBS})
