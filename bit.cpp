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

using std::cin;
using std::cout;
using std::vector;

struct Asset {
	unsigned id; uint8_t* mem; unsigned width; unsigned height; unsigned rowbytes;
};
struct Glyph {
	unsigned id; unsigned parent_id; unsigned asset_id; int x; int y; unsigned width; unsigned height; unsigned children;
};
struct Scene {
	vector<Asset> assets;
	vector<Glyph> glyphs;
};
static const unsigned NO_PAR = UINT_MAX;
Scene scene = {
		{
		{ 0, NULL, 1280, 720, 1280*4 },
		{ 1, NULL, 100, 100, 100*4 }
		},
		{
				{ 0, NO_PAR, 0, 0, 0, 1280, 720 },
				{ 1, NO_PAR, 1, 0, 0, 100, 100},
				{ 2, NO_PAR, 1, 100,100,1080,520 },
				{ 3, 2,      1, 0, 0, 100, 100},
				{ 4, 2,      1, 100, 0, 100, 100},
				{ 5, 2,      1, 200, 0, 100, 100},
				{ 6, 2,      1, 300, 0, 100, 100},
				{ 7, 2,      1, 400, 0, 100, 100},
				{ 8, 2,      1, 500, 0, 100, 100},
				{ 9, 2,      1, 600, 0, 100, 100},
				{ 10, 2,     1, 700, 0, 100, 100},
				{ 11, 2,     1, 800, 0, 100, 100},
				{ 12, 2,     1, 900, 0, 100, 100},
				{ 13, 2,     1, 1000, 0, 100, 100},
		}
};

void blit(uint8_t* srcbuf, unsigned srcbufwidth, unsigned srcbufheight, unsigned srcbufrowbytes
		, int srcx, int srcy, unsigned srcwidth, unsigned srcheight
		, uint8_t* dstbuf, unsigned dstbufwidth, unsigned dstbufheight, unsigned dstbufrowbytes
		, int dstx, int dsty, unsigned dstwidth, unsigned dstheight)
{
	for (unsigned y = 0; y<srcheight; y++) {
		uint32_t* dstrow = (uint32_t*)(dstbuf + (dsty+y)*dstbufrowbytes);
		uint32_t* srcrow = (uint32_t*)(srcbuf + (srcy+y)*srcbufrowbytes);
		for (unsigned x = 0; x<srcwidth; x++) {
			dstrow[x+dstx] = srcrow[x+srcx];
		}

	}
}
extern uint8_t* Mem;
void draw_scene()
{
	for (auto g : scene.glyphs) {
		g.children = 0;
	}
	for (auto g : scene.glyphs) {
		if (g.parent_id != NO_PAR)
			scene.glyphs[g.parent_id].children++;
	}

	for (auto g : scene.glyphs) {
		int offsetx = 0;
		int offsety = 0;
		if (g.children)
			continue;
		if (g.parent_id != NO_PAR)
		{
			offsetx = scene.glyphs[g.parent_id].x;
			offsety = scene.glyphs[g.parent_id].y;
		}
		blit(scene.assets[g.asset_id].mem, scene.assets[g.asset_id].width, scene.assets[g.asset_id].height, scene.assets[g.asset_id].rowbytes
				, 0, 0, scene.assets[g.asset_id].width, scene.assets[g.asset_id].height
				, Mem, 1280, 720, 1280*4
				, g.x+offsetx, g.y+offsety, g.width, g.height
				);
	}
}
int main(int, char**argv)
{
	auto file = std::fstream(argv[1], std::ios::in | std::ios::binary);
	Json::Value root;
	file >> root;
	cout << root << std::endl;

	scene.assets[0].mem = new uint8_t[720*1280*4];
	uint32_t* pixels = (uint32_t*)scene.assets[0].mem;
	for (unsigned y = 0; y<720; y++)
		for (unsigned x = 0; x<1280; x++)
			pixels[y*1280+x] = ((x^y)&1)?0xFFFFFFFF:0xFF000000;
	scene.assets[1].mem = new uint8_t[100*100*4];
	pixels = (uint32_t*)scene.assets[1].mem;
	for (unsigned y = 0; y<100; y++)
		for (unsigned x = 0; x<100; x++)
			pixels[y*100+x] = 0xFFFF0000;
	bitwindow* win = bitwindow::create();
	win->configure(Rect((1920-1280)/2,(1080-720)/2,1280,720));
	draw_scene();
	win->repaint();
	//XCBWindow::eventLoop();
	while (true)
	{
		pollfd pollfds[1] = { { XCBWindow::event_fd(), POLLIN, 0} };
		int p = poll(pollfds, 1, 16);
		if (p > 0) {
			if (pollfds[0].revents == POLLIN) {
				if (!XCBWindow::pollEvent())
					break;
			}
		}
		else
		{
			static int dx=8;
			static int dy=8;
			scene.glyphs[1].x+=dx;
			scene.glyphs[1].y+=dy;
			if (scene.glyphs[1].x >= 1180)
				dx=-dx;
			else if (scene.glyphs[1].x <= 0)
				dx=-dx;
			if (scene.glyphs[1].y >= 620)
				dy=-dy;
			else if (scene.glyphs[1].y <= 0)
				dy=-dy;
			draw_scene();
			win->repaint();
		}
	}
	return 0;
}


