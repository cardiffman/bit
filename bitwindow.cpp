/*
 * bitwindow.cpp
 *
 *  Created on: Dec 1, 2018
 *      Author: menright
 */

#include "bitwindow.h"

#include <iostream>
#include <cstdlib> // for exit

using namespace std;

uint8_t* Mem;
unsigned bytes;

bitwindow::bitwindow(const Rect& rect)
: XCBWindow(createXcbWindow("bitwindow", rect))
{
}
bitwindow* bitwindow::create()
{
	Mem = new uint8_t[(1280*720*4)];
	bytes = 1280*720*4;
	uint32_t* pixels = (uint32_t*)Mem;
	for (unsigned y = 0; y<720; y++)
		for (unsigned x = 0; x<1280; x++)
			pixels[y*1280+x] = ((x^y)&1)?0xFFFFFFFF:0xFF000000;
	bitwindow* w = new bitwindow(Rect(360,120,1280,720));
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
	xcb_void_cookie_t chk = xcb_put_image_checked(xcb_connection,
			XCB_IMAGE_FORMAT_Z_PIXMAP, window, gcontext, (uint16_t)1280/*width*/, (uint16_t)720/*height*/, 0,
			0, 0, XCBWindow::xcb_screen->root_depth, bytes, Mem);
	xcb_flush(xcb_connection);
	//cerr << "Image put, checking\n";
	/*xcb_generic_error_t**/ error = xcb_request_check(xcb_connection, chk);
	if (error)
	{
		cerr << "ERROR: can't put_image :"<<
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
#if 0
	//xcb_rectangle_t rect; rect.x = x; rect.y=y;rect.width=width; rect.height=height;
	//xcb_poly_rectangle(xcb_connection, window, gcontext, 1, &rect);
	xcb_segment_t segs[2] = {
			{	(int16_t)x, (int16_t)y, (int16_t)(x+width), (int16_t)(y+height) },
			{   (int16_t)x, (int16_t)(y+height), (int16_t)(x+width), (int16_t)y }
	};
	xcb_poly_segment(xcb_connection, window, gcontext, 2, segs);
	//xcb_free_gc(xcb_connection, gcontext);
	xcb_flush(xcb_connection);
	clog << __LINE__ << endl;
#endif
	xcb_free_gc(xcb_connection, gcontext);
	repaint();

}
