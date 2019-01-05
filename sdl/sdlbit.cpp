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
#include "res_path.h"
#include "cleanup.h"

using std::vector;

const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;

//We'll be scaling our tiles to be 40x40
const int TILE_SIZE = 40;

namespace SDLBIT {
struct Asset {
	unsigned id; SDL_Texture* image; unsigned width; unsigned height; unsigned rowbytes;
};
struct Glyph {
	unsigned id; unsigned parent_id; unsigned asset_id; int x; int y; unsigned width; unsigned height; uint32_t color; unsigned children;
};
struct Scene {
	vector<Asset> assets;
	vector<Glyph> glyphs;
};
}
static const unsigned NO_PAR = 0;
SDLBIT::Scene scene = {
		{
		{0},
		{ 1, NULL, 1280, 720, 1280*4 },
		{ 2, NULL, 100, 100, 100*4 }
		},
		{
				{0},
				{ 1, NO_PAR, 0, 0, 0, 1280, 720 ,0xff0000FF},
				{ 2, NO_PAR, 0, 0, 0, 100, 100,0xffFFFF00},
				{ 3, NO_PAR, 0, 100,100,1080,520 ,0xff808080},
				{ 4, 3,      0, 0, 0, 100, 100,0xffFF00FF},
				{ 5, 3,      0, 100, 0, 100, 100,0xff808080},
				{ 6, 3,      0, 200, 0, 100, 100,0xff808080},
				{ 7, 3,      0, 300, 0, 100, 100,0xff808080},
				{ 8, 3,      0, 400, 0, 100, 100,0xff808080},
				{ 9, 3,      0, 500, 0, 100, 100,0xff808080},
				{ 10, 3,     0, 600, 0, 100, 100,0xff808080},
				{ 11, 3,     0, 700, 0, 100, 100,0xff808080},
				{ 12, 3,     0, 800, 0, 100, 100,0xff808080},
				{ 13, 3,     0, 900, 0, 100, 100,0xff808080},
				{ 14, 3,     0, 1000, 0, 100, 100,0xff808080},
		}
};

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
}

/*
 * Loads an image into a texture on the rendering device
 * @param file The image file to load
 * @param ren The renderer to load the texture onto
 * @return the loaded texture, or nullptr if something went wrong.
 */
SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren){
	SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
	if (texture == nullptr){
		logSDLError(std::cout, "LoadTexture");
	}
	return texture;
}
/**
* Draw an SDL_Texture to an SDL_Renderer at position x, y, with some desired
* width and height
* @param tex The source texture we want to draw
* @param ren The renderer we want to draw to
* @param x The x coordinate to draw to
* @param y The y coordinate to draw to
* @param w The width of the texture to draw
* @param h The height of the texture to draw
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}
/**
* Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
* the texture's width and height
* @param tex The source texture we want to draw
* @param ren The renderer we want to draw to
* @param x The x coordinate to draw to
* @param y The y coordinate to draw to
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	//Query the texture to get its width and height to use
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

/*
 * Draw an SDL_Texture to an SDL_Renderer at some destination rect
 * taking a clip of the texture if desired
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param dst The destination rectangle to render the texture too
 * @param clip The sub-section of the texture to draw (clipping rect)
 *		default of nullptr draws the entire texture
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr){
	SDL_RenderCopy(ren, tex, clip, &dst);
}
/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
 * the texture's width and height and taking a clip of the texture if desired
 * If a clip is passed, the clip's width and height will be used instead of the texture's
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 * @param clip The sub-section of the texture to draw (clipping rect)
 *		default of nullptr draws the entire texture
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip){
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if (clip != nullptr){
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else {
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}
	renderTexture(tex, ren, dst, clip);
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

namespace SDLBIT {

void draw_scene(SDL_Renderer* renderer)
{
	for (auto g : scene.glyphs) {
		g.children = 0;
	}
	for (auto g : scene.glyphs) {
		if (g.parent_id != NO_PAR)
			scene.glyphs[g.parent_id].children++;
	}

	for (auto g : scene.glyphs) {
		int offsetx = 0;
		int offsety = 0;
		if (g.children)
			continue;
		if (g.parent_id != NO_PAR)
		{
			offsetx = scene.glyphs[g.parent_id].x;
			offsety = scene.glyphs[g.parent_id].y;
		}
		if (g.asset_id)
		{
			renderTexture(scene.assets[g.asset_id].image, renderer, g.x+offsetx, g.y+offsety);
		}
		else
		{
			SDL_Rect r; r.x = g.x+offsetx; r.y = g.y+offsety; r.w = g.width; r.h = g.height;
			SDL_SetRenderDrawColor(renderer, (g.color&0x000FF0000)>>16,(g.color&0x00000FF00)>>8, (g.color&0x0000000FF),
					(g.color&0x0FF000000)>>24);
			SDL_RenderFillRect(renderer, &r);
		}
	}
}
}

int main(int, char**){
	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	//Setup our window and renderer
	SDL_Window *window = SDL_CreateWindow("Lesson 5", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr){
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr){
		logSDLError(std::cout, "CreateRenderer");
		cleanup(window);
		SDL_Quit();
		return 1;
	}
	const std::string resPath = getResourcePath("Lesson5");
	SDL_Texture *image = loadTexture(resPath + "image.png", renderer);
	if (image == nullptr){
		cleanup(image, renderer, window);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	//iW and iH are the clip width and height
	//We'll be drawing only clips so get a center position for the w/h of a clip
	int iW = 100, iH = 100;
	int x = SCREEN_WIDTH / 2 - iW / 2;
	int y = SCREEN_HEIGHT / 2 - iH / 2;

	//Setup the clips for our image
	SDL_Rect clips[4];
	//Since our clips our uniform in size we can generate a list of their
	//positions using some math (the specifics of this are covered in the lesson)
	for (int i = 0; i < 4; ++i){
		clips[i].x = i / 2 * iW;
		clips[i].y = i % 2 * iH;
		clips[i].w = iW;
		clips[i].h = iH;
	}
	//Specify a default clip to start with
	int useClip = 0;

	SDL_Event e;
	bool quit = false;
	while (!quit){
		//Event Polling
		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT){
				quit = true;
			}
			//Use number input to select which clip should be drawn
			if (e.type == SDL_KEYDOWN){
				switch (e.key.keysym.sym){
					case SDLK_1:
					case SDLK_KP_1:
						useClip = 0;
						break;
					case SDLK_2:
					case SDLK_KP_2:
						useClip = 1;
						break;
					case SDLK_3:
					case SDLK_KP_3:
						useClip = 2;
						break;
					case SDLK_4:
					case SDLK_KP_4:
						useClip = 3;
						break;
					case SDLK_ESCAPE:
						quit = true;
						break;
					default:
						break;
				}
			}
		}
		//Rendering
		SDL_RenderClear(renderer);
		//Draw the image
		SDLBIT::draw_scene(renderer);
		//Update the screen
		SDL_RenderPresent(renderer);
	}
	//Clean up
	cleanup(image, renderer, window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
