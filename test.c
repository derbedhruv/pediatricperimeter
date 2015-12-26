/*
gcc test.c -I/usr/local/include/opencv -I/usr/local/include/opencv2 -L/usr/local/lib/ -g  -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_stitching -lSDL -lSDL_image
*/

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

static struct timeval tm1;

static inline void start()
{
    gettimeofday(&tm1, NULL);
}

static inline void stop()
{
    struct timeval tm2;
    gettimeofday(&tm2, NULL);

    unsigned long long t = 1000 * (tm2.tv_sec - tm1.tv_sec) + (tm2.tv_usec - tm1.tv_usec) / 1000;
    printf("%llu ms\n", t);
    printf("%lf fps\n", 80.0/t*1000);
}

int main(){
    
    // Open device in UNIX
    int fd;
    if((fd = open("/dev/video0",O_RDWR))<0){
        perror("Could not open camera");
        exit(-1);
    }

    // Retreive capabilities
    struct v4l2_capability capabilities = {0};
    if(ioctl(fd, VIDIOC_QUERYCAP, &capabilities) < 0){
        perror("Could not query camera for capabilities");
        exit(-1);
    }

    // Check if device can support video and video streaming
    if(!(capabilities.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING))){
        perror("Device does not support video");
        exit(-1);
    }

    puts("Video Camera detected...");

    // Choose pixel and frame format    
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 640;
    format.fmt.pix.height = 480;

    // Tell camera that this is the format we need
    if(ioctl(fd,VIDIOC_S_FMT,&format) < 0){
        perror("Error in configuring streaming format");
        exit(-1);
    }

    // Request buffers
    const int BUF_N = 2;
    struct v4l2_requestbuffers bufrequest = {0};
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP; // using linux mmap
    bufrequest.count = BUF_N;

    if(ioctl(fd,VIDIOC_REQBUFS,&bufrequest) < 0){
        perror("Buffer request failed");
        exit(-1);
    }

    char *buffer_start[BUF_N];
    struct v4l2_buffer bufferinfo[BUF_N];
    int buf_no;
    for(buf_no=0;buf_no<BUF_N; buf_no++){
        memset(&bufferinfo[buf_no],0,sizeof(struct v4l2_buffer));

        bufferinfo[buf_no].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo[buf_no].memory = V4L2_MEMORY_MMAP;
        bufferinfo[buf_no].index = buf_no;

        if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo[buf_no]) < 0){
            perror("Buffer query failed");
            exit(-1);
        }

        // Connect device buffer to system RAM
        buffer_start[buf_no] = mmap(NULL,bufferinfo[buf_no].length,
            PROT_READ | PROT_WRITE,MAP_SHARED,fd,bufferinfo[buf_no].m.offset);

        if(buffer_start == MAP_FAILED){
            perror("Failed to map buffer to RAM");
            exit(-1);
        }
     
        memset(buffer_start[buf_no], 0, bufferinfo[buf_no].length);
    }

    buf_no = 0;

    // Initialise everything.
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG);
     
    // Get the screen's surface.
    SDL_Surface* screen = SDL_SetVideoMode(
        format.fmt.pix.width,
        format.fmt.pix.height,
        32, SDL_HWSURFACE
    );

    SDL_RWops* buffer_stream; 
    SDL_Surface* picture;
    SDL_Rect position;
    position.x = 0;
    position.y = 0;
    position.w = 800;
    position.h = 600;

    // Put the buffer in the incoming queue.
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo[0]) < 0){
        perror("VIDIOC_QBUF");
        exit(1);
    }

    // Activate streaming
    int type = bufferinfo[0].type;
    if(ioctl(fd,VIDIOC_STREAMON,&type) < 0){
        perror("Could not switch on streaming");
        exit(-1);
    }

    int k;
    for(k=0;k<100;k++){
        int buf_no_next = (buf_no+1)%BUF_N;
        // Dequeue the buffer.
        if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo[buf_no]) < 0){
            perror("VIDIOC_QBUF");
            exit(1);
        }

        bufferinfo[buf_no_next].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo[buf_no_next].memory = V4L2_MEMORY_MMAP;

        // Queue the next one.
        if(ioctl(fd, VIDIOC_QBUF, &bufferinfo[buf_no_next]) < 0){
            perror("VIDIOC_QBUF");
            exit(1);
        }
     
        // Create a stream based on our buffer.
        buffer_stream = SDL_RWFromMem(buffer_start[buf_no], bufferinfo[buf_no].length);

        // Create a surface using the data coming out of the above stream.
        picture = IMG_Load_RW(buffer_stream,0);

        // Blit the surface and flip the screen.
        SDL_BlitSurface(picture, NULL, screen, &position);
        SDL_Flip(screen);

        printf("%d\n",k);

        SDL_Event event;
        /* Poll for events */
        while(SDL_PollEvent(&event)){   
            switch( event.type ){
                /* SDL_QUIT event (window close) */
                case SDL_QUIT:
                    goto clean_exit;
                default:
                    break;
            }
        }

        if(k==20)
            start();
        buf_no = buf_no_next;
    }

    stop();

clean_exit:

    // Deactivate streaming
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(1);
    }

    puts("Press any key to exit...");
    getchar();

    close(fd);
    // Free everything, and unload SDL & Co.
    SDL_FreeSurface(picture);
    SDL_RWclose(buffer_stream);
    IMG_Quit();
    SDL_Quit();
}