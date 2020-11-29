cmake_minimum_required(VERSION 3.7)
project(bit C CXX)

add_executable(sdlbit sdlbit.cpp sdlaux.cpp)
set_property(TARGET bit PROPERTY CXX_STANDARD 14)
message(STATUS "sdl includes ${SDL2_INCLUDE_DIRS}")
target_include_directories(sdlbit PRIVATE ../anim ../jsmn ${SDL2_INCLUDE_DIRS})
target_link_libraries(sdlbit anim ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${FT_LIBRARIES})

