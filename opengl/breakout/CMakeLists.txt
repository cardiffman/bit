project(game,CXX)
cmake_minimum_required(VERSION 3.7)

add_executable(game
	game.cpp
	breakout.cpp 
	texture.cpp 
	resource_manager.cpp
	shader.cpp
	sprite_renderer.cpp
	)

target_include_directories(game PRIVATE /usr/include/SOIL)
target_link_libraries(game glut GLEW glfw GL SOIL)
