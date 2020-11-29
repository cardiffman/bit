#include "shmwindow.h"

#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <xcb/xcb_image.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <iostream>
using std::cerr;
using std::endl;
using std::clog;

bool SHMEngine::get_shared_memory()
{
    xcb_shm_query_version_reply_t*  reply;

    xcb_shm_query_version_cookie_t cookie = xcb_shm_query_version(XCBWindow::xcb_connection);
    clog << "xcb_shm_query_version cookie: " << cookie.sequence << endl;
    reply = xcb_shm_query_version_reply(
        		XCBWindow::xcb_connection,
                cookie, 
                NULL
            );

    if(!reply || !reply->shared_pixmaps){
        clog << "Shm error..." << endl;
        exit(0);
    }
//#define WID 512
//#define HEI 512

    info.shmid   = shmget(IPC_PRIVATE, 1280*720*4, IPC_CREAT | 0777);
    info.shmaddr = (uint8_t*)shmat(info.shmid, 0, 0);

    info.shmseg = xcb_generate_id(XCBWindow::xcb_connection);
    xcb_shm_attach(XCBWindow::xcb_connection, info.shmseg, info.shmid, 0);
    //shmctl(info.shmid, IPC_RMID, 0);

    uint32_t* data = (uint32_t*)info.shmaddr;
    screenBuffer = makeBuffer(RectSize(1280,720), info.shmaddr, 1280*4);
    return true;
}

GraphicsBuffer* SHMEngine::makeBuffer(const RectSize& dims)
{
	return makeBufferInternal(dims);
}
GraphicsBuffer* SHMEngine::makeBuffer(const RectSize& dims, void* data, uint32_t rowBytes)
{
	auto r =  new BasicBuffer();
	r->dims = dims;
	r->rowbytes = rowBytes;
	r->mem =  static_cast<uint8_t*>(data);
	return r;
}
BasicBuffer* SHMEngine::makeBufferInternal(const RectSize& dims)
{
	auto r =  new BasicBuffer();
	r->dims = dims;
	r->rowbytes = dims.width*4;
	r->mem =  new uint8_t[dims.width*4*dims.height];
	return r;
}
SHMEngine* engine;

shmwindow::shmwindow(const Rect& rect)
: XCBWindow(createXcbWindow("shmwindow", rect))
, pix(0)
{
}
shmwindow* shmwindow::create()
{
	shmwindow* w = new shmwindow(Rect((1920-1280)/2,(1080-720)/2,1280,720));

    engine = new SHMEngine();
    w->pix = xcb_generate_id(XCBWindow::xcb_connection);
    auto cookie = xcb_shm_create_pixmap(
    	XCBWindow::xcb_connection,
        w->pix,
        w->getWindow(),
        1280, 720,
        XCBWindow::xcb_screen->root_depth,
        engine->info.shmseg,
        0
    );
    clog << "xcb_shm_create_pixmap cookie " << cookie.sequence << endl;
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
