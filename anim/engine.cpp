#include "engine.h"
//#include "bitbuffer.h"
#include <malloc.h>

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