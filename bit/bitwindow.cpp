/*
 * bitwindow.cpp
 *
 *  Created on: Dec 1, 2018
 *      Author: menright
 */

#include "bitwindow.h"
#include "engine.h"

#include <iostream>
#include <cstdlib> // for exit

using namespace std;

bitwindow::bitwindow(const Rect& rect)
: XCBWindow(createXcbWindow("bitwindow", rect))
{
}
bitwindow* bitwindow::create(GraphicsEngine* engine_)
{
	auto screen = engine_->getScreenBuffer();
	engine_->fill(screen, Area(0,0,1280,720), 0xFFEECCDD);
	bitwindow* w = new bitwindow(Rect((1920-1280)/2,(1080-720)/2,1280,720));
	w->engine = engine_;
	return w;
}
void bitwindow::repaint()
{
	xcb_gcontext_t gcontext = xcb_generate_id(xcb_connection);
	uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND; // | XCB_GC_FONT;
	uint32_t value_list[3];
	value_list[0] = xcb_screen->black_pixel;
	value_list[1] = xcb_screen->white_pixel;
	//value_list[2] = font;
	xcb_void_cookie_t cookie_gc = xcb_create_gc_checked(xcb_connection,
			gcontext, window, mask, value_list);
	xcb_generic_error_t* error = xcb_request_check(xcb_connection, cookie_gc);
	uint8_t* ptr;
	uint32_t pitch;
	auto sc = engine->getScreenBuffer();
	sc->lock(ptr, pitch);
	xcb_void_cookie_t chk = xcb_put_image_checked(xcb_connection,
			XCB_IMAGE_FORMAT_Z_PIXMAP, window, gcontext, (uint16_t)1280/*width*/, (uint16_t)720/*height*/, 0,
			0, 0, XCBWindow::xcb_screen->root_depth, pitch*sc->dims.height, ptr);
	sc->unlock();
	xcb_flush(xcb_connection);
	//cerr << "Image put, checking\n";
	/*xcb_generic_error_t**/ error = xcb_request_check(xcb_connection, chk);
	if (error)
	{
		cerr << __FUNCTION__ << ": ERROR: can't put_image :"<<
				unsigned(error->error_code) << endl;
		xcb_disconnect(xcb_connection);
		exit(-1);
	}
	xcb_free_gc(xcb_connection, gcontext);
}
void bitwindow::onExpose(int x,int y,int width,int height,int count)
{
	xcb_gcontext_t gcontext = xcb_generate_id(xcb_connection);
	uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND; // | XCB_GC_FONT;
	uint32_t value_list[3];
	value_list[0] = xcb_screen->black_pixel;
	value_list[1] = xcb_screen->white_pixel;
	//value_list[2] = font;
	xcb_void_cookie_t cookie_gc = xcb_create_gc_checked(xcb_connection,
			gcontext, window, mask, value_list);
	xcb_generic_error_t* error = xcb_request_check(xcb_connection, cookie_gc);
	if (error)
	{
		cerr << "ERROR: can't create gc : " << unsigned(error->error_code) << endl;
		xcb_disconnect (xcb_connection);
		exit(-1);
	}
	xcb_free_gc(xcb_connection, gcontext);
	repaint();

}
