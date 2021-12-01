#!/bin/bash

#!/bin/bash

DISPLAY=:0 GST_DEBUG_DUMP_DOT_DIR=/home/simba/gstreamer_graph_files gst-launch-1.0 zedsrc camera-fps=15 camera-resolution=2 \
    sdk-verbose=1 stream-type=2 depth-mode=1 \
    ! autovideoconvert ! queue ! nvoverlaysink -e


DISPLAY=:0 GST_DEBUG_DUMP_DOT_DIR=$HOME/gstreamer_graph_files gst-launch-1.0 \
    zedsrc camera-fps=15 camera-resolution=2 stream-type=0 depth-mode=1 \
    sdk-verbose=1 stream-type=2 \
    ! queue \
    ! zeddemux is-depth=true name=demux \
    demux.src_left ! queue ! videoscale ! video/x-raw,width=672,height=376 ! autovideoconvert ! nvoverlaysink  \
    demux.src_aux ! queue ! autovideoconvert ! fakevideosink 

# split left view and convert it to nvmm, then show on nvoverlaysink

DISPLAY=:0 GST_DEBUG_DUMP_DOT_DIR=$HOME/gstreamer_graph_files gst-launch-1.0 \
    zedsrc camera-fps=15 camera-resolution=2 depth-mode=1 \
    sdk-verbose=1 stream-type=2 \
    ! video/x-raw,format=BGRA,width=1280,height=1440,framerate=15/1 \
    ! zeddemux is-depth=true name=demux \
    demux.src_left ! queue ! videoconvert ! nvvidconv ! nvoverlaysink sync=false \
    demux.src_aux ! queue ! autovideoconvert ! fakevideosink 

# display left view and depth map

DISPLAY=:0 GST_DEBUG_DUMP_DOT_DIR=$HOME/gstreamer_graph_files gst-launch-1.0 \
    zedsrc camera-fps=15 camera-resolution=2 depth-mode=1 \
    sdk-verbose=1 stream-type=4 \
    ! video/x-raw,format=BGRA,width=1280,height=1440,framerate=15/1 \
    ! queue ! videoconvert ! nvvidconv ! nvoverlaysink sync=false 

DISPLAY=:0 GST_DEBUG_DUMP_DOT_DIR=/home/simba/gstreamer_graph_files   gst-launch-1.0 \
    zedsrc ! videoconvert ! 'video/x-raw, format=(string)I420' ! omxh264enc ! rtph264pay name=pay0 pt=96  name=pay0

gst-zed-rtsp-launch -a 192.168.68.126 zedsrc ! videoconvert ! 'video/x-raw, format=(string)I420' ! omxh264enc ! rtph264pay name=pay0 pt=96  name=pay0
