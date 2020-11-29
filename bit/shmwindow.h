#pragma once

#include "XCBWindow.h"
#include "scene_builder.h"
#include "engine.h"
#include <xcb/xcb_image.h>

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

class SHMEngine : public BlittingEngine
{
public:
    SHMEngine()
    {
        get_shared_memory();
    }
    bool supportsBuffer(GraphicsBuffer* buffer) { return true; }
    GraphicsBuffer* getScreenBuffer()
    {
        return screenBuffer;
    }
    GraphicsBuffer* makeBuffer(const RectSize& dims);
    GraphicsBuffer* makeBuffer(const RectSize& dims, void* data, uint32_t rowBytes);
    xcb_shm_segment_info_t          info;
private:
    bool get_shared_memory();
    BasicBuffer* makeBufferInternal(const RectSize& dims);
    GraphicsBuffer* screenBuffer;
};
extern SHMEngine* engine;
