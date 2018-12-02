/*
 * XCBWindow.h
 *
 *  Created on: Oct 20, 2012
 *      Author: Mike
 */

#ifndef XCBWINDOW_H_
#define XCBWINDOW_H_

#include <xcb/xcb.h>
#include "xcb/xcb_keysyms.h"

#include <set>
#include <string>
namespace WebKit {
	class WebPageProxy;
}

class Size {
public:
	Size(int w, int h) : m_width(w), m_height(h) {}
	int m_width, m_height;
	int width() const { return m_width; }
	int height() const { return m_height; }
};
class Rect {
public:
	Rect(int x, int y, int width, int height)
	: m_x(x), m_y(y), m_width(width), m_height(height) {}
	int m_x, m_y, m_width, m_height;
	int x() const { return m_x; }
	int y() const { return m_y; }
	int width() const { return m_width; }
	int height() const { return m_height; }
};
class KeyDescriptor {
public:
	explicit KeyDescriptor(uint32_t unicode) : unicode(unicode)
	{
		shiftMode=controlMode=altMode=lockMode=numlockMode=0;
		scan = 0;
	}
	explicit KeyDescriptor(uint32_t unicode, uint32_t scan) : unicode(unicode), scan(scan)
	{
		shiftMode=controlMode=altMode=lockMode=numlockMode=0;
	}
	uint32_t code() const { return unicode; }
	void setShift(bool v) { shiftMode=v; }
	void setNumlock(bool v) { numlockMode=v; }
	void setControl(bool v) { controlMode=v; }
	void setAlt(bool v) { altMode=v; }
	void setLock(bool v) { lockMode=v; }
	bool shift() const { return shiftMode!=0; }
	bool control() const { return controlMode!=0; }
	bool alt() const { return altMode!=0; }
	bool lock() const { return lockMode!=0; }
	bool numlock() const { return numlockMode!=0; }
	std::string toString() const;
private:
	uint32_t unicode;
	uint32_t scan;
	unsigned shiftMode:1;
	unsigned controlMode:1;
	unsigned altMode:1;
	unsigned lockMode:1;
	unsigned numlockMode:1;
};
class XCBWindow
{
public:
	static XCBWindow* create(const std::string& title);
	virtual ~XCBWindow();
	void configure(const Rect& dest);
	void clear_area(const Rect& area);
	virtual void onExpose(int x, int y, int width, int height, int count);
	void onMouseUp(xcb_button_release_event_t *ev);
	void onMouseDown(xcb_button_press_event_t *ev);
	virtual void onKeyRelease(const KeyDescriptor& key);
	virtual void onKeyPress(const KeyDescriptor& key);
	void onMouseMove(xcb_motion_notify_event_t* ev);
	virtual void onConfigure(int16_t x, int16_t y, int16_t width, int16_t height);
	Size getSize() const;
	static xcb_connection_t* xcb_connection;
	static bool doOneEvent(xcb_generic_event_t* event);
	static void eventLoop();
	static bool pollEvent();
	static int event_fd();
	static void getKeyTranslations(xcb_keysym_t syms[], const xcb_key_press_event_t& ev);
protected:
	explicit XCBWindow(xcb_window_t window);
	static xcb_window_t createXcbWindow(const std::string& title, const Rect& rect);
	static xcb_screen_t* xcb_screen;
	xcb_window_t window;
private:
	static XCBWindow* matchEvent(xcb_generic_event_t* event);
	static void basics();
	static KeyDescriptor keyRigamarole(xcb_key_press_event_t *ev,const char* edge);
	static std::set<XCBWindow*> windows;
	static const xcb_setup_t* setup;
	static xcb_key_symbols_t* keysymbols;
	static xcb_atom_t delete_atom;
	static xcb_atom_t protocols_atom;
};

#endif /* XCBWINDOW_H_ */
