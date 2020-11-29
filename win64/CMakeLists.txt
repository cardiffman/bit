project (win CXX)
add_executable(win win.cpp)
target_include_directories(win PRIVATE ../anim ../jsmn)
target_link_libraries(win PRIVATE gdi32 cairo anim ${FT_LIBRARIES})
