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
private:
	bitwindow(const Rect& rect);
};

#endif /* BITWINDOW_H_ */
