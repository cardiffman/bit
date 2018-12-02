/*
 * XCBWindow.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Mike
 */
#include "XCBWindow.h"
#include "cairo/cairo.h"

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <iomanip>

using std::cerr;
using std::clog;
using std::endl;
using std::ostream;
using std::string;
using std::setfill;
using std::setw;
using std::hex;
using std::dec;
using std::ostringstream;

xcb_screen_t* XCBWindow::xcb_screen=0;
xcb_connection_t* XCBWindow::xcb_connection=0;
std::set<XCBWindow*> XCBWindow::windows;
xcb_atom_t XCBWindow::protocols_atom = 0;
xcb_atom_t XCBWindow::delete_atom = 0;
const xcb_setup_t* XCBWindow::setup = 0;
xcb_key_symbols_t* XCBWindow::keysymbols = 0;

std::string KeyDescriptor::toString() const {
	std::ostringstream os;
	os << '[' << hex << setw(4) << setfill('0')<< unicode << " scan " << scan << setw(0)<< dec << " shift " << shiftMode << " control " << controlMode << ']';
	return os.str();
}

XCBWindow::XCBWindow(xcb_window_t window)
: window(window)
{
	windows.insert(this);
}

XCBWindow::~XCBWindow()
{
}
void XCBWindow::clear_area(const Rect& area)
{
	xcb_clear_area(xcb_connection, 1, window, (uint16_t)area.x(), (uint16_t)area.y(), (uint16_t)area.width(), (uint16_t)area.height());
	xcb_flush(xcb_connection);
}

void XCBWindow::onExpose(int x, int y, int width, int height, int count)
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
	//xcb_rectangle_t rect; rect.x = x; rect.y=y;rect.width=width; rect.height=height;
	//xcb_poly_rectangle(xcb_connection, window, gcontext, 1, &rect);
	xcb_segment_t segs[2] = {
			{	(int16_t)x, (int16_t)y, (int16_t)(x+width), (int16_t)(y+height) },
			{   (int16_t)x, (int16_t)(y+height), (int16_t)(x+width), (int16_t)y }
	};
	xcb_poly_segment(xcb_connection, window, gcontext, 2, segs);
	//xcb_free_gc(xcb_connection, gcontext);
	xcb_flush(xcb_connection);
	clog << __PRETTY_FUNCTION__ << " AREA " << x << ' ' << y << ' ' << width << ' ' << height << endl;
}

void XCBWindow::onMouseMove(xcb_motion_notify_event_t* event)
{
}

void XCBWindow::onMouseDown(xcb_button_press_event_t* event)
{
}

void XCBWindow::onMouseUp(xcb_button_release_event_t* event)
{
}
void XCBWindow::onKeyRelease(const KeyDescriptor& key)
{
	clog << __PRETTY_FUNCTION__ << ' '<< key.toString() << endl;
}
void XCBWindow::onKeyPress(const KeyDescriptor& key)
{
	clog << __PRETTY_FUNCTION__ << ' '<< key.toString() << endl;
}

void XCBWindow::onConfigure(int16_t x, int16_t y, int16_t width, int16_t height)
{
	clog << __PRETTY_FUNCTION__ << ' ' << this << endl;
}

Size XCBWindow::getSize() const
{
    xcb_get_geometry_cookie_t geo_cookie = xcb_get_geometry (xcb_connection,
                                                window);

    xcb_generic_error_t* error;
    xcb_get_geometry_reply_t * geo = xcb_get_geometry_reply (xcb_connection,
                                                      geo_cookie,
                                                      &error);
	if (error)
	{
		cerr << "ERROR: can't create gc : " << unsigned(error->error_code) << endl;
		xcb_disconnect (xcb_connection);
		exit(-1);
	}
	Size size(geo->width, geo->height);
	free(geo);
	return size;
}
XCBWindow* XCBWindow::matchEvent(xcb_generic_event_t* event)
{
    xcb_window_t win = 0;
	switch (event->response_type & ~0x80)
	{
	case XCB_KEY_PRESS:
	case XCB_KEY_RELEASE:
		win = ((xcb_key_press_event_t*)event)->event;
		break;
	case XCB_BUTTON_PRESS:
	case XCB_BUTTON_RELEASE:
		win = ((xcb_button_press_event_t*)event)->event;
		break;
	case XCB_MOTION_NOTIFY:
		win = ((xcb_motion_notify_event_t*)event)->event;
		break;
	case XCB_ENTER_NOTIFY:
	case XCB_LEAVE_NOTIFY:
		win = ((xcb_enter_notify_event_t*)event)->event;
		break;
	case XCB_FOCUS_IN:
	case XCB_FOCUS_OUT:
		win = ((xcb_focus_in_event_t*)event)->event;
		break;
	case XCB_EXPOSE:
		win = ((xcb_expose_event_t*)event)->window;
		break;
	case XCB_GRAPHICS_EXPOSURE:
		win = ((xcb_graphics_exposure_event_t*)event)->drawable;
		break;
	case XCB_NO_EXPOSURE:
		win = ((xcb_no_exposure_event_t*)event)->drawable;
		break;
	case XCB_VISIBILITY_NOTIFY:
		win = ((xcb_visibility_notify_event_t*)event)->window;
		break;
	case XCB_CREATE_NOTIFY:
		win = ((xcb_create_notify_event_t*)event)->window;
		break;
	case XCB_DESTROY_NOTIFY:
		win = ((xcb_destroy_notify_event_t*)event)->window;
		break;
	case XCB_UNMAP_NOTIFY:
		win = ((xcb_unmap_notify_event_t*)event)->window; // or should it be 'event'?
		break;
	case XCB_MAP_NOTIFY:
		win = ((xcb_map_notify_event_t*)event)->window;
		break;
	case XCB_MAP_REQUEST:
		win = ((xcb_map_request_event_t*)event)->window;
		break;
	case XCB_REPARENT_NOTIFY:
		win = ((xcb_reparent_notify_event_t*)event)->window;
		break;
	case XCB_CONFIGURE_NOTIFY:
		win = ((xcb_configure_notify_event_t*)event)->window;
		break;
	case XCB_CONFIGURE_REQUEST:
		win = ((xcb_configure_request_event_t*)event)->window;
		break;
	case XCB_GRAVITY_NOTIFY:
		win = ((xcb_gravity_notify_event_t*)event)->window;
		break;
	case XCB_RESIZE_REQUEST:
		win = ((xcb_resize_request_event_t*)event)->window;
		break;
	case XCB_CIRCULATE_NOTIFY:
		win = ((xcb_circulate_notify_event_t*)event)->window;
		break;
	case XCB_PROPERTY_NOTIFY:
		win = ((xcb_property_notify_event_t*)event)->window;
		break;
	case XCB_SELECTION_CLEAR:
		win = ((xcb_selection_clear_event_t*)event)->owner;
		break;
	case XCB_SELECTION_REQUEST:
		win = ((xcb_selection_request_event_t*)event)->owner;
		break;
	case XCB_SELECTION_NOTIFY:
		win = ((xcb_selection_notify_event_t*)event)->requestor;
		break;
	case XCB_COLORMAP_NOTIFY:
		win = ((xcb_colormap_notify_event_t*)event)->window;
		break;
	case XCB_CLIENT_MESSAGE:
		win = ((xcb_client_message_event_t*)event)->window;
		break;
#if 0 // doesn't involve a window directly
	case XCB_MAPPING_NOTIFY:
		win = ((xcb_mapping_notify_event_t*)event)->window;
		break;
#endif
	}
    if (win != 0)
    {
        for (std::set<XCBWindow*>::iterator pW = windows.begin(); pW != windows.end(); ++pW)
        {
            if ((*pW)->window == win)
                return *pW;
        }
    }
    return 0;
}
#undef EVENT_NOISE
#undef NOISY_KEYS
#ifdef NOISY_KEYS
static void
print_modifiers (ostream& os, uint32_t mask)
{
  const char **mod, *mods[] = {
    "Shift", "Lock", "Ctrl", "Alt",
    "Mod2", "Mod3", "Mod4", "Mod5",
    "Button1", "Button2", "Button3", "Button4", "Button5"
  };
  os << "Modifier mask: ";
  for (mod = mods ; mask; mask >>= 1, mod++)
    if (mask & 1)
      os << (*mod) << ' ';
}
#endif

extern "C" void showKeySyms(xcb_key_symbols_t* syms)
{
#ifdef EVENT_NOISE
    clog << setw(4) << setfill('0') << hex;
    for (unsigned i=8; i<256; ++i)
    {
  	  xcb_keysym_t actual = xcb_key_symbols_get_keysym(syms, i, 0);
  	  xcb_keysym_t actual1 = xcb_key_symbols_get_keysym(syms, i, 1);
  	  xcb_keysym_t actual2 = xcb_key_symbols_get_keysym(syms, i, 2);
  	  xcb_keysym_t actual3 = xcb_key_symbols_get_keysym(syms, i, 3);
  	  xcb_keysym_t actual4 = xcb_key_symbols_get_keysym(syms, i, 4);
  	  xcb_keysym_t actual5 = xcb_key_symbols_get_keysym(syms, i, 5);
  	  clog << "KeySyms for key " << unsigned(i)<< ':' << "U+"<< actual << ',' << "U+"<< actual1 << ',' << "U+"<< actual2 << ',' << "U+"<< actual3 << ',' << "U+"<< actual4 << ',' << "U+"<< actual5 << endl;
    }
    clog << setw(0) << dec;
#endif
}
string utf8of(xcb_keysym_t actual)
{
	string r;
	if (actual < 128)
		r.push_back((char)actual);
	else if (actual < (1<<11)) {
		r.push_back((actual>>6) + 0xC0);
		r.push_back((actual & 0x3F) + 0x80);
	} else if (actual < (1<<16)) {
		r.push_back((actual>>12) + 0xE0);
		r.push_back(((actual>>6) & 0x3F) + 0x80);
		r.push_back((actual & 0x3F) + 0x80);
	} else if (actual < (1<<21)) {
		r.push_back((actual>>18) + 0xF0);
		r.push_back(((actual>>12) & 0x3F) + 0x80);
		r.push_back(((actual>>6) & 0x3F) + 0x80);
		r.push_back((actual & 0x3F) + 0x80);
	} else if (actual < (1<<26)) {
		r.push_back((actual>>24) + 0xF8);
		r.push_back(((actual>>18) & 0x3F) + 0x80);
		r.push_back(((actual>>12) & 0x3F) + 0x80);
		r.push_back(((actual>>6) & 0x3F) + 0x80);
		r.push_back((actual & 0x3F) + 0x80);
	} else if (actual < (uint32_t)(1<<31)) {
		r.push_back((actual>>30) + 0xFC);
		r.push_back(((actual>>24) & 0x3F) + 0x80);
		r.push_back(((actual>>18) & 0x3F) + 0x80);
		r.push_back(((actual>>12) & 0x3F) + 0x80);
		r.push_back(((actual>>6) & 0x3F) + 0x80);
		r.push_back((actual & 0x3F) + 0x80);
	} else {
		return utf8of(0xDC58);
	}
	return r;
}
KeyDescriptor XCBWindow::keyRigamarole(xcb_key_press_event_t *ev,const char* edge)
{
#ifdef NOISY_KEYS
	ostringstream v; ostringstream u;
	print_modifiers(clog, ev->state);
#endif
	xcb_keysym_t actual = xcb_key_symbols_get_keysym(XCBWindow::keysymbols, ev->detail, 0);
	xcb_keysym_t actual1 = xcb_key_symbols_get_keysym(XCBWindow::keysymbols, ev->detail, 1);
#ifdef NOISY_KEYS
	xcb_keysym_t actual2 = xcb_key_symbols_get_keysym(keysymbols, ev->detail, 2);
	xcb_keysym_t actual3 = xcb_key_symbols_get_keysym(keysymbols, ev->detail, 3);
	xcb_keysym_t actual4 = xcb_key_symbols_get_keysym(keysymbols, ev->detail, 4);
	xcb_keysym_t actual5 = xcb_key_symbols_get_keysym(keysymbols, ev->detail, 5);
#endif
	xcb_keycode_t* key_code = xcb_key_symbols_get_keycode(keysymbols, actual);
	//xcb_keycode_t* key_code1 = xcb_key_symbols_get_keycode(keysymbols, actual1);
	uint32_t whichCode;
	switch (ev->state & (XCB_KEY_BUT_MASK_SHIFT|XCB_KEY_BUT_MASK_LOCK))
	{
	case XCB_KEY_BUT_MASK_SHIFT:
	case XCB_KEY_BUT_MASK_LOCK:
#ifdef NOISY_KEYS
		v << (char)actual1;
		u << "U+" << setw(4) << setfill('0')<< hex << actual1;
#endif
		whichCode = actual1;
		break;
	default://case XCB_KEY_BUT_MASK_SHIFT|XCB_KEY_BUT_MASK_LOCK:
#ifdef NOISY_KEYS
		v << (char)actual;
		u << "utf8:" << utf8of(actual)<<' ';
		u << "U+" << setw(4) << setfill('0')<< hex << actual;
#endif
		whichCode = actual;
		break;
	}
#ifdef NOISY_KEYS
	//window = XCBWindow::matchEvent(event);
	clog << "Key "<<edge<<" in window "<< ev->event << ": " << v.str() << '/'<< u.str();
	//clog << " 'keycode': " << key_code<< ' '<< " 'keycode1': " << key_code1;
	clog << ' ' << hex << setw(4) << setfill('0') << unsigned(ev->detail) <<',' << actual << ',' << actual1 << ',' << actual2 << ',' << actual3 <<  ',' << actual4 << ',' << actual5 << dec << setw(0)<< endl;
#endif
	KeyDescriptor retval(whichCode, ev->detail);
	if (ev->state & 1) retval.setShift(true);
	if (ev->state & 2) retval.setLock(true);
	if (ev->state & 4) retval.setControl(true);
	if (ev->state & 8) retval.setAlt(true);
	if (ev->state & 16) retval.setNumlock(true);
	free(key_code);
	return retval;
}
void XCBWindow::eventLoop()
{
	while (1)
	{
		xcb_generic_event_t  *event;
		event = xcb_wait_for_event (XCBWindow::xcb_connection);
		bool more = XCBWindow::doOneEvent(event);
		free(event);
		if (!more)
			break;
	}
}
bool XCBWindow::doOneEvent(xcb_generic_event_t* event)
{
	XCBWindow* window;
	xcb_expose_event_t* expose;
	switch (event->response_type & ~0x80)
	{
	case XCB_CLIENT_MESSAGE: {
		clog << "XCB_CLIENT_MESSAGE" << endl;
		if((*(xcb_client_message_event_t*)event).data.data32[0] == XCBWindow::delete_atom)
		{
			clog << "WM_DELETE_WINDOW" << endl;
			window = XCBWindow::matchEvent(event);
			clog << "Matched " << window << endl;
			windows.erase(window);
			//delete window;
			//clog << "deleted " << endl;
			//free(event);
			//clog << "freed " << endl;
			if (windows.size() == 0)
			{
				xcb_disconnect(XCBWindow::xcb_connection);
				clog << "disconnected" << endl;
			}
			return windows.size() > 0;
		}
	}
    break;
    case XCB_EXPOSE:
    	expose = (xcb_expose_event_t*)event;
#ifdef EVENT_NOISE
    	{ ostringstream os;
    	os << "Xcb expose event x "<< expose->x <<" y " << expose->y << " width " << expose->width << " height " << expose->height << endl;
    	clog << os.str() << flush; }
#endif
    	window = XCBWindow::matchEvent(event);
    	if (window != 0)
    	{
    		window->onExpose(expose->x, expose->y, expose->width, expose->height, expose->count);
    	}
    	break;
    case XCB_ENTER_NOTIFY: {
#ifdef EVENT_NOISE
    	ostringstream os;
    	xcb_enter_notify_event_t *ev = (xcb_enter_notify_event_t *)event;
    	window = XCBWindow::matchEvent(event);
    	os << "Mouse entered window " << window << ", at coordinates ("<< ev->event_x<<","<<ev->event_y << ")" << endl;
    	clog << os.str() << flush;
#endif
    	break;
    }
    case XCB_LEAVE_NOTIFY: {
#ifdef EVENT_NOISE
    	xcb_leave_notify_event_t *ev = (xcb_leave_notify_event_t *)event;
    	ostringstream os;
    	window = XCBWindow::matchEvent(event);
    	os << "Mouse left window " << window << ", at coordinates ("<< ev->event_x<<","<<ev->event_y << ")" << endl;
    	clog << os.str() << flush;
#endif
    	break;
    }
    case XCB_KEY_PRESS: {
    	xcb_key_press_event_t *ev = (xcb_key_press_event_t *)event;
		KeyDescriptor key = keyRigamarole(ev,"pressed");
    	window = XCBWindow::matchEvent(event);
		window->onKeyPress(key);
    	break;
    }
    case XCB_KEY_RELEASE: {
    	xcb_key_release_event_t *ev = (xcb_key_release_event_t *)event;
		KeyDescriptor key = keyRigamarole((xcb_key_press_event_t*)ev,"released");
    	window = XCBWindow::matchEvent(event);
		window->onKeyRelease(key);
    	break;
    }
    case XCB_BUTTON_PRESS: {
    	xcb_button_press_event_t *ev = (xcb_button_press_event_t*)event;
#ifdef EVENT_NOISE
    	ostringstream os;
    	os << "XCB_BUTTON_PRESS " << ev->event_x << ',' << ev->event_y << ' ';
    	print_modifiers(os, ev->state); os << ' ';
    	os << int(ev->detail);//print_modifiers(os, ev->detail);
    	os << endl;
    	clog << os.str() << flush;
#endif
    	window = XCBWindow::matchEvent(event);
    	window->onMouseDown(ev);
    	break;
    }
    case XCB_BUTTON_RELEASE: {
    	xcb_button_release_event_t *ev = (xcb_button_release_event_t*)event;
#ifdef EVENT_NOISE
    	ostringstream os;
    	os << "XCB_BUTTON_RELEASE " << ev->event_x << ',' << ev->event_y << ' ';
    	print_modifiers(os, ev->state); os << ' ';
    	os << int(ev->detail);//print_modifiers(os, ev->detail);
    	os << endl;
    	clog << os.str() << flush;
#endif
    	window = XCBWindow::matchEvent(event);
    	window->onMouseUp(ev);
    	break;
    }
    case XCB_MOTION_NOTIFY: {
    	xcb_motion_notify_event_t* ev = (xcb_motion_notify_event_t*)event;
    	window = XCBWindow::matchEvent(event);
    	window->onMouseMove(ev);
    	break;
    }
    case XCB_FOCUS_IN:
#ifdef EVENT_NOISE
     clog << "XCB_FOCUS_IN" << endl;
#endif
     break;
    case XCB_FOCUS_OUT:
#ifdef EVENT_NOISE
     clog << "XCB_FOCUS_OUT" << endl;
#endif
     break;
    case XCB_VISIBILITY_NOTIFY:
#ifdef EVENT_NOISE
     clog << "XCB_VISIBILITY_NOTIFY" << endl;
#endif
     break;
    case XCB_MAP_NOTIFY:
#ifdef EVENT_NOISE
     clog << "XCB_MAP_NOTIFY" << endl;
#endif
     break;
    case XCB_MAP_REQUEST:
#ifdef EVENT_NOISE
     clog << "XCB_MAP_REQUEST" << endl;
#endif
     break;
    case XCB_REPARENT_NOTIFY:
#ifdef EVENT_NOISE
     clog << "XCB_REPARENT_NOTIFY" << endl;
#endif
     break;
    case XCB_CONFIGURE_NOTIFY: {
//#ifdef EVENT_NOISE
    	clog << "XCB_CONFIGURE_NOTIFY" << endl;
//#endif
    	window = XCBWindow::matchEvent(event);
    	xcb_configure_notify_event_t* ev = (xcb_configure_notify_event_t*)event;
    	clog << "x " << ev->x << " y " << ev->y << " w " << ev->width << " h " << ev->height << endl;
    	window->onConfigure(ev->x, ev->y, ev->width, ev->height);
    }
    break;
    case XCB_PROPERTY_NOTIFY:
#ifdef EVENT_NOISE
     clog << "XCB_PROPERTY_NOTIFY" << endl;
#endif
     break;
    default:
    	break;
    }
	return true;
}

void XCBWindow::basics()
{
	int xcb_screen_number;
    xcb_connection = xcb_connect(0, &xcb_screen_number);
    if (xcb_connection == 0)
    {
        cerr << "Can't make X connection" << endl;
        exit(1);
    }
    setup = xcb_get_setup(xcb_connection);
    if (setup == 0)
    {
        cerr << "Can't get X setup" << endl;
        xcb_disconnect(xcb_connection);
        exit(1);
    }
    xcb_screen = 0;
    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(setup);
    for (; screen_iter.rem != 0; --xcb_screen_number, xcb_screen_next(&screen_iter))
    {
        if (xcb_screen==0)
        {
        	xcb_screen = screen_iter.data;
            break;
        }
    }
    if (xcb_screen == 0)
    {
        cerr << "Can't get X screen" << endl;
        xcb_disconnect(xcb_connection);
        exit(1);
    }
    keysymbols = xcb_key_symbols_alloc(xcb_connection);
}

/* From icccm.c */
static void
xcb_set_wm_protocols(xcb_connection_t *c, xcb_atom_t wm_protocols,
                     xcb_window_t window, uint32_t list_len, xcb_atom_t *list)
{
#define ATOM ((xcb_atom_t)4)
  xcb_change_property(c, XCB_PROP_MODE_REPLACE, window, wm_protocols, ATOM, 32,
                      list_len, list);
}

static void StartXcbMonitor();
xcb_window_t XCBWindow::createXcbWindow(const string& title, const Rect& rect)
{
    if (!XCBWindow::xcb_connection)
        basics();
    xcb_window_t window = xcb_generate_id(XCBWindow::xcb_connection);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2];

    values[0] = XCBWindow::xcb_screen->white_pixel;
    values[1] =
        XCB_EVENT_MASK_KEY_PRESS |
        XCB_EVENT_MASK_KEY_RELEASE |
        XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE |
        XCB_EVENT_MASK_EXPOSURE |
        XCB_EVENT_MASK_ENTER_WINDOW |
        XCB_EVENT_MASK_LEAVE_WINDOW |
        XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_VISIBILITY_CHANGE |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY |
        XCB_EVENT_MASK_RESIZE_REDIRECT |
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
        XCB_EVENT_MASK_FOCUS_CHANGE |
        XCB_EVENT_MASK_PROPERTY_CHANGE |
        XCB_EVENT_MASK_COLOR_MAP_CHANGE |
        XCB_EVENT_MASK_OWNER_GRAB_BUTTON |
        0;
    ;
    xcb_void_cookie_t
    cookie_window = xcb_create_window_checked (XCBWindow::xcb_connection,
    		XCBWindow::xcb_screen->root_depth,
                                               window, XCBWindow::xcb_screen->root,
                                               rect.x(), rect.y(), rect.width(), rect.height(),
                                               0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                               XCBWindow::xcb_screen->root_visual,
                                               mask, values);
    xcb_void_cookie_t
    cookie_map = xcb_map_window_checked (XCBWindow::xcb_connection, window);

    /* error managing */
    xcb_generic_error_t*
    error = xcb_request_check (XCBWindow::xcb_connection, cookie_window);
    if (error) {
        cerr << "ERROR: can't create window : " << error->error_code << endl;
        xcb_disconnect (XCBWindow::xcb_connection);
        return -1;
    }
    error = xcb_request_check (XCBWindow::xcb_connection, cookie_map);
    if (error) {
        cerr << "ERROR: can't map window : " << error->error_code << endl;
        xcb_disconnect (XCBWindow::xcb_connection);
        return -1;
    }

    string title_icon = title;//"XCB target";
    /* Set the title of the window */
    xcb_change_property (XCBWindow::xcb_connection, XCB_PROP_MODE_REPLACE, window,
    		XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
    		title_icon.size(), title_icon.data());

    /* Set the title of the window icon */
    xcb_change_property (XCBWindow::xcb_connection, XCB_PROP_MODE_REPLACE, window,
    		XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
    		title_icon.size(), title_icon.data());


    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(XCBWindow::xcb_connection, 0, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(XCBWindow::xcb_connection, cookie, 0);
    XCBWindow::protocols_atom = reply->atom;
    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(XCBWindow::xcb_connection, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(XCBWindow::xcb_connection, cookie2, 0);
    XCBWindow::delete_atom = reply2->atom;
    /* Listen to X client messages in order to be able to pickup
 the "delete window" message that is generated for example
 when someone clicks the top-right X button within the window
 manager decoration (or when user hits ALT-F4). */
    xcb_set_wm_protocols (XCBWindow::xcb_connection, XCBWindow::protocols_atom, window, 1, &XCBWindow::delete_atom);

    xcb_flush(XCBWindow::xcb_connection);
    StartXcbMonitor();
    return window;
}

#undef LINK_WITH_G
#ifdef LINK_WITH_G
GPollFD xcb_fd;
GQueue* xcb_event_queue;
// "Prepare" function of event source
static gboolean xcb_gsource_prepare(GSource* gs, gint* timeout)
{
	*timeout = -1;
	// Return true if "ready to dispatch" (i.e. can skip check).
	return !g_queue_is_empty(xcb_event_queue);
}
// "Check" function of event source
static gboolean xcb_gsource_check(GSource* gs)
{
	if (xcb_fd.revents & G_IO_IN)
	{
		xcb_generic_event_t* event;
		if (xcb_connection_has_error(XCBWindow::xcb_connection))
		{
			return true;
		}
		while (true)
		{
			event = xcb_poll_for_event(XCBWindow::xcb_connection);
			if (event == NULL)
				break;
			g_queue_push_tail(xcb_event_queue, event);
		}
	}
	// return true if "ready to dispatch"
	return !g_queue_is_empty(xcb_event_queue);
}
// "Dispatch" function of event source
static gboolean xcb_gsource_dispatch(GSource* gs, GSourceFunc, gpointer)
{
	xcb_generic_event_t* event;
	gboolean ret = true;
	event = (xcb_generic_event_t*)g_queue_pop_head(xcb_event_queue);
	if (event)
	{
		XCBWindow::doOneEvent(event);
		free(event);
	}
	return ret;
}
// Function pointer table for event source based on XCB events
static GSourceFuncs xcb_source_funcs = {
		xcb_gsource_prepare,
		xcb_gsource_check,
		xcb_gsource_dispatch,
		NULL,0,0
};
void LinkXcbToGlibLoop()
{
    using WebCore::RunLoop;
	GSource* source = g_source_new(&xcb_source_funcs, sizeof(GSource));
	if (!source)
	{
		clog << __PRETTY_FUNCTION__ << " error making event source" << endl;
		exit(1);
	}
	xcb_fd.fd = xcb_get_file_descriptor(XCBWindow::xcb_connection);
	xcb_fd.events = G_IO_IN;
	g_source_add_poll(source, &xcb_fd);
	xcb_event_queue = g_queue_new();
	RunLoop::main()->attachEvents(source);
	// I believe I do not have to set up the callback for the source,
	// since I have something else I wish to do with the events seen,
	// and using the callback doesn't help me do it.
}

static void StartXcbMonitor()
{
    static bool linked=false;
    if (!linked)
    {
        LinkXcbToGlibLoop();
        linked =true;
    }
}
#else
static void StartXcbMonitor()
{
}
#endif

void XCBWindow::getKeyTranslations(xcb_keysym_t syms[], const xcb_key_press_event_t& ev)
{
	syms[0] = xcb_key_symbols_get_keysym(keysymbols, ev.detail, 0);
	syms[1] = xcb_key_symbols_get_keysym(keysymbols, ev.detail, 1);
}

XCBWindow* XCBWindow::create(const string& title)
{
	XCBWindow* w = new XCBWindow(createXcbWindow(title, Rect(20,20,800,600)));
	StartXcbMonitor();
	return w;
}
void XCBWindow::configure(const Rect& dest)
{
	uint32_t coords[] = { (uint32_t)dest.x(), (uint32_t)dest.y(), (uint32_t)dest.width(), (uint32_t)dest.height() };
	xcb_configure_window(this->xcb_connection, this->window, XCB_CONFIG_WINDOW_X
								| XCB_CONFIG_WINDOW_Y
								| XCB_CONFIG_WINDOW_WIDTH
								| XCB_CONFIG_WINDOW_HEIGHT, coords);
}
