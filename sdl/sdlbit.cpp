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
#include "scene.h"
#include "scene_builder.h"

using std::vector;
using std::cout;
using std::endl;

const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = SCREEN_HEIGHT;

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
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
BitBuffer screen;

int main(int argc, char** argv){
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
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr){
		logSDLError(std::cout, "CreateRenderer");
		cleanup(window);
		SDL_Quit();
		return 1;
	}
#if 1 // Enable interesting code when event loop is debugged
    int width=SCREEN_WIDTH, height=SCREEN_HEIGHT;

    screen.rowbytes = width*4;
    screen.dims.width = width;
    screen.dims.height = height;
    screen.mem = (uint8_t*)malloc(screen.rowbytes * screen.dims.height);
    memset(screen.mem, 0, screen.rowbytes * screen.dims.height);

	SceneBuilder builder;
	if (argc > 1)
	{
		try {
			builder.parse_containers(argv[1]);
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
#if 1
    auto sdlTexture = SDL_CreateTexture(renderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_LockTexture(sdlTexture, NULL, reinterpret_cast<void**>(&screen.mem), reinterpret_cast<int*>(&screen.rowbytes));
    draw_scene(scene,screen);
    SDL_UnlockTexture(sdlTexture);
    SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
#else
    draw_scene(scene,screen);
    auto sdlTexture = SDL_CreateTexture(renderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_Rect sdlRect; sdlRect.x = 0; sdlRect.y = 0; sdlRect.w = SCREEN_WIDTH; sdlRect.h = SCREEN_HEIGHT;
    cout << "Texture created " << sdlTexture <<endl;
    int res = SDL_UpdateTexture(sdlTexture, NULL, screen.mem, screen.dims.height*screen.rowbytes);
    cout << "Texture update " << res <<endl;
    SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
#endif
#endif
	SDL_RenderPresent(renderer);

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
		//Rendering
#if 1
	    SDL_LockTexture(sdlTexture, NULL, reinterpret_cast<void**>(&screen.mem), reinterpret_cast<int*>(&screen.rowbytes));
	    draw_scene(scene,screen);
	    SDL_UnlockTexture(sdlTexture);
	    SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
#else
		SDL_RenderClear(renderer);
		//Draw the image
		draw_scene(scene, screen);
	    SDL_UpdateTexture(sdlTexture, &sdlRect, screen.mem, screen.rowbytes);
		//Update the screen
		SDL_RenderPresent(renderer);
#endif
	}
	//Clean up
	cleanup(renderer, window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
