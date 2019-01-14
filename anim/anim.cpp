/*
 * anim.cpp
 *
 *  Created on: Jan 3, 2019
 *      Author: menright
 */

#include "scene.h"
#include "engine.h"
#include <iostream>

using std::cout;
using std::endl;

std::ostream& operator <<(std::ostream& out, const Area& diag)
{
	out << '('<<diag.x << ',' << diag.y << ',' << diag.width << ',' << diag.height << ')';
	return out;
}
std::ostream& operator <<(std::ostream& out, const RectSize& diag)
{
	out << '(' << diag.width << ',' << diag.height << ')';
	return out;
}
Container* parent_container(Scene& scene, const Container* container)
{
	Container* parent = &scene.containers[container->parent_id];
	if (parent->id == ID_NULL)
		return nullptr;
	return parent;
}
enum { CLIP_IN, CLIP_PARTIAL, CLIP_OUT };
int number_line(int low_limit, int size_limit, int low, int size, int* draw_low, int* draw_size)
{
	if (low+size < low_limit)
	{
		*draw_low = *draw_size = 0;
		return CLIP_OUT;
	}
	if (low > low_limit+size_limit)
	{
		*draw_low = *draw_size = 0;
		return CLIP_OUT;
	}
	if (low >= low_limit && low+size <= low_limit+size_limit)
	{
		*draw_low = low;
		*draw_size = size;
		return CLIP_IN;
	}
	int draw_left = low; int draw_right = low+size;
	if (draw_left < low_limit)
	{
		draw_left = low_limit;
	}
	if (draw_right > low_limit+size_limit)
	{
		draw_right = low_limit+size_limit;
	}
	*draw_low = draw_left;
	*draw_size = draw_right - draw_left;
	return CLIP_PARTIAL;
}
int clip_area_to_area(const Area& limit_area, const Area& area, Area& draw)
{
	int res1 = number_line(limit_area.x, limit_area.width, area.x, area.width, &draw.x, &draw.width);
	if (res1 == CLIP_OUT)
	{
		draw.y = 0; draw.height = 0;
		return res1;
	}
	int res2 = number_line(limit_area.y, limit_area.height, area.y, area.height, &draw.y, &draw.height);
	if (res2 == CLIP_OUT)
	{
		draw.x = 0; draw.width = 0;
		return res2;
	}
	if (res1 == CLIP_IN && res2 == CLIP_IN)
		return CLIP_IN;
	return CLIP_PARTIAL;
}
int clip_container_to_heirarchy(Scene& scene, const Container& container, Area& draw, Area& screen)
{
	draw = container.area;
	screen = container.area;
	Container* parent = parent_container(scene, &container);
	int res = CLIP_IN;
	while (parent)
	{
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
		{
			return CLIP_OUT;
		}
		parent = parent_container(scene, parent);
	}
	res = clip_area_to_area(Area(0,0,1280,720), draw, draw);
	return res;
}
/*
 * @param container a Container that contains some area (such as asset area)
 * @param draw the area to start with and the final clipped area
 * @param screen the final area without clipping.
 */
int clip_area_to_heirarchy(Scene& scene, const Container& container, Area& draw, Area& screen)
{
	draw.x += container.area.x;
	draw.y += container.area.y;
	screen = draw;
	Container* parent = parent_container(scene, &container);
	int res = CLIP_IN;
	while (parent)
	{
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
		{
			return CLIP_OUT;
		}
		parent = parent_container(scene, parent);
	}
	return res;
}

void draw_container(const Container& g, Scene& scene, GraphicsBuffer* screen, GraphicsEngine* engine)
{
	if (g.children)
	{
		cout << "Container " << g.id << " has children, they will be drawn" << endl;
		return;
	}

	Area draw, container_screen;
	if (CLIP_OUT == clip_container_to_heirarchy(scene, g, draw, container_screen)
		|| draw.width == 0
		|| draw.height == 0)
	{
		cout << "Container " << g.id << " clipped out" << endl;
		return;
	}
	if (g.asset_id == ID_NULL)
	{
		cout << "Filling color " << std::hex << g.color << std::dec << " for container " << g.id << " at " << draw << endl;
		engine->fill(screen, draw, g.color);
	}
	else
	{
		Area asset_draw, asset_screen;
		Asset& asset = scene.assets[g.asset_id];
		Area asset_area = Area(0,0,asset.image->dims.width,asset.image->dims.height);
		asset_draw = asset_area;
		if (CLIP_OUT == clip_area_to_heirarchy(scene, g, asset_draw, asset_screen))
		{
			cout << "Container " << g.id << " asset " << asset.id << " clipped out" << endl;
			return;
		}
		asset_area;
		bool unscaled = g.area.width == asset.image->dims.width && g.area.height == asset.image->dims.height;
		cout << "Drawing asset " << asset.id << " for container " << g.id << ' ' << (unscaled?"unscaled":"scaled") <<" at " << asset_draw << " from " << asset.image->dims << " as " << asset_screen << " csc " << draw << " cs " << container_screen << endl;
		// If the asset area and the container area are equal then
		// draw 1:1 using the clipping so far.
		if (g.area.width == asset.image->dims.width && g.area.height == asset.image->dims.height)
		{
			cout << "unscaled" << endl;
			engine->blit(screen
					, draw.x, draw.y
					, asset.image
					, asset_area
					);
		}
		else
		{
			// If the asset has to be stretched, then the clipped screen coordinates of the container
			// need to be mapped down to the asset's u,v space.
			// Believe it or not I got this right the second time I wrote it from scratch.
			// In this case, asset_draw set up with the asset's coordinates and then mapped to
			// the screen and clipped. asset_screen is the asset's screen coordinates without clipping.
			// However if the above conditional is false (which it is) then the rectangle
			// The lhs of the asset in u,v is asset_draw.x - asset_screen.x
			// The rhs of the asset in u,v is asset_draw.x + asset_draw.width - asset_screen.x
			int num_asset_width = asset.image->dims.width;
			int den_asset_width = container_screen.width;
			int num_asset_height = asset.image->dims.height;
			int den_asset_height = container_screen.height;
			asset_area.x = (draw.x - container_screen.x) * num_asset_width / den_asset_width;
			asset_area.width = draw.width * num_asset_width / den_asset_width;
			asset_area.y = (draw.y - container_screen.y) * num_asset_height / den_asset_height;
			asset_area.height = draw.height * num_asset_height /den_asset_height;
			cout << " Transformed " << asset_area << endl;
			cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
					<< den_asset_width << ',' << den_asset_height << ')' << endl;
			// Sanity check:
			if (asset_area.x < 0)
			{
				cout << asset_area << " asset draw left is negative" << endl;
				return;
			}
			if (asset_area.width > asset.image->dims.width)
			{
				cout << asset_area << " vs " << asset.image->dims << " asset draw width is greater than asset width " << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				cout << " Input width " << draw.width << endl;
				return;
			}
			if (asset_area.x + asset_area.width > asset.image->dims.width)
			{
				cout << asset_area << " vs " << asset.image->dims << " asset draw right exceeds asset right" << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				return;
			}
			if (asset_area.y < 0)
			{
				cout << asset_area << " asset draw top is negative" << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				return;
			}
			if (asset_area.height > asset.image->dims.height)
			{
				cout << asset_area << " vs " << asset.image->dims << " asset draw height is greater than asset height " << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				return;
			}
			if (asset_area.y + asset_area.height > asset.image->dims.height)
			{
				cout << asset_area << " vs " << asset.image->dims << " asset draw bottom exceeds asset bottom" << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				return;
			}
			engine->stretchSrcCopy(screen, draw, asset.image, asset_area);
		}
	}

}

/* \brief return true if container's ancestors have a loop
 * @param container the container of interest.
 */
bool check_parent_loop(Scene& scene, const Container& container)
{
	Container* parent = nullptr;
	const Container* g = &container;
	int count = 0;
	parent = parent_container(scene, g);
	while (parent && count < 100)
	{
		parent = parent_container(scene, parent);
		count++;
	}
	if (count >= 100)
	{
		cout << "check_parent_loop: broken container: " << container.id << ' ' << container.parent_id << endl;
		g = &container;
		parent = parent_container(scene, g);
		while (parent && count < 100)
		{
			cout << "check_parent_loop: parent chain:" << parent->id << ' ' << parent->parent_id << endl;
			parent = parent_container(scene, parent);
			count++;
		}
		return true;
	}
	return false;
}

int clip_container_to_parents(Scene& scene, const Container& container, Area& draw)
{
	draw = container.area;
	Container* parent = parent_container(scene, &container);
	int res = CLIP_IN;
	while (parent)
	{
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
			return CLIP_OUT;
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		parent = parent_container(scene, parent);
	}
	return res;
}
void make_screen_rect(Scene& scene, const Container& container, Area& screen)
{
	screen = container.area;
	auto parent = parent_container(scene, &container);
	while (parent)
	{
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		parent = parent_container(scene, parent);
	}
}
void draw_scene(Scene& scene, GraphicsEngine* engine)
{
	cout << "Drawing" << endl;
	for (auto& g : scene.containers) {
		g.children = 0;
	}
	for (auto& g : scene.containers) {
		if (g.parent_id != ID_NULL)
			scene.containers[g.parent_id].children++;
	}
	Container* parent = nullptr;
	for (unsigned ig = 1; ig<scene.containers.size(); ++ig) {
		Container& g = scene.containers[ig];
		if (check_parent_loop(scene, g))
			continue;
		draw_container(g, scene, engine->getScreenBuffer(), engine);
	}
}


