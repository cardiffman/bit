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

using std::cin;
using std::cout;

int main(int, char**argv)
{
	auto file = std::fstream(argv[1], std::ios::in | std::ios::binary);
	Json::Value root;
	file >> root;
	cout << root << std::endl;

	bitwindow* win = bitwindow::create();
	win->configure(Rect(360,128,1280,720));
	//XCBWindow::eventLoop();
	pollfd pollfds[1] = { { XCBWindow::event_fd(), POLLIN, 0} };
	while (true)
	{
		int p = poll(pollfds, 1, 0);
		if (p == 1) {
			if (pollfds[0].revents == POLLIN) {
				if (!XCBWindow::pollEvent())
					break;
			}
		}
	}
	return 0;
}


