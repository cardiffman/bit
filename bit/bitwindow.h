/*
 * bitwindow.h
 *
 *  Created on: Dec 1, 2018
 *      Author: menright
 */

#ifndef BITWINDOW_H_
#define BITWINDOW_H_

#include "XCBWindow.h"

class bitwindow : public XCBWindow
{
public:
	static bitwindow* create();
	void onExpose(int x, int y, int width, int height, int count);
	void repaint();
private:
	bitwindow(const Rect& rect);
};
struct Area {
	Area() : x(), y(), width(), height() {}
	Area(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
	int x; int y; int width; int height;
};
struct RectSize {
	RectSize() : width(), height() {}
	RectSize(int width, int height) : width(width), height(height) {}
	int width; int height;
};
struct BitBuffer {
	uint8_t* mem; RectSize dims; unsigned rowbytes;
};


#endif /* BITWINDOW_H_ */
