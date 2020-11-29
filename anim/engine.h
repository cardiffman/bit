#ifndef INCLUDED_GRAPHICS_ENGINE_H
#define INCLUDED_GRAPHICS_ENGINE_H
#include "geometry.h"
#include <stdbool.h>
#include <stdint.h>
struct GraphicsBuffer {
    virtual ~GraphicsBuffer() {}
    RectSize dims;
    virtual void lock(uint8_t*& pixels, uint32_t& rowBytes) = 0;
    virtual void unlock() = 0;
};
struct GraphicsEngine {
    virtual ~GraphicsEngine() {}
    virtual bool supportsBuffer(GraphicsBuffer* buffer) = 0;
    virtual GraphicsBuffer* getScreenBuffer() = 0;
    virtual GraphicsBuffer* makeBuffer(const RectSize& dims) = 0;
    virtual GraphicsBuffer* makeBuffer(const RectSize& dims, void* data, uint32_t rowBytes) = 0;
    virtual void blit(GraphicsBuffer* dst, int dstX, int dstY, GraphicsBuffer* src, const Area& srcArea) = 0;
    virtual void fill(GraphicsBuffer* dst, const Area& dstArea, uint32_t color) = 0;
    virtual void stretchSrcCopy(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea) = 0;
    virtual void stretchSrcOver(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea) = 0;
};

struct BlittingEngine : public GraphicsEngine
{
    void blit(GraphicsBuffer *dst, int dstX, int dstY, GraphicsBuffer *src, const Area &srcArea);
    void fill(GraphicsBuffer *dst, const Area &dstArea, uint32_t color);
    void stretchSrcCopy(GraphicsBuffer *dst, const Area &dstArea, GraphicsBuffer *src, const Area &srcArea);
    void stretchSrcOver(GraphicsBuffer *dst, const Area &dstArea, GraphicsBuffer *src, const Area &srcArea);
};

struct BasicBuffer : public GraphicsBuffer
{
	uint8_t* mem;
	unsigned rowbytes;
	void lock(uint8_t*& data, uint32_t& rowBytes) { data = mem; rowBytes = this->rowbytes;}
	void unlock() {}
};


GraphicsEngine* init_base_engine(GraphicsBuffer* screen=nullptr);
#endif
