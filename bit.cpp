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

using std::cin;
using std::cout;

int main(int, char**argv)
{
	auto file = std::fstream(argv[1], std::ios::in | std::ios::binary);
	Json::Value root;
	file >> root;
	cout << root;

	bitwindow* win = bitwindow::create();
	win->configure(Rect(360,128,1280,720));
	XCBWindow::eventLoop();
	return 0;
}


