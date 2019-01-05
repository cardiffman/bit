/*
 * scene.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef BIT_SCENE_H_
#define BIT_SCENE_H_

#include <vector>
#include "bitbuffer.h"

struct Area {
	Area() : x(), y(), width(), height() {}
	Area(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
	int x; int y; int width; int height;
};
struct Asset {
	unsigned id; BitBuffer image;
};
struct Container {
	Area area; unsigned id; unsigned parent_id; unsigned asset_id; unsigned color; unsigned children;
};
struct Scene {
	std::vector<Asset> assets;
	std::vector<Container> containers;
};
static const unsigned ID_NULL = 0;

void draw_scene(Scene& scene, BitBuffer& buf);



#endif /* BIT_SCENE_H_ */
