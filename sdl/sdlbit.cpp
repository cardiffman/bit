/*
 * sdlbit.cpp
 *
 *  Created on: Dec 15, 2018
 *      Author: menright
 */

#include <iostream>
#include <string>
#include <vector>
#include <limits.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "cleanup.h"
#include "scene.h"
#include "scene_builder.h"
#include "engine.h"
#include <cstring>
#include <unistd.h>

using std::vector;
using std::cout;
using std::endl;

const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
}

class SDLScreenBuffer : public GraphicsBuffer
{
public:
	SDLScreenBuffer(SDL_Window *window);
	~SDLScreenBuffer() { cleanup(renderer); }
	void lock(uint8_t*& pixels, uint32_t& rowBytes) { pixels = nullptr; rowBytes = 0; }
	void unlock() {}
	SDL_Renderer *renderer;
private:
	friend class SDLEngine;
};

class SDLBuffer : public GraphicsBuffer
{
public:
	SDLBuffer(SDL_Renderer* renderer, const RectSize& dims, void* data, uint32_t rowBytes);
	SDLBuffer(SDL_Renderer* renderer, const RectSize& dims);
	~SDLBuffer()
	{
		cleanup(sdlTexture);
	}
	void lock(uint8_t*& pixels, uint32_t& rowBytes) { pixels = nullptr; rowBytes = 0; }
	void unlock() {}
	SDL_Texture* sdlTexture;
private:
};

class SDLEngine : public BlittingEngine
{
public:
	SDLEngine(SDL_Window* window);
	~SDLEngine() { delete screen; }
	bool supportsBuffer(GraphicsBuffer* b)
	{
		return true;
	}
	GraphicsBuffer* getScreenBuffer();
	GraphicsBuffer* makeBuffer(const RectSize& dims);
	GraphicsBuffer* makeBuffer(const RectSize& dims, void* data, uint32_t rowBytes);
	void fill(GraphicsBuffer* dst, const Area& dstArea, uint32_t color);
	void blit(GraphicsBuffer* dst, int dstX, int dstY, GraphicsBuffer* src, const Area& srcArea);
	void stretchSrcCopy(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea);
	void stretchSrcOver(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea);
	SDLScreenBuffer* screen;
private:
	SDL_Window* window;
};

SDLScreenBuffer::SDLScreenBuffer(SDL_Window *window)
{
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr){
		logSDLError(std::cout, "CreateRenderer");
		throw "CreateRenderer";
	}
}

SDLBuffer::SDLBuffer(SDL_Renderer* renderer, const RectSize& dims, void* data, uint32_t rowBytes)
{
	this->dims = dims;
	auto sdlSurface = SDL_CreateRGBSurfaceFrom(data, dims.width, dims.height, 32, rowBytes, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (!sdlSurface)
		throw "SDLBuffer from data";
	sdlTexture = SDL_CreateTextureFromSurface(renderer, sdlSurface);
	if (!sdlTexture)
	{
		logSDLError(std::cout, "CreateTextureFromSurface");
		throw "CreateTextureFromSurface";
	}
	cleanup(sdlSurface);
}
SDLBuffer::SDLBuffer(SDL_Renderer* renderer, const RectSize& dims)
{
	throw "streaming texture";
	sdlTexture = SDL_CreateTexture(renderer,
										SDL_PIXELFORMAT_ARGB8888,
										SDL_TEXTUREACCESS_STREAMING,
										dims.width, dims.height);
	if (!sdlTexture)
	{
		logSDLError(std::cout, "CreateTexture");
		throw "CreateTexture";
	}
}
SDLEngine::SDLEngine(SDL_Window* window) : screen(0), window(window)
{
	screen = new SDLScreenBuffer(window);
}

GraphicsBuffer* SDLEngine::getScreenBuffer()
{
	if (screen == nullptr)
		screen = new SDLScreenBuffer(window);
	return screen;
}
GraphicsBuffer* SDLEngine::makeBuffer(const RectSize& dims, void* data, uint32_t rowBytes)
{
	return new SDLBuffer(screen->renderer, dims, data, rowBytes);
}
GraphicsBuffer* SDLEngine::makeBuffer(const RectSize& dims)
{
	return new SDLBuffer(screen->renderer, dims);
}
void SDLEngine::fill(GraphicsBuffer* dst, const Area& dstArea, uint32_t color)
{
	SDLScreenBuffer* cdst = dynamic_cast<SDLScreenBuffer*>(dst);
	SDL_SetRenderDrawColor(screen->renderer, (color&0x00FF0000)>>16, (color&0x0000FF00)>>8, (color&0x000000FF), (color&0xFF000000)>>24);
	SDL_Rect dstRect = {dstArea.x, dstArea.y, dstArea.width, dstArea.height};
	SDL_RenderFillRect(screen->renderer, &dstRect);
}

void SDLEngine::blit(GraphicsBuffer* dst, int dstX, int dstY, GraphicsBuffer* src, const Area& srcArea)
{
	SDLScreenBuffer* cdst = dynamic_cast<SDLScreenBuffer*>(dst);
	SDLBuffer* csrc = dynamic_cast<SDLBuffer*>(src);
	SDL_Rect dstRect = {dstX, dstY, srcArea.width, srcArea.height};
	SDL_Rect srcRect = {srcArea.x, srcArea.y, srcArea.width, srcArea.height};
	SDL_SetTextureBlendMode(csrc->sdlTexture, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(cdst->renderer, csrc->sdlTexture, &srcRect, &dstRect);
}

void SDLEngine::stretchSrcCopy(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea)
{
	SDLScreenBuffer* cdst = dynamic_cast<SDLScreenBuffer*>(dst);
	SDLBuffer* csrc = dynamic_cast<SDLBuffer*>(src);
	SDL_Rect dstRect = {dstArea.x, dstArea.y, dstArea.width, dstArea.height};
	SDL_Rect srcRect = {srcArea.x, srcArea.y, srcArea.width, srcArea.height};
	SDL_SetTextureBlendMode(csrc->sdlTexture, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(cdst->renderer, csrc->sdlTexture, &srcRect, &dstRect);
}

void SDLEngine::stretchSrcOver(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea)
{
	SDLScreenBuffer* cdst = dynamic_cast<SDLScreenBuffer*>(dst);
	SDLBuffer* csrc = dynamic_cast<SDLBuffer*>(src);
	SDL_Rect dstRect = {dstArea.x, dstArea.y, dstArea.width, dstArea.height};
	SDL_Rect srcRect = {srcArea.x, srcArea.y, srcArea.width, srcArea.height};
	SDL_SetTextureBlendMode(csrc->sdlTexture, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(cdst->renderer, csrc->sdlTexture, &srcRect, &dstRect);
}

/*
 * Render the message we want to display to a texture for drawing
 * @param message The message we want to display
 * @param fontFile The font we want to use to render the text
 * @param color The color we want the text to be
 * @param fontSize The size we want the font to be
 * @param renderer The renderer to load the texture in
 * @return An SDL_Texture containing the rendered message, or nullptr if something went wrong
 */
SDL_Texture* renderText(const std::string &message, const std::string &fontFile, SDL_Color color,
		int fontSize, SDL_Renderer *renderer)
{
	//Open the font
	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr){
		logSDLError(std::cout, "TTF_OpenFont");
		return nullptr;
	}
	//We need to first render to a surface as that's what TTF_RenderText returns, then
	//load that surface into a texture
	SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
	if (surf == nullptr){
		TTF_CloseFont(font);
		logSDLError(std::cout, "TTF_RenderText");
		return nullptr;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == nullptr){
		logSDLError(std::cout, "CreateTexture");
	}
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	return texture;
}

Scene scene;

int main(int argc, char** argv){
	char path[2048];

	readlink("/proc/self/exe",path, sizeof(path));
	char* sl = strrchr(path, '/');
	sl[1] = 0;
	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	//Setup our window and renderer
	SDL_Window *window = SDL_CreateWindow("sdlbit", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr){
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	{
	SDLEngine engine(window);


	SceneBuilder builder;
	if (argc > 1)
	{
		try {
			builder.parse_containers(path, argv[1], &engine);
		} catch (char const * ex) {
			cout << "Exception " << ex << endl;
			return 1;
		} catch (const std::string& ex) {
			cout << "Exception " << ex << endl;
			return 1;
		}
		scene.containers = builder.nc;
		scene.assets = builder.na;
	}
	else
	{
		scene.containers.push_back(Container());
		scene.containers.push_back(Container({{ 0,0,SCREEN_WIDTH,SCREEN_HEIGHT },1,0,0,0xFFFF0000}));
		scene.assets = {{0}};
	}
	draw_scene(scene, &engine);
	SDL_RenderPresent(engine.screen->renderer);

	SDL_Event e;
	bool quit = false;
	while (!quit){
		//Event Polling
		if (SDL_WaitEvent(&e)){
			if (e.type == SDL_QUIT){
				quit = true;
			} else if (e.type == SDL_KEYDOWN) {
				;
			} else if (e.type == SDL_KEYUP) {
				;
			}
		}
	}
	}
	//Clean up
	cleanup(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
