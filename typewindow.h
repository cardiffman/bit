/*
 * typewindow.h
 *
 *  Created on: Dec 8, 2018
 *      Author: menright
 */

#ifndef TYPEWINDOW_H_
#define TYPEWINDOW_H_

#include "XCBWindow.h"

class typewindow : public XCBWindow
{
public:
	static typewindow* create();
	void onExpose(int x, int y, int width, int height, int count);
	void repaint();
	void set_text(const std::string& t);
	void onConfigure(int16_t, int16_t, int16_t, int16_t) {}
private:
	typewindow(const Rect& rect);
	std::string text;
};

#endif /* TYPEWINDOW_H_ */
