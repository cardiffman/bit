/*
 * scene.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef BIT_SCENE_H_
#define BIT_SCENE_H_

#include <vector>

class GraphicsBuffer;
class GraphicsEngine;

#include "geometry.h"

struct Asset {
	unsigned id; GraphicsBuffer* image;
};
struct Container {
	Area area; unsigned id; unsigned parent_id; unsigned asset_id; unsigned color; unsigned children;
};
struct Scene {
	std::vector<Asset> assets;
	std::vector<Container> containers;
};
static const unsigned ID_NULL = 0;

void draw_scene(Scene& scene, GraphicsEngine* engine);
void draw_scene2(Scene& scene, GraphicsEngine* engine);



#endif /* BIT_SCENE_H_ */
