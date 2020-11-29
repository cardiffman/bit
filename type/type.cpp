/*
 * type.cpp
 *
 *  Created on: Dec 8, 2018
 *      Author: menright
 */

#include "typewindow.h"
#include <fstream>
#include <iostream>
#include <poll.h>
#include <vector>
#include <unistd.h>
#include <climits>

using std::cin;
using std::cout;
using std::vector;
using std::cerr;
using std::endl;

int main(int argc, char** argv)
{
	typewindow* win = typewindow::create();
	win->configure(Rect((1920-1280)/2,(1080-720)/2,1280,720));
	win->set_text("This is a random piece of text to draw using FreeType");
	win->repaint();
	XCBWindow::eventLoop();
}

