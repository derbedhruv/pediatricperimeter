#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <limits.h>

#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <chrono>

#include "SDL.h"

const int FRAME_WIDTH = 480;
const int FRAME_HEIGHT = 360;
const int FRAME_RATE = 120;

int frameCount = 0;

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

struct Node{
	private:
		std::atomic<Node *> next; //prevents concurrent RW/WW 
	public:
		cv::Mat frame;
		int id;
		
		Node(){
			next.store(NULL);
			id = -1;
		}
	
		void append(Node *newNode){
			this->next.store(newNode);
		}
		
		inline Node *get_next(){
			return this->next.load();
		}
};

Node *frame_captures;
Node *tail;

void frameWrite(const std::string &filename = "output.avi"){
	cv::VideoWriter diskWriter;
	cv::Size size(FRAME_WIDTH,FRAME_HEIGHT);
	Node *diskHead = frame_captures;
	diskWriter.open(filename,cv::VideoWriter::fourcc('F','M','P','4'),30,size,true);
	if(!diskWriter.isOpened()){
		std::cerr << "Could not open the video for writing" << std::endl;
		exit(-1);
	}
	while(1){
		if(diskHead->get_next()!=NULL){
			printf("Write: %d\n",diskHead->id);
			diskWriter << diskHead->frame;
			Node *temp = diskHead;
			diskHead = diskHead->get_next();
			delete temp;
		}
		else if(done){
			return;
		}
		else{
		std::this_thread::sleep_for(std::chrono::duration<int,std::centi>(300)); // sleep for 0.5s
		}
	}
}

void copy_Frame(SDL_Texture *texture){
	/* Capture frame from OpenCV, copy to texture memory */
    SDL_Color color = {0,0,0,255};

    Uint32 *dst;
    int row, col;
    void *pixels;
    int pitch;

    if (SDL_LockTexture(texture, NULL, &pixels, &pitch) < 0) {
		std::cerr << "Couldn't lock texture" << std::endl;
    }
    
    cv::Mat &frame = tail->frame;
    camera.read(frame);
    tail->append(new Node);
    tail->id = frameCount++;
    printf("Capture: %d\n",tail->id);
    tail = tail->get_next();
    
    uchar *data = frame.data;
    for (row = 0; row < FRAME_HEIGHT; ++row) {
        dst = (Uint32*)((Uint8*)pixels + row * pitch);
        for (col = 0; col < FRAME_WIDTH; col+=1) {
			color.b = *(data + frame.step[0]*row + frame.step[1]*col + 0);
			color.g = *(data + frame.step[0]*row + frame.step[1]*col + 1);
			color.r = *(data + frame.step[0]*row + frame.step[1]*col + 2);
            *dst++ = (0xFF000000|(color.r<<16)|(color.g<<8)|color.b); // Alpha R G B 
        }
    }
    
	SDL_UnlockTexture(texture);
}

void loop() {
	/* Main loop for frame capture,  render */
    SDL_Event event;

	if (counter == 0){
		start = clock();
	}

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
			case SDL_KEYDOWN:
            	if (event.key.keysym.sym == SDLK_ESCAPE) {
                	done = SDL_TRUE;
            	}
            	break;
        	case SDL_QUIT:
            	done = SDL_TRUE;
            	break;
        }
    }

	copy_Frame(texture); // copy to texture

    SDL_RenderCopy(renderer, texture, NULL, NULL); // Upload to GPU
    SDL_RenderPresent(renderer);
    
	counter = (counter+1)%120;
	if (counter == 119){
		end = clock();
		//sec = difftime(end, start);
		sec = (double)(end - start)/CLOCKS_PER_SEC;
		fps = 120.0/sec;
		printf("%f --- %.2f fps\n",sec, fps);
	}
}

int main(){
	cv::Mat frame;
    /* Set video capture properties */
    camera.open(0);
    camera.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
    //camera.set(CV_CAP_PROP_FPS,FRAME_RATE);
    camera.set(CV_CAP_PROP_CONVERT_RGB,true);
    
    std::chrono::steady_clock::time_point begin,end;
    for(int i=0;i<100;i++){
    	camera.read(frame);
    	if(i==19){
    		begin = std::chrono::steady_clock::now();
    	}
    	std::cout << i << std::endl;
    }
    end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count() << std::endl;
    std::cout << 80.0*1000000/std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << std::endl;
    camera.release();
    exit(0);

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
    
    // Request a streaming texture (allocated memory for streaming textures on GPU is close to CPU bus for high frame rate)
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FRAME_WIDTH, FRAME_HEIGHT);
    if (!texture) {
        std::cerr << "Couldn't set create texture" << std::endl;
        return 1;
    }
    
	frame_captures = new Node;
	tail = frame_captures;
 
    std::thread diskWriting(frameWrite,"output.avi");
    
    /* Begin capture */
    while (!done){
    	loop();
    }   
    
	/* Release stuff */
    camera.release();
	SDL_Quit();
    SDL_DestroyRenderer(renderer);

    diskWriting.join();

    return 0;
    
}