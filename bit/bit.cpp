/*
 * bit.cpp
 *
 *  Created on: Nov 17, 2018
 *      Author: menright
 */

#include "bitwindow.h"
#include "json/json.h"
#include <fstream>
#include <iostream>
#include <poll.h>
#include <vector>
#include <unistd.h>
#include <climits>
#include <fcntl.h>

using std::cin;
using std::cout;
using std::vector;
using std::cerr;
using std::endl;

struct Asset {
	unsigned id; BitBuffer image;
};
struct Container {
	Area area; unsigned id; unsigned parent_id; unsigned asset_id; unsigned color; unsigned children;
};
struct Scene {
	vector<Asset> assets;
	vector<Container> containers;
};
static const unsigned ID_NULL = 0;
Scene scene = {
		{
		{ 0, {NULL, {0,0}, 0} },
		{ 1, {NULL, {1280, 720}, 1280*4 }},
		{ 2, {NULL, {100, 100}, 100*4 }},
		{ 3, 0},
		},
		{
				{ {0,0,0,0}, 0, 0, 0 },
				{ {0, 0, 1280, 720}, 1, ID_NULL, ID_NULL, 0xff808080 },
				{ {0, 0, 100, 100}, 2, ID_NULL, ID_NULL, 0xffff0000},
				{ {100,100,1080,520}, 3, ID_NULL, 1,  },
				{ {  0, 0, 100, 100}, 4, 3, },
				{ {100, 0, 100, 100}, 5, 3,      2, },
				{ {200, 0, 100, 100}, 6, 3,      2, },
				{ {300, 0, 100, 100}, 7, 3,      2, },
				{ {400, 0, 100, 100}, 8, 3,      2, },
				{ {500, 0, 100, 100}, 9, 3,      2, },
				{ {600, 0, 100, 100},10, 3,      2, },
				{ {700, 0, 100, 100},11, 3,     2, },
				{ {800, 0, 100, 100},12, 3,     2, },
				{ {900, 0, 100, 100},13, 3,     2, },
				{ {1000, 0, 100, 100},14, 3,     2, },
		}
};

void blit(BitBuffer& src
		, const Area& srcArea
		, BitBuffer& dst
		, const Area& dstArea)
{
	for (unsigned y = 0; y<srcArea.height; y++) {
		uint32_t* dstrow = (uint32_t*)(dst.mem + (dstArea.y+y)*dst.rowbytes);
		uint32_t* srcrow = (uint32_t*)(src.mem + (srcArea.y+y)*src.rowbytes);
		for (unsigned x = 0; x<srcArea.width; x++) {
			dstrow[x+dstArea.x] = srcrow[x+srcArea.x];
		}
	}
}
void fill(uint32_t color
		, BitBuffer& dst
		, const Area& dstArea)
{
	for (unsigned y = 0; y<dstArea.height; y++) {
		uint32_t* dstrow = (uint32_t*)(dst.mem + (dstArea.y+y)*dst.rowbytes);
		for (unsigned x = 0; x<dstArea.width; x++) {
			dstrow[x+dstArea.x] = color;
		}
	}
}
extern BitBuffer screen;
Container* parent_container(const Container* container)
{
	Container* parent = &scene.containers[container->parent_id];
	if (parent->id == ID_NULL)
		return nullptr;
	return parent;
}
/* \brief return true if container's ancestors have a loop
 * @param container the container of interest.
 */
bool check_parent_loop(const Container& container)
{
	Container* parent = nullptr;
	const Container* g = &container;
	int count = 0;
	parent = parent_container(g);
	while (parent && count < 100)
	{
		parent = parent_container(parent);
		count++;
	}
	if (count >= 100)
	{
		cout << "check_parent_loop: " << container.id << ' ' << container.parent_id << endl;
		g = &container;
		parent = parent_container(parent);
		while (parent && count < 100)
		{
			cout << "check_parent_loop: " << parent->id << ' ' << parent->parent_id << endl;
			parent = parent_container(parent);
			count++;
		}
		return true;
	}
	return false;
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
int clip_container_to_parents(const Container& container, Area& draw)
{
	draw = container.area;
	Container* parent = parent_container(&container);
	int res = CLIP_IN;
	while (parent)
	{
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
			return CLIP_OUT;
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		parent = parent_container(parent);
	}
	return res;
}
int clip_container_to_heirarchy(const Container& container, Area& draw, Area& screen)
{
	draw = container.area;
	screen = container.area;
	Container* parent = parent_container(&container);
	int res = CLIP_IN;
	while (parent)
	{
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
			return CLIP_OUT;
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		parent = parent_container(parent);
	}
	res = clip_area_to_area(Area(0,0,1280,720), draw, draw);
	return res;
}
/*
 * @param container a Container that contains some area (such as asset area)
 * @param draw the area to start with and the final clipped area
 * @param screen the final area without clipping.
 */
int clip_area_to_heirarchy(const Container& container, Area& draw, Area& screen)
{
	draw.x += container.area.x;
	draw.y += container.area.y;
	screen = draw;
	Container* parent = parent_container(&container);
	int res = CLIP_IN;
	while (parent)
	{
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
			return CLIP_OUT;
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		parent = parent_container(parent);
	}
	return res;
}
void draw_scene()
{
	cout << "Drawing" << endl;
	for (auto g : scene.containers) {
		g.children = 0;
	}
	for (auto g : scene.containers) {
		if (g.parent_id != ID_NULL)
			scene.containers[g.parent_id].children++;
	}
	Container* parent = nullptr;
	for (unsigned ig = 1; ig<scene.containers.size(); ++ig) {
		Container& g = scene.containers[ig];
		if (check_parent_loop(g))
			continue;

		if (g.children)
		{
			cout << "Container " << g.id << " has children, they will be drawn" << endl;
			continue;
		}

		Area draw, container_screen;
		if (CLIP_OUT == clip_container_to_heirarchy(g, draw, container_screen))
		{
			cout << "Container " << g.id << " clipped out" << endl;
			continue;
		}
		if (g.asset_id == ID_NULL)
		{
			cout << "Filling color " << g.color << " for container " << g.id << " at " << g.area.x << "," << g.area.y << endl;
			fill(g.color, screen, draw);
		}
		else
		{
			Area asset_draw, asset_screen;
			Asset& asset = scene.assets[g.asset_id];
			Area asset_area = Area(0,0,asset.image.dims.width,asset.image.dims.height);
			asset_draw = asset_area;
			if (CLIP_OUT == clip_area_to_heirarchy(g, asset_draw, asset_screen))
			{
				cout << "Container " << g.id << " asset " << asset.id << " clipped out" << endl;
				continue;
			}
			asset_area;
			cout << "Drawing asset " << asset.id << " for container " << g.id << endl;
			blit(asset.image
					, asset_area
					, screen
					, g.area
					);
		}
	}
}
double get_time()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec + (ts.tv_nsec*1e-9);
}
double timeProcessing(double& anim_last, bitwindow* win)
{
	static double anim_next = 0;
	double anim_now = get_time();

	static int dx=60;
	static int dy=60;
	double deltat = anim_now-anim_last;
	//cout << "Delta t: " << deltat << endl;
	anim_last=get_time();
	anim_next = anim_last + 1.0/60;
	//cout << "Container 2 x " << scene.glyphs[2].area.x << " y " << scene.glyphs[2].area.y << " " << deltat << endl;
	scene.containers[2].area.x+=dx * deltat;
	scene.containers[2].area.y+=dy * deltat;
	if (scene.containers[2].area.x+scene.containers[2].area.width > 1280)
	{
		cout << "X Flip: " << scene.containers[2].area.x << ' ' << scene.containers[2].area.x+scene.containers[2].area.width << ' ' << dx << endl;
		int overage = scene.containers[2].area.x+scene.containers[2].area.width - 1280;
		cout << "X Flip overage " << overage << endl;
		scene.containers[1].area.x = 1280 - scene.containers[2].area.width - overage;
		dx=-60;
		cout << "X Flip: " << scene.containers[2].area.x << ' ' << scene.containers[2].area.x+scene.containers[2].area.width << ' ' << dx << endl;
	}
	else if (scene.containers[2].area.x < 0)
	{
		cout << "X Flip: " << scene.containers[2].area.x << ' ' << dx << endl;
		int overage = -scene.containers[2].area.x;
		scene.containers[2].area.x = 0 + overage;
		dx=60;
		cout << "X Flip: " << scene.containers[2].area.x << ' ' << dx << endl;
	}
	if (scene.containers[2].area.y+scene.containers[2].area.height > 720)
	{
		cout << "Y Flip 720: " << scene.containers[2].area.y << ' ' << dy << endl;
		int overage = scene.containers[2].area.y+scene.containers[2].area.height - 720;
		cout << "Y Flip overage " << overage << endl;
		scene.containers[2].area.y = 720 - scene.containers[2].area.height - overage;
		if (scene.containers[2].area.y+scene.containers[2].area.height > 720)
		{
			cout << "Flipped but still bad?? " << scene.containers[2].area.y << ' ' << scene.containers[2].area.y+scene.containers[2].area.height << endl;
		}
		dy=-60;
		cout << "Y Flip 720: " << scene.containers[2].area.y << ' ' << dy << endl;
	}
	else if (scene.containers[2].area.y < 0)
	{
		cout << "Y Flip: " << scene.containers[2].area.y << ' ' << dy << endl;
		int overage = -scene.containers[2].area.y;
		scene.containers[2].area.y = 0 + overage;
		dy=-dy;
		cout << "Y Flip: " << scene.containers[2].area.y << ' ' << dy << endl;
	}
	draw_scene();
	win->repaint();
	return anim_next;
}

#include "jsmn.h"
void parse_json(const char* file)
{
	jsmn_parser jp;
	jsmn_init(&jp);
	int fd = open(file, O_RDONLY);
	int jslength = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	char* jstext = new char[jslength+1];
	read(fd, jstext, jslength);
	int tokes = jsmn_parse(&jp, jstext, jslength, NULL, 0);
	jsmntok_t* tokens = new jsmntok_t[tokes];
	jsmn_init(&jp);
	jsmn_parse(&jp, jstext, jslength, tokens, tokes);
	for (int i=0; i<tokes; ++i)
	{
		cout << i << ": type=" << tokens[i].type << " start: " << tokens[i].start << " end: " << tokens[i].end
			<< " size: " << tokens[i].size << endl;//<< " " << std::string(tokens[i].end-tokens[i].start, jstext+tokens[i].start);
	}
	close(fd);
}
int main(int, char**argv)
{
	//auto file = std::fstream(argv[1], std::ios::in | std::ios::binary);
	//Json::Value root;
	//file >> root;
	//cout << root << std::endl;
	parse_json(argv[1]);

	scene.assets[1].image.mem = new uint8_t[720*1280*4];
	scene.assets[1].image.rowbytes = 1280*4;
	scene.assets[1].image.dims = RectSize(1280,720);
	uint32_t* pixels = (uint32_t*)scene.assets[1].image.mem;
	for (unsigned y = 0; y<720; y++)
		for (unsigned x = 0; x<1280; x++)
			pixels[y*1280+x] = ((x^y)&1)?0xFFFFFFFF:0xFF000000;
	scene.assets[2].image.mem = new uint8_t[100*100*4];
	scene.assets[2].image.rowbytes = 100*4;
	scene.assets[2].image.dims = RectSize(100,100);
	pixels = (uint32_t*)scene.assets[2].image.mem;
	for (unsigned y = 0; y<100; y++)
		for (unsigned x = 0; x<100; x++)
			pixels[y*100+x] = 0xFFFF0000;
	bitwindow* win = bitwindow::create();
	win->configure(Rect((1920-1280)/2,(1080-720)/2,1280,720));
	draw_scene();
	win->repaint();
	//XCBWindow::eventLoop();

	double anim_last = get_time();
	double anim_next = anim_last+1.0/60;
	double poll_xcb = 0;
	while (true)
	{
		if (!XCBWindow::pollEvents())
			break;
		//cout << "Time Check " << endl;
		double anim_now = get_time();
		if (anim_now >= anim_next)
		{
			anim_next = timeProcessing(anim_last, win);
			cout << "Tick : " << anim_next << endl;
		}
	}
	return 0;
}


