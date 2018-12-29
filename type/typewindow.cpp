/*
 * typewindow.cpp
 *
 *  Created on: Dec 1, 2018
 *      Author: menright
 */

#include "typewindow.h"
#include <ft2build.h>
#include FT_FREETYPE_H


#include <iostream>
#include <cstdlib> // for exit
#include <cstring>

using namespace std;

uint8_t* Mem;
unsigned bytes;

typewindow::typewindow(const Rect& rect)
: XCBWindow(createXcbWindow("typewindow", rect))
{
}
typewindow* typewindow::create()
{
	Mem = new uint8_t[(1280*720*4)];
	bytes = 1280*720*4;
	uint32_t* pixels = (uint32_t*)Mem;
	memset(Mem, 0xFF, bytes);
	typewindow* w = new typewindow(Rect((1920-1280)/2,(1080-720)/2,1280,720));
	return w;
}
void my_draw_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y)
{
	FT_Int i, j, p, q;
	FT_Int x_max = x + bitmap->width;
	FT_Int y_max = y + bitmap->rows;
	for (i = x, p = 0; i<x_max; i++, p++)
	{
		for (j = y, q = 0; j<y_max; j++, q++)
		{
			if (i < 0 || j < 0 ||
				i >= 1280 || j >= 720)
				continue;
			((uint32_t*)Mem)[j*1280+i] = (255-bitmap->buffer[q * bitmap->width + p])*0x010101+0xFF000000;
		}
	}
}


void typewindow::repaint()
{
	FT_Library library;
	FT_Face face;
	int fterror = FT_Init_FreeType(&library);
	if (fterror)
	{
		puts("freetype library");
		exit(1);
	}
	fterror = FT_New_Face(library, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0, &face);
	if (fterror == FT_Err_Unknown_File_Format)
	{
		puts("Font not valid");
		exit(1);
	}
	else if (fterror)
	{
		printf("FreeType error %d\n", fterror);
		exit(1);
	}
	else if (face == NULL)
	{
		printf("FreeType face is null but no error?\n");
		exit(1);
	}
	fterror = FT_Set_Pixel_Sizes(face, 0, 32); //16 pixels high
	FT_GlyphSlot  slot = face->glyph;  /* a small shortcut */
	int           pen_x, pen_y, n;
	 FT_UInt  glyph_index;

	 pen_x = 0;
	 pen_y = 32;

	for (auto ch : text)
	{
	  /* retrieve glyph index from character code */
	  glyph_index = FT_Get_Char_Index( face, ch );

	  /* load glyph image into the slot (erase previous one) */
	  fterror = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
	  if ( fterror )
	    continue;  /* ignore errors */

	  /* convert to an anti-aliased bitmap */
	  fterror = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
	  if ( fterror )
	    continue;

	  /* now, draw to our target surface */
	  my_draw_bitmap( &slot->bitmap,
	                  pen_x + slot->bitmap_left,
	                  pen_y - slot->bitmap_top );

	  /* increment pen position */
	  pen_x += slot->advance.x >> 6;
	  pen_y += slot->advance.y >> 6; /* not useful for now */
	}
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
void typewindow::onExpose(int x,int y,int width,int height,int count)
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
void typewindow::set_text(const std::string& t)
{
	text = t;
}
