/*
 * layerguy.cpp
 *
 *  Created on: Dec 17, 2018
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
#include <cstdlib>

using std::vector;

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

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;

int mrandom(int N)
{
	int r = rand();
	int m = RAND_MAX % N;
	int x = RAND_MAX - m;
	while (r > x)
	{
		r = rand();
	}
	return r % N;
}
int main(int argc, char** argv)
{
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

	auto texture = loadTexture("test.png", renderer);

	SDL_Rect r;
	vector<SDL_Rect> rects;
	srand(time(0));
	for (int i=0; i<1000; i++)
	{
		r.x = mrandom(1280);
		r.y = mrandom(720);
		r.w = mrandom(1280);
		r.h = mrandom(720);
		rects.push_back(r);
	}
	SDL_RenderClear(renderer);
	struct timespec start;
	clock_gettime(CLOCK_REALTIME, &start);
	for (auto rect : rects)
	{
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_RenderDrawRect(renderer, &rect);
		renderTexture(texture, renderer, rect.x, rect.y, rect.h, rect.y);
	}
	//Update the screen
	SDL_RenderPresent(renderer);
	struct timespec done;
	clock_gettime(CLOCK_REALTIME, &done);
	int borrow = done.tv_nsec < start.tv_nsec;
	int msec = (done.tv_sec - start.tv_sec - borrow)*1000 + (1000000000*borrow + done.tv_nsec - start.tv_nsec+500000)/1000000;
	printf("Render time %d msec\n", msec);
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
						//useClip = 0;
						break;
					case SDLK_2:
					case SDLK_KP_2:
						//useClip = 1;
						break;
					case SDLK_3:
					case SDLK_KP_3:
						//useClip = 2;
						break;
					case SDLK_4:
					case SDLK_KP_4:
						//useClip = 3;
						break;
					case SDLK_ESCAPE:
						quit = true;
						break;
					default:
						break;
				}
			}
		}
	}
}
