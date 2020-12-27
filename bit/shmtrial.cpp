/*
 * shmtrial.cpp
 *
 *  Created on: Jan 7, 2019
 *      Author: menright
 */

#include "shmwindow.h"
#include "scene_builder.h"
#include "engine.h"
#include <cstring>
#include <unistd.h>

#include <iostream>
using std::cerr;
using std::endl;
using std::cout;
using std::clog;

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

const char* base_scene =
		"{ \"containers\": ["
			  "{ \"area\": {\"x\":0,\"y\":0,\"width\":1280,\"height\":720 }"
			  " ,\"fill\": [80,80,80]"
			  "} "
			  ", {\"area\": {\"x\":0,\"y\":0,\"width\":100, \"height\":100 }"
			  " ,\"fill\": [255,0,0]"
			  "} "
				", {\"area\": {\"x\":100,\"y\":100,\"width\":1080, \"height\":520 }"
				" ,\"fill\": [255,0,0]"
				" ,\"containers\": ["
				"  {\"area\": {\"x\":  0,  \"y\":0, \"width\": 127, \"height\":190},\"asset\": \"picture1\"},"
				"  {\"area\": {\"x\":127,  \"y\":0, \"width\": 127, \"height\":190},\"asset\": \"picture2\",},"
				"  {\"area\": {\"x\":254,  \"y\":0, \"width\": 127, \"height\":190},\"asset\": \"picture3\", },"
				"  {\"area\": {\"x\":381,  \"y\":0, \"width\": 127, \"height\":190},\"asset\": \"picture4\", },"
				"  {\"area\": {\"x\":508,  \"y\":0, \"width\":127, \"height\":190},\"asset\": \"picture5\", },"
				"  {\"area\": {\"x\":635,  \"y\":0, \"width\":127, \"height\":190},\"asset\": \"picture6\", },"
				"  {\"area\": {\"x\":762,  \"y\":0, \"width\":127, \"height\":190},\"asset\": \"picture7\", },"
				"  {\"area\": {\"x\":889,  \"y\":0, \"width\":127, \"height\":190},\"asset\": \"picture8\", },"
				"  {\"area\": {\"x\":1016, \"y\":0, \"width\":127, \"height\":190},\"asset\":\"picture9\", },"
				"  {\"area\": {\"x\":1143, \"y\":0, \"width\":127, \"height\":190},\"asset\":\"picture10\", },"
				"  {\"area\": {\"x\":1270, \"y\":0, \"width\":127, \"height\":190},\"asset\":\"picture1\", }"
					"]"
			  	"} "
		   "]"
		 ", \"assets\": ["
			  "{\"label\": \"picture1\", \"url\": \"res/Lesson5/Vice2018.jpg\"}"
			  ",{\"label\": \"picture2\", \"url\": \"res/Lesson5/BB_Online_Dom_Payoff_1-Sheet_H-Steinfeld_BB_Bridge_Autobot.jpg\"}"
			  ",{\"label\": \"picture3\", \"url\": \"res/Lesson5/GRC_Tsr1Sheet_GrinchAndMax_.jpg\"}"
			  ",{\"label\": \"picture4\", \"url\": \"res/Lesson5/MULE_VERT_MAIN_DOM_2764x4096_master.jpg\"}"
			  ",{\"label\": \"picture5\", \"url\": \"res/Lesson5/TheFavourite2018.jpg\"}"
			  ",{\"label\": \"picture6\", \"url\": \"res/Lesson5/SecondAct_27x40_1Sheet_RGB.jpg\"}"
			  ",{\"label\": \"picture7\", \"url\": \"res/Lesson5/AQAMN_VERT_MAIN_DUO_DOM_2764x4096_master.jpg\"}"
			  ",{\"label\": \"picture8\", \"url\": \"res/Lesson5/TSNGO_TicketingBanner_250x375_r2.jpg\"}"
			  ",{\"label\": \"picture9\", \"url\": \"res/Lesson5/HolmesAndWatson2018.jpg\"}"
			  ",{\"label\": \"picture10\", \"url\": \"res/Lesson5/SpiderManIntoTheSpiderVerse2018.jpg\"}"
			  ",{\"label\": \"picture11\", \"url\": \"res/Lesson5/WTM_HeroPoster.jpg\"}"
		      ",{\"label\": \"picture12\", \"url\": \"res/Lesson5/MQOS_OneSheet.jpg\"}"
		      ",{\"label\": \"picture13\", \"url\": \"res/Lesson5/MPR-Payoff_1-Sheet_v8a_Sm.jpg\"}"
		   "]"
		"}";


int main(int argc, char** argv)
{
	char path[2048];

	shmwindow* win = shmwindow::create();
	extern SHMEngine* engine;
	bool testing=false;
	int filearg = 1;
	readlink("/proc/self/exe",path, sizeof(path));
	char* sl = strrchr(path, '/');
	sl[1] = 0;
	try {
		if (argc==1)
		{
			SceneBuilder builder;
			builder.parse_containers_from_string(path, base_scene, engine);
			scene.containers = builder.nc;
			scene.assets = builder.na;
			//return 0;
		}
		else if (argc>=2)
		{
			SceneBuilder builder;
			if (std::string(argv[1])=="--testing"){
				testing = true;
				filearg = 2;
			}
			builder.parse_containers(path, argv[filearg], engine);
			scene.containers = builder.nc;
			scene.assets = builder.na;
		}
	} catch (const char* ex) {
		cout << ex << endl;
		return 1;
	}
	win->configure(Rect((1920-1280)/2,(1080-720)/2,1280,720));

	if (testing)
		draw_scene2(scene,engine);
	else
		draw_scene(scene,engine);
	win->repaint();


	XCBWindow::eventLoop();
	return 0;
}
