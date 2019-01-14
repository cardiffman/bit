/*
 * bit.cpp
 *
 *  Created on: Nov 17, 2018
 *      Author: menright
 */

#include "bitwindow.h"
#include <iostream>
#include <vector>
#include "read_JPEG_file.h"
#include "scene.h"
#include "scene_builder.h"
#include "engine.h"

using std::cin;
using std::cout;
using std::vector;
using std::cerr;
using std::endl;

Scene scene = {
		{
		{ 0, 0},
		{ 1, 0},
		{ 2, 0},
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
		{13, 0}
		},
		{
				{ {0,0,0,0}, 0, 0, 0 },
				{ {  0,  0, 1280, 720}, 1, ID_NULL, ID_NULL, 0xff808080 },
				{ {  0,  0,  100, 100}, 2, ID_NULL, ID_NULL, 0xffff0000},
				{ {100,100, 1080, 520}, 3, ID_NULL, ID_NULL, 0xffff0000  },
				{ {  0,  0,  127, 190}, 4, 3,       1},
				{ {127,  0,  127, 190}, 5, 3,       2, },
				{ {254,  0,  127, 190}, 6, 3,       3, },
				{ {381,  0,  127, 190}, 7, 3,      4, },
				{ {508,  0, 127, 190}, 8, 3,      5, },
				{ {635,  0, 127, 190}, 9, 3,      6, },
				{ {762,  0, 127, 190},10, 3,      7, },
				{ {889,  0, 127, 190},11, 3,      8, },
				{ {1016, 0, 127, 190},12, 3,      9, },
				{ {1143, 0, 127, 190},13, 3,     10, },
				{ {1270, 0, 127, 190},14, 3,     1, },
		}
};

double get_time()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec + (ts.tv_nsec*1e-9);
}
int main(int argc, char**argv)
{
	GraphicsEngine* engine = init_base_engine();
	try {
		if (argc==2)
		{
			SceneBuilder builder;
			builder.parse_containers(argv[1], engine);
			scene.containers = builder.nc;
			scene.assets = builder.na;
		}
	} catch (const char* ex) {
		cout << ex << endl;
		return 1;
	}
	if (argc == 1)
	{
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/Vice2018.jpg", engine, scene.assets[1].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/BB_Online_Dom_Payoff_1-Sheet_H-Steinfeld_BB_Bridge_Autobot.jpg", engine, scene.assets[2].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/GRC_Tsr1Sheet_GrinchAndMax_.jpg", engine, scene.assets[3].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MULE_VERT_MAIN_DOM_2764x4096_master.jpg", engine, scene.assets[4].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/TheFavourite2018.jpg", engine, scene.assets[5].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/SecondAct_27x40_1Sheet_RGB.jpg", engine, scene.assets[6].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/AQAMN_VERT_MAIN_DUO_DOM_2764x4096_master.jpg", engine, scene.assets[7].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/TSNGO_TicketingBanner_250x375_r2.jpg", engine, scene.assets[8].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/HolmesAndWatson2018.jpg", engine, scene.assets[9].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/SpiderManIntoTheSpiderVerse2018.jpg", engine, scene.assets[10].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/WTM_HeroPoster.jpg", engine, scene.assets[11].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MQOS_OneSheet.jpg", engine, scene.assets[12].image);
		read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MPR-Payoff_1-Sheet_v8a_Sm.jpg", engine, scene.assets[13].image);
	}
	bitwindow* win = bitwindow::create(engine);
	win->configure(Rect((1920-1280)/2,(1080-720)/2,1280,720));
	draw_scene(scene,engine);
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


