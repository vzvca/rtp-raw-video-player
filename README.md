# rtp-raw-video-player

Plays RTP stream with raw video payload.

## Introduction

RAW video can be sent over RTP, the way the video is packed in RTP frames is described in RFC. Video is not encoded and images are streamed as YUV 4:2:0 frames. Each RTP frame contains portions of image scanline. This RTP video payload is rarely used because it consumes a lot of network bandwidth compared to other payloads like h264.

It makes t very easy to write a very simple video player with a few lines of C without any external dependencies. This is the purpose of this project.

The player will output RGB frames either diretly to `/dev/fb0` or to a file which is memory mapped. Another program can mmap the same file and render the video in a window, take snapshots, convert the video to another format ... A demo program displaying the video in a window is provided, it uses SDL2.

As raw video pixels are received in YCbCr color space, the output can be black and white (Y only is used which is very fast) or color. A compilation option (__BLACK_AND_WHITE__) can be used to build a player doing black and white rendering, by default is does color video. The pixel format conversion is pure software which is not very efficient but doesn't add any dependency. 


## Building

### Dependencies

The player itself doesn't have any dependency. SDL2 is required for the demo program rendering the video in a window. On debian systems, the following command install the required dependencies :

~~~~
apt-get install libsdl2-dev
~~~~

A RAW video RTP streamer is provided because. It's a small shell script running a gstreamer pipeline, which means that gstreamer needs to be installed too. On debian systems, use the following :

~~~~
apt-get install gstreamer1.0-plugins-base-doc gstreamer1.0-plugins-base gstreamer1.0-tools
~~~~

### Compiling

`make` will build the player and `make sdl-win` will build the SDL2 video renderer.


## Using the program

### Streaming 

Starts the gstreamer pipeline in a terminal

~~~~
./stream-raw.sh
~~~~

Remember to add the famous 'multicast route' on the interface used for streaming :

~~~~
sudo /sbin/route add -net 224.0.0.0 netmask 240.0.0.0 dev lo
~~~~

Starts the video decoder in a second terminal

~~~~
./vpl -i lo -o /tmp/video.img -w 720 -h 576
~~~~

In a third terminal, starts the video display program

~~~~
./sdl-win -i /tmp/video.img -w 720 -h 576
~~~~


