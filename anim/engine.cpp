#include "engine.h"
//#include "bitbuffer.h"
#include <malloc.h>
#include <iostream>
using namespace std;

struct BasicEngine : public BlittingEngine
{
	BasicEngine(GraphicsBuffer* screen=nullptr);
	bool supportsBuffer(GraphicsBuffer* subj);
	GraphicsBuffer* makeBuffer(const RectSize& dims);
	GraphicsBuffer* makeBuffer(const RectSize& dims, void* data, uint32_t rowBytes);
	GraphicsBuffer* getScreenBuffer() { return screen; }
private:
    BasicBuffer* makeBufferInternal(const RectSize& dims);
	GraphicsBuffer* screen;
};
BasicEngine::BasicEngine(GraphicsBuffer* screen)
: screen(screen)
{
	if (screen == nullptr)
		this->screen = makeBufferInternal(RectSize(1280,720));
}

bool BasicEngine::supportsBuffer(GraphicsBuffer* p)
{
	return dynamic_cast<BasicBuffer*>(p)!=nullptr;
}
void BlittingEngine::blit(GraphicsBuffer* pdst
		, int dstX, int dstY
		, GraphicsBuffer* psrc
		, const Area& srcArea
		)
{
	uint8_t* dstptr; uint32_t dst_rowbytes;
	uint8_t* srcptr; uint32_t src_rowbytes;
	pdst->lock(dstptr, dst_rowbytes);
	psrc->lock(srcptr, src_rowbytes);
	uint32_t* dstrow = (uint32_t*)(dstptr + dstY*dst_rowbytes);
	uint32_t* srcrow = (uint32_t*)(srcptr + srcArea.y*src_rowbytes);
	for (unsigned y = 0; y<srcArea.height; y++) {
		for (unsigned x = 0; x<srcArea.width; x++) {
			dstrow[x+dstX] = srcrow[x+srcArea.x];
		}
		dstrow = dstrow + (dst_rowbytes/4);//(uint32_t*)(((uint8_t*)dstrow)+dst.rowbytes);
		srcrow = srcrow + (src_rowbytes/4);//(uint32_t*)(((uint8_t*)srcrow)+src.rowbytes);
	}
	psrc->unlock();
	pdst->unlock();
}

void BlittingEngine::stretchSrcCopy(GraphicsBuffer *pdst, const Area &dstArea, GraphicsBuffer *psrc, const Area &srcArea)
{
	uint8_t *dstptr;
	uint32_t dst_rowbytes;
	uint8_t *srcptr;
	uint32_t src_rowbytes;
	pdst->lock(dstptr, dst_rowbytes);
	psrc->lock(srcptr, src_rowbytes);

	uint32_t* idstptr = (uint32_t*)dstptr;
	uint32_t* isrcptr = (uint32_t*)srcptr;
//#define FLOATS
#ifdef FLOATS
	float x_ratio = ((float)(srcArea.width-1))/dstArea.width ;
	float y_ratio = ((float)(srcArea.height-1))/dstArea.height ;
	int a, b, c, d, x, y, index ;
    float x_diff, y_diff, blue, red, green ;
#else
	int x_ratio = (int)((srcArea.width << 16) / dstArea.width) + 1;
	int y_ratio = (int)((srcArea.height << 16) / dstArea.height) + 1;
    int a, b, c, d, x, y, index ;
    float x_diff, y_diff, blue, red, green ;
#endif
    int offset = 0 ;
    for (int i=0;i<dstArea.height;i++) {
        for (int j=0;j<dstArea.width;j++) {
#ifdef FLOATS
        	x = (int)(x_ratio * j) ;
        	y = (int)(y_ratio * i) ;
        	x_diff = (x_ratio * j) - x ;
        	y_diff = (y_ratio * i) - y ;
#else
            x =      (x_ratio * j)>>16 ;
            y =      (y_ratio * i)>>16 ;
            x_diff = (x_ratio * j)&0xFFFF ;
            y_diff = (y_ratio * i)&0xFFFF ;
#endif
            index = (y*src_rowbytes/4+x) ;
            a = isrcptr[index] ;
            if (j<dstArea.width-1)
            	b = isrcptr[index+1] ;
            else b = 0;
            if (i<dstArea.height-1)
            	c = isrcptr[index+src_rowbytes/4] ;
            else c = 0;
            if (j<dstArea.width-1 && i<dstArea.height-1)
            	d = isrcptr[index+src_rowbytes/4+1] ;
            else d = 0;

            // blue element
            // Yb = Ab(1-w)(1-h) + Bb(w)(1-h) + Cb(h)(1-w) + Db(wh)
#ifdef FLOATS
            blue = (a&0xff)*(1-x_diff)*(1-y_diff) + (b&0xff)*(x_diff)*(1-y_diff) +
                   (c&0xff)*(y_diff)*(1-x_diff)   + (d&0xff)*(x_diff*y_diff);

            // green element
            // Yg = Ag(1-w)(1-h) + Bg(w)(1-h) + Cg(h)(1-w) + Dg(wh)
            green = ((a>>8)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>8)&0xff)*(x_diff)*(1-y_diff) +
                    ((c>>8)&0xff)*(y_diff)*(1-x_diff)   + ((d>>8)&0xff)*(x_diff*y_diff);

            // red element
            // Yr = Ar(1-w)(1-h) + Br(w)(1-h) + Cr(h)(1-w) + Dr(wh)
            red = ((a>>16)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>16)&0xff)*(x_diff)*(1-y_diff) +
                  ((c>>16)&0xff)*(y_diff)*(1-x_diff)   + ((d>>16)&0xff)*(x_diff*y_diff);
#else
            blue = (int64_t)((a&0xff)*(65536-x_diff)*(65536-y_diff) + (b&0xff)*(x_diff)*(65536-y_diff) +
                   (c&0xff)*(y_diff)*(65536-x_diff)   + (d&0xff)*(x_diff*y_diff))>>32;

            // green element
            // Yg = Ag(1-w)(1-h) + Bg(w)(1-h) + Cg(h)(1-w) + Dg(wh)
            green = (int64_t)(((a>>8)&0xff)*(65536-x_diff)*(65536-y_diff) + ((b>>8)&0xff)*(x_diff)*(65536-y_diff) +
                    ((c>>8)&0xff)*(y_diff)*(65536-x_diff)   + ((d>>8)&0xff)*(x_diff*y_diff))>>32;

            // red element
            // Yr = Ar(1-w)(1-h) + Br(w)(1-h) + Cr(h)(1-w) + Dr(wh)
            red = (int64_t)(((a>>16)&0xff)*(65536-x_diff)*(65536-y_diff) + ((b>>16)&0xff)*(x_diff)*(65536-y_diff) +
                  ((c>>16)&0xff)*(y_diff)*(65536-x_diff)   + ((d>>16)&0xff)*(x_diff*y_diff))>>32;
#endif

            idstptr[(i+dstArea.y)*dst_rowbytes/4+dstArea.x+j] =
                    0xff000000 | // hardcode alpha
                    ((((int)red)<<16)&0xff0000) |
                    ((((int)green)<<8)&0xff00) |
                    ((int)blue) ;
        }
    }
	psrc->unlock();
	pdst->unlock();
}
void BlittingEngine::fill(GraphicsBuffer *pdst, const Area &dstArea, uint32_t color)
{
	uint8_t* dstptr; uint32_t dst_rowbytes;
	pdst->lock(dstptr, dst_rowbytes);
	for (unsigned y = 0; y<dstArea.height; y++) {
		uint32_t* dstrow = (uint32_t*)(dstptr + (dstArea.y+y)*dst_rowbytes);
		for (unsigned x = 0; x<dstArea.width; x++) {
			dstrow[x+dstArea.x] = color;
		}
	}
	pdst->unlock();
}

void BlittingEngine::stretchSrcOver(GraphicsBuffer* pdst, const Area& dstArea, GraphicsBuffer* psrc, const Area& srcArea)
{
	uint8_t *dstptr;
	uint32_t dst_rowbytes;
	uint8_t *srcptr;
	uint32_t src_rowbytes;
	pdst->lock(dstptr, dst_rowbytes);
	psrc->lock(srcptr, src_rowbytes);

	int x_ratio = (int)((srcArea.width << 16) / dstArea.width) + 1;
	int y_ratio = (int)((srcArea.height << 16) / dstArea.height) + 1;
	for (int i = 0; i < dstArea.height; ++i)
	{
		int y2 = ((i * y_ratio) >> 16);
		uint32_t *dstmem = (uint32_t *)(dstptr + dst_rowbytes * (i + dstArea.y) + 4 * dstArea.x);
		uint32_t *srcmem = (uint32_t *)(srcptr + src_rowbytes * (y2 + srcArea.y) + 4 * srcArea.x);
		for (int j = 0; j < dstArea.width; ++j)
		{
			int x2 = ((j * x_ratio) >> 16);
			dstmem[j] = srcmem[x2];
		}
	}
	psrc->unlock();
	pdst->unlock();
}

GraphicsBuffer* BasicEngine::makeBuffer(const RectSize& dims)
{
	return makeBufferInternal(dims);
}
GraphicsBuffer* BasicEngine::makeBuffer(const RectSize& dims, void* data, uint32_t rowBytes)
{
	auto r =  new BasicBuffer();
	r->dims = dims;
	r->rowbytes = rowBytes;
	r->mem =  static_cast<uint8_t*>(data);
	return r;
}
BasicBuffer* BasicEngine::makeBufferInternal(const RectSize& dims)
{
	auto r =  new BasicBuffer();
	r->dims = dims;
	r->rowbytes = dims.width*4;
	r->mem =  new uint8_t[dims.width*4*dims.height];
	return r;
}
GraphicsEngine* init_base_engine(GraphicsBuffer* screen)
{
	return new BasicEngine(screen);
}