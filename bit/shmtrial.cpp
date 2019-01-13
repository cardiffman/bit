/*
 * shmtrial.cpp
 *
 *  Created on: Jan 7, 2019
 *      Author: menright
 */

#include "XCBWindow.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <xcb/xcb_image.h>

#include "scene_builder.h"
#include "bitbuffer.h"
#include <iostream>
using std::cerr;
using std::endl;
using std::clog;

class shmwindow : public XCBWindow
{
public:
	static shmwindow* create();
	void onExpose(int x, int y, int width, int height, int count);
	void repaint();
	xcb_window_t getWindow() { return window; }
	void copy();
    SceneBuilder builder;
private:
	shmwindow(const Rect& rect);
	virtual void onKeyRelease(const KeyDescriptor& key);
	virtual void onKeyPress(const KeyDescriptor& key);
	xcb_pixmap_t pix;
};
BitBuffer screen;

shmwindow::shmwindow(const Rect& rect)
: XCBWindow(createXcbWindow("shmwindow", rect))
, pix(0)
{
}
shmwindow* shmwindow::create()
{
	shmwindow* w = new shmwindow(Rect((1920-1280)/2,(1080-720)/2,1280,720));

    xcb_shm_query_version_reply_t*  reply;
    xcb_shm_segment_info_t          info;

    reply = xcb_shm_query_version_reply(
    		XCBWindow::xcb_connection,
        xcb_shm_query_version(XCBWindow::xcb_connection),
        NULL
    );

    if(!reply || !reply->shared_pixmaps){
        printf("Shm error...\n");
        exit(0);
    }
//#define WID 512
//#define HEI 512

    info.shmid   = shmget(IPC_PRIVATE, 1280*720*4, IPC_CREAT | 0777);
    info.shmaddr = (uint8_t*)shmat(info.shmid, 0, 0);

    info.shmseg = xcb_generate_id(XCBWindow::xcb_connection);
    xcb_shm_attach(XCBWindow::xcb_connection, info.shmseg, info.shmid, 0);
    shmctl(info.shmid, IPC_RMID, 0);

    uint32_t* data = (uint32_t*)info.shmaddr;
    screen.mem = (uint8_t*)data; screen.rowbytes=1280*4; screen.dims.width=1280; screen.dims.height=720;

    w->pix = xcb_generate_id(XCBWindow::xcb_connection);
    xcb_shm_create_pixmap(
    	XCBWindow::xcb_connection,
        w->pix,
        w->getWindow(),
        1280, 720,
        XCBWindow::xcb_screen->root_depth,
        info.shmseg,
        0
    );
	return w;
}
void shmwindow::repaint()
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
		cerr << __FUNCTION__ << ": ERROR: can't put_image :"<<
				unsigned(error->error_code) << endl;
		xcb_disconnect(xcb_connection);
		exit(-1);
	}
    xcb_copy_area(
    	XCBWindow::xcb_connection,
        pix,
        getWindow(),
        gcontext,
        0, 0, 0, 0,
        1280, 720
    );
	xcb_free_gc(xcb_connection, gcontext);
    xcb_flush(XCBWindow::xcb_connection);
}
void shmwindow::onExpose(int x,int y,int width,int height,int count)
{
	repaint();
}

/*
    xcb_shm_query_version_reply_t*  reply;
    xcb_shm_segment_info_t          info;

    reply = xcb_shm_query_version_reply(
        connection,
        xcb_shm_query_version(connection),
        NULL
    );

    if(!reply || !reply->shared_pixmaps){
        printf("Shm error...\n");
        exit(0);
    }

    info.shmid   = shmget(IPC_PRIVATE, WID*HEI*4, IPC_CREAT | 0777);
    info.shmaddr = shmat(info.shmid, 0, 0);

    info.shmseg = xcb_generate_id(connection);
    xcb_shm_attach(connection, info.shmseg, info.shmid, 0);
    shmctl(info.shmid, IPC_RMID, 0);

    uint32_t* data = (uint32_t*)info.shmaddr;

    xcb_pixmap_t pix = xcb_generate_id(connection);
    xcb_shm_create_pixmap(
        connection,
        pix,
        window,
        WID, HEI,
        screen->root_depth,
        info.shmseg,
        0
    );

    int i = 0;
    for (; i<50000; ++i)
		data[i] = 0xFFFFFFFF;

    while(1){
        usleep(10000);

        data[i] = 0xFFFFFFFF;
        i++;

        xcb_copy_area(
            connection,
            pix,
            window,
            gcontext,
            0, 0, 0, 0,
            WID, HEI
        );

        xcb_flush(connection);
    }

    xcb_shm_detach(connection, info.shmseg);
    shmdt(info.shmaddr);

    xcb_free_pixmap(connection, pix);

 */

void shmwindow::copy()
{
    xcb_gcontext_t          gcontext;
    uint32_t value_mask;
    uint32_t value_list[2];

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = XCBWindow::xcb_screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;

    gcontext = xcb_generate_id(XCBWindow::xcb_connection);
    xcb_create_gc(XCBWindow::xcb_connection, gcontext, getWindow(), value_mask, value_list);

    xcb_copy_area(
    	XCBWindow::xcb_connection,
        pix,
        getWindow(),
        gcontext,
        0, 0, 0, 0,
        1280, 720
    );
    xcb_flush(XCBWindow::xcb_connection);
	xcb_free_gc(xcb_connection, gcontext);
}
void shmwindow::onKeyRelease(const KeyDescriptor& key)
{
	clog << __PRETTY_FUNCTION__ << ' '<< key.toString() << endl;
    std::string key_name;
    if (key.code() == 0x0020)
        key_name = "space";
    else if (key.code() == 0xff08)
        key_name = "backspace";
    else return; // nothing to dispatch
    for (auto userInput : builder.userInputs)
    {
        if (userInput.key == key_name)
        {
            clog << __PRETTY_FUNCTION__ << ' ' << "Should play " << userInput.animation << endl;
            return;
        }
    }
    clog << __PRETTY_FUNCTION__ << ' ' << "No animation to trigger" << endl;
}
void shmwindow::onKeyPress(const KeyDescriptor& key)
{
	clog << __PRETTY_FUNCTION__ << ' '<< key.toString() << endl;
}
int main(int argc, char** argv)
{
	shmwindow* win = shmwindow::create();
    //xcb_gcontext_t          gcontext;
	win->configure(Rect((1920-1280)/2,(1080-720)/2,1280,720));
    win->builder.parse_containers(argv[1]);
    Scene scene;
	scene.containers = win->builder.nc;
	scene.assets = win->builder.na;
    draw_scene(scene, screen);
	win->repaint();

    //int i = 0;
    //for (; i<50000; ++i)
	//	data[i] = 0xFFFFFF;
    if (argc < 2)
    	return 1;

    win->copy();
    int dx = 30;
	while (true)
	{
		if (!XCBWindow::pollEvents())
			break;
        usleep(10000);
#if 0
        Container& container = scene.containers[15];
        if (dx > 0)
        {
        	if (container.area.x > 400)
        		dx = -dx;
        }
        else
        {
        	if (container.area.x < -100)
        		dx = -dx;
        }
        container.area.x+=dx;

        //data[i] = 0xFFFFFFFF;
        //i++;

        draw_scene(scene, screen);
        win->repaint();
#endif
	}
	return 0;
}
