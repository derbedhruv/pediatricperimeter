#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <time.h>
#include <limits.h>

#include <opencv2/opencv.hpp>

#include "SDL.h"

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 360;
const int FRAME_RATE = 60;

// fps counter begin
time_t start, end;
int counter = 0;
double sec;
double fps;
// fps counter end

SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_bool done = SDL_FALSE;

cv::VideoCapture camera;

void copy_Frame(SDL_Texture *texture){
    SDL_Color color = {0,0,0,255};
    cv::Mat frame;

    Uint32 *dst;
    int row, col;
    void *pixels;
    int pitch;

    if (SDL_LockTexture(texture, NULL, &pixels, &pitch) < 0) {
		std::cerr << "Couldn't lock texture" << std::endl;
    }
    
    camera.read(frame);
    
    uchar *data = frame.data;
    for (row = 0; row < FRAME_HEIGHT; ++row) {
        dst = (Uint32*)((Uint8*)pixels + row * pitch);
        for (col = 0; col < FRAME_WIDTH; col+=1) {
			color.b = *(data + frame.step[0]*row + frame.step[1]*col + 0);
			color.g = *(data + frame.step[0]*row + frame.step[1]*col + 1);
			color.r = *(data + frame.step[0]*row + frame.step[1]*col + 2);
            *dst++ = (0xFF000000|(color.r<<16)|(color.g<<8)|color.b);
        }
    }
    
	SDL_UnlockTexture(texture);
}

void loop() {
    SDL_Event event;

	if (counter == 0){
		time(&start);
	}

//     while (SDL_PollEvent(&event)) {
//         switch (event.type) {
// 			case SDL_KEYDOWN:
//             	if (event.key.keysym.sym == SDLK_ESCAPE) {
//                 	done = SDL_TRUE;
//             	}
//             	break;
//         	case SDL_QUIT:
//             	done = SDL_TRUE;
//             	break;
//         }
//         //SDL_Delay(0.1);
//     }

	copy_Frame(texture);

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    
	time(&end);
	counter++;
	if (counter > 30){
		sec = difftime(end, start);
		fps = counter/sec;
		printf("%.2f fps\n", fps);
	}
}

int main(){
    SDL_Window *window;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Couldn't initialize SDL" << std::endl;
        return 1;
    }
    
    /* Create the window and renderer */
    window = SDL_CreateWindow("PediPeri",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              FRAME_WIDTH, FRAME_WIDTH,
                              SDL_WINDOW_RESIZABLE);
    
    if (!window) {
        std::cerr << "Couldn't set create window" << std::endl;
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        std::cerr << "Couldn't set create renderer" << std::endl;
        return 1;
    }
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FRAME_WIDTH, FRAME_HEIGHT);
    if (!texture) {
        std::cerr << "Couldn't set create texture" << std::endl;
        return 1;
    }
    
    /* Set video capture properties */
    camera.open(0);
    camera.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
    camera.set(CV_CAP_PROP_FPS,FRAME_RATE);
    camera.set(CV_CAP_PROP_CONVERT_RGB,true);
    
    while (!done){
    	loop();
    }
    
    camera.release();

	SDL_Quit();
    SDL_DestroyRenderer(renderer);
    return 0;
    
}