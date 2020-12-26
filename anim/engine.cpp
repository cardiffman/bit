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
    float x_diff, y_diff, blue, red, green, alpha ;
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
            alpha = ((a>>24)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>24)&0xff)*(x_diff)*(1-y_diff) +
                  ((c>>24)&0xff)*(y_diff)*(1-x_diff)   + ((d>>24)&0xff)*(x_diff*y_diff);
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
            alpha = (int64_t)(((a>>24)&0xff)*(65536-x_diff)*(65536-y_diff) + ((b>>24)&0xff)*(x_diff)*(65536-y_diff) +
                  ((c>>24)&0xff)*(y_diff)*(65536-x_diff)   + ((d>>24)&0xff)*(x_diff*y_diff))>>32;
#endif

            idstptr[(i+dstArea.y)*dst_rowbytes/4+dstArea.x+j] =
			#if 0
                    0xff000000 | // hardcode alpha
			#else
                    ((((int)alpha)<<24)&0xff000000) |
			#endif
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

unsigned srcOver32(unsigned src, unsigned dst)
{
	unsigned char r, g, b, a;
	//unsigned alpha, inv_alpha;
	unsigned char* psrc = (unsigned char*)&src;
	unsigned char* pdst = (unsigned char*)&dst;

	unsigned int alpha = psrc[3];
	unsigned int inv_alpha = 255 - alpha;
#if 0
	b = (unsigned char)((psrc[0] + inv_alpha * pdst[0])/255);
	g = (unsigned char)((psrc[1] + inv_alpha * pdst[1])/255);
	r = (unsigned char)((psrc[2] + inv_alpha * pdst[2])/255);
#else
	b = (unsigned char)((alpha * psrc[0] + inv_alpha * pdst[0])/255);
	g = (unsigned char)((alpha * psrc[1] + inv_alpha * pdst[1])/255);
	r = (unsigned char)((alpha * psrc[2] + inv_alpha * pdst[2])/255);
#endif
	a = 0xff;
	return (a<<24)|(r<<16)|(g<<8)|b;
}
void BlittingEngine::stretchSrcOver(GraphicsBuffer* pdst, const Area& dstArea, GraphicsBuffer* psrc, const Area& srcArea)
{
	uint8_t *dstptr;
	uint32_t dst_rowbytes;
	uint8_t *srcptr;
	uint32_t src_rowbytes;
	pdst->lock(dstptr, dst_rowbytes);
	psrc->lock(srcptr, src_rowbytes);

	uint32_t* idstptr = (uint32_t*)dstptr;
	uint32_t* isrcptr = (uint32_t*)srcptr;

	int x_ratio = (int)((srcArea.width << 16) / dstArea.width) + 1;
	int y_ratio = (int)((srcArea.height << 16) / dstArea.height) + 1;
	for (int i = 0; i < dstArea.height; ++i)
	{
#if 1
		for (int j = 0; j < dstArea.width; ++j)
		{
            int x =      (x_ratio * j)>>16 ;
            int y =      (y_ratio * i)>>16 ;
            int x_diff = (x_ratio * j)&0xFFFF ;
            int y_diff = (y_ratio * i)&0xFFFF ;

            unsigned index = (y*src_rowbytes/4+x) ;
            unsigned a, b, c, d; // The corners that we give weight to.
            unsigned red, green, blue, alpha;
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

            unsigned aweight = (65535-x_diff)*(65535-y_diff);
            unsigned bweight = x_diff*(65535-y_diff);
            unsigned cweight = (65535-x_diff)*y_diff;
            unsigned dweight = x_diff*y_diff;
            // Yc = Ac(1-w)(1-h) + Bc(w)(1-h) + Cc(h)(1-w) + Dc(wh)
            blue = ((a&255LL)*aweight + (b&255LL)*bweight + (c&255LL)*cweight + (d&255LL)*dweight)>>32;
            alpha = (((a>>24)&255LL)*aweight + ((b>>24)&255LL)*bweight + ((c>>24)&255LL)*cweight + ((d>>24)&255LL)*dweight)>>32;
            red =   (((a>>16)&255LL)*aweight + ((b>>16)&255LL)*bweight + ((c>>16)&255LL)*cweight + ((d>>16)&255LL)*dweight)>>32;
			green = (((a>> 8)&255LL)*aweight + ((b>> 8)&255LL)*bweight + ((c>> 8)&255LL)*cweight + ((d>> 8)&255LL)*dweight)>>32;
            auto& p = idstptr[(i+dstArea.y)*dst_rowbytes/4+dstArea.x+j];
            p = srcOver32((alpha<<24)|(red<<16)|(green<<8)|blue, p);
            //p = srcOver32((128<<24)|(red<<16)|(green<<8)|blue, p);
		}
#else
		int y2 = ((i * y_ratio) >> 16);
#if 0
		uint8_t* dstmem = dstptr + dst_rowbytes * (i + dstArea.y) + 4 * dstArea.x;
		uint8_t* srcmem = srcptr + src_rowbytes * (y2 + srcArea.y) + 4 * srcArea.x;
		for (int j = 0; j < dstArea.width; ++j)
		{
			int x2 = ((j * x_ratio) >> 16);
			uint8_t* fg = srcmem+x2*4;
			uint8_t* bg = dstmem+j*4;
#if 1
			unsigned int alpha = fg[3];
			unsigned int inv_alpha = 255 - alpha;
			bg[0] = (unsigned char)((alpha * fg[0] + inv_alpha * bg[0])/255);
			bg[1] = (unsigned char)((alpha * fg[1] + inv_alpha * bg[1])/255);
			bg[2] = (unsigned char)((alpha * fg[2] + inv_alpha * bg[2])/255);
			bg[3] = 0xff;
#elif 1
			unsigned int alpha = fg[3];
			unsigned int inv_alpha = 255 - alpha;
			bg[0] = (unsigned char)((alpha * fg[0] + inv_alpha * bg[0]) >> 8);
			bg[1] = (unsigned char)((alpha * fg[1] + inv_alpha * bg[1]) >> 8);
			bg[2] = (unsigned char)((alpha * fg[2] + inv_alpha * bg[2]) >> 8);
			bg[3] = 0xff;
#else
			unsigned int alpha = fg[3] + 1;
			unsigned int inv_alpha = 256 - fg[3];
			bg[0] = (unsigned char)((alpha * fg[0] + inv_alpha * bg[0]) >> 8);
			bg[1] = (unsigned char)((alpha * fg[1] + inv_alpha * bg[1]) >> 8);
			bg[2] = (unsigned char)((alpha * fg[2] + inv_alpha * bg[2]) >> 8);
			bg[3] = 0xff;
#endif
		}
#else
		uint32_t *dstmem = (uint32_t *)(dstptr + dst_rowbytes * (i + dstArea.y) + 4 * dstArea.x);
		uint32_t *srcmem = (uint32_t *)(srcptr + src_rowbytes * (y2 + srcArea.y) + 4 * srcArea.x);
		for (int j = 0; j < dstArea.width; ++j)
		{
			int x2 = ((j * x_ratio) >> 16);
			unsigned src_a = (srcmem[x2] >> 24) & 0xFF;
			unsigned src_r = (srcmem[x2] >> 16) & 0xFF;
			unsigned src_g = (srcmem[x2] >> 8) & 0xFF;
			unsigned src_b = (srcmem[x2]) & 0xFF;
			unsigned dst_a = (dstmem[j] >> 24) & 0xFF;
			unsigned dst_r = (dstmem[j] >> 16) & 0xFF;
			unsigned dst_g = (dstmem[j] >> 8) & 0xFF;
			unsigned dst_b = (dstmem[j]) & 0xFF;
			unsigned inv_a = 255-src_a;
			if (dst_a == 255)
			{
				unsigned out_a = ((dst_a * inv_a) + (src_a )/255);
				dst_r = ((dst_r * dst_a * inv_a) + (src_r * src_a)/255)/out_a;
				dst_g = ((dst_g * dst_a * inv_a) + (src_g * src_a)/255)/out_a;
				dst_b = ((dst_b * dst_a * inv_a) + (src_b * src_a)/255)/out_a;
				dst_a = out_a;
			}
			else if (dst_a == 0)
			{
				dst_r = dst_b = dst_g = 0;
			}
			else
			{
				unsigned out_a = ((dst_a * inv_a) + (src_a )/255);
				dst_r = ((dst_r * dst_a * inv_a) + (src_r * src_a)/255)/out_a;
				dst_g = ((dst_g * dst_a * inv_a) + (src_g * src_a)/255)/out_a;
				dst_b = ((dst_b * dst_a * inv_a) + (src_b * src_a)/255)/out_a;
				dst_a = out_a;
			}
			dstmem[j] = (dst_a << 24) + (dst_r << 16) + (dst_g << 8) + dst_b; //srcmem[x2];
		}
#endif
#endif
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