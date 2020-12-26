/*
 * bitwindow.h
 *
 *  Created on: Dec 1, 2018
 *      Author: menright
 */

#ifndef BITWINDOW_H_
#define BITWINDOW_H_

#include "XCBWindow.h"

class GraphicsEngine;

class bitwindow : public XCBWindow
{
public:
	static bitwindow* create(GraphicsEngine*);
	void onExpose(int x, int y, int width, int height, int count);
	void repaint();
private:
	bitwindow(const Rect& rect);
	GraphicsEngine* engine;
};

#endif /* BITWINDOW_H_ */
