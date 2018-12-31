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
#include <list>
#include <unistd.h>
#include <climits>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include "read_png_file.h"
#include "read_JPEG_file.h"
#include "scene.h"
#include "scene_builder.h"

using std::cin;
using std::cout;
using std::vector;
using std::cerr;
using std::endl;

Scene scene = {
		{
		{ 0, {NULL, {0,0}, 0} },
		{ 1, {NULL, {1280, 720}, 1280*4 }},
		{ 2, {NULL, {100, 100}, 100*4 }},
		{ 3, 0},
		{ 4, 0},
		{ 5, 0},
		{ 6, 0},
		{ 7, 0},
		{ 8, 0},
		{ 9, 0},
		{10, 0},
		{11, 0},
		{12, 0},
		{13, 0},
		{14, 0},
		{15, 0},
		},
		{
				{ {0,0,0,0}, 0, 0, 0 },
				{ {  0,  0, 1280, 720}, 1, ID_NULL, ID_NULL, 0xff808080 },
				{ {  0,  0,  100, 100}, 2, ID_NULL, 3},//ID_NULL, 0xffff0000},
				{ {100,100, 1080, 520}, 3, ID_NULL, 1,  },
				{ {  0,  0,  127, 190}, 4, 3,       3},
				{ {127,  0,  127, 190}, 5, 3,       4, },
				{ {254,  0,  127, 190}, 6, 3,       5, },
				{ {381,  0,  127, 190}, 7, 3,      6, },
				{ {508,  0, 127, 190}, 8, 3,      7, },
				{ {635,  0, 127, 190}, 9, 3,      8, },
				{ {762,  0, 127, 190},10, 3,      9, },
				{ {889,  0, 127, 190},11, 3,     10, },
				{ {1016, 0, 127, 190},12, 3,     11, },
				{ {1143, 0, 127, 190},13, 3,     12, },
				{ {1270, 0, 127, 190},14, 3,     2, },
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
void stretch(BitBuffer& src
		, const Area& srcArea
		, BitBuffer& dst
		, const Area& dstArea)
{
	int x_ratio = (int)((srcArea.width<<16)/dstArea.width) +1;
    int y_ratio = (int)((srcArea.height<<16)/dstArea.height) +1;
    for (int i=0; i<dstArea.height; ++i)
    {
		int y2 = ((i*y_ratio)>>16);
		uint32_t* dstmem = (uint32_t*)(dst.mem + dst.rowbytes*(i+dstArea.y)+4*dstArea.x);
		uint32_t* srcmem = (uint32_t*)(src.mem + src.rowbytes*(y2+srcArea.y)+4*srcArea.x);
    	for (int j=0; j<dstArea.width; ++j)
    	{
    		int x2 = ((j*x_ratio)>>16);
    		dstmem[j] = srcmem[x2];
    		//dst.mem[((i*dst.dims.width)+j)*4+0]=src.mem[((y2*src.dims.width)+x2)*4+0];
    		//dst.mem[((i*dst.dims.width)+j)*4+1]=src.mem[((y2*src.dims.width)+x2)*4+1];
    		//dst.mem[((i*dst.dims.width)+j)*4+2]=src.mem[((y2*src.dims.width)+x2)*4+2];
    		//dst.mem[((i*dst.dims.width)+j)*4+3]=src.mem[((y2*src.dims.width)+x2)*4+3];
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
		cout << "check_parent_loop: broken container: " << container.id << ' ' << container.parent_id << endl;
		g = &container;
		parent = parent_container(g);
		while (parent && count < 100)
		{
			cout << "check_parent_loop: parent chain:" << parent->id << ' ' << parent->parent_id << endl;
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
void draw_container(const Container& g)
{
	if (g.children)
	{
		cout << "Container " << g.id << " has children, they will be drawn" << endl;
		return;
	}

	Area draw, container_screen;
	if (CLIP_OUT == clip_container_to_heirarchy(g, draw, container_screen))
	{
		cout << "Container " << g.id << " clipped out" << endl;
		auto parent = parent_container(&g);
		cout << " " << g.area.x << ',' << g.area.y << ',' << g.area.width << ',' << g.area.height << endl;
		cout << " " << parent->area.x << ',' << parent->area.y << ',' << parent->area.width << ',' << parent->area.height << endl;
		return;
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
			return;
		}
		asset_area;
		cout << "Drawing asset " << asset.id << " for container " << g.id << endl;
		if (g.area.width == asset_area.width && g.area.height == asset_area.height)
		blit(asset.image
				, asset_area
				, screen
				, g.area
				);
		else
			stretch(asset.image, asset_area, screen, g.area);
	}

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
		draw_container(g);
	}
}
double get_time()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec + (ts.tv_nsec*1e-9);
}
#include "parse_json_utils.h"
#include "scene_builder.h"
int main(int argc, char**argv)
{
	try {
		//parse_json(argv[1]);
		if (argc==2)
		{
			SceneBuilder builder;
			builder.parse_containers(argv[1]);
			scene.containers = builder.nc;
			scene.assets = builder.na;
		}
	} catch (const char* ex) {
		cout << ex << endl;
		return 1;
	}
	if (argc == 1)
	{
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

	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/Vice2018.jpg", &scene.assets[3].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/BB_Online_Dom_Payoff_1-Sheet_H-Steinfeld_BB_Bridge_Autobot.jpg", &scene.assets[4].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/GRC_Tsr1Sheet_GrinchAndMax_.jpg", &scene.assets[5].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MULE_VERT_MAIN_DOM_2764x4096_master.jpg", &scene.assets[6].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/TheFavourite2018.jpg", &scene.assets[7].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/SecondAct_27x40_1Sheet_RGB.jpg", &scene.assets[8].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/AQAMN_VERT_MAIN_DUO_DOM_2764x4096_master.jpg", &scene.assets[9].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/TSNGO_TicketingBanner_250x375_r2.jpg", &scene.assets[10].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/HolmesAndWatson2018.jpg", &scene.assets[11].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/SpiderManIntoTheSpiderVerse2018.jpg", &scene.assets[12].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/WTM_HeroPoster.jpg", &scene.assets[13].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MQOS_OneSheet.jpg", &scene.assets[14].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MPR-Payoff_1-Sheet_v8a_Sm.jpg", &scene.assets[15].image);
	}
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
	}
	return 0;
}


