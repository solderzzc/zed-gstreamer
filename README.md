<h1 align="center">
  ZED GStreamer plugins
</h1>

<h4 align="center">GStreamer plugins for the ZED stereo camera family</h4>

<p align="center">
  <a href="#key-features">Key Features</a> •
  <a href="#build">Build</a> •
  <a href="#plugins-parameters">Plugins parameters</a> •
  <a href="#metadata">Metadata</a> •
  <a href="#pipeline-examples">Example pipelines</a> • 
  <a href="#rtsp-server">RTSP Server</a> • 
  <a href="#related">Related</a> •
  <a href="#license">License</a>
</p>
<br>

# Key Features
GStreamer plugin package for ZED Cameras. The package is composed of several plugins:

* [`zedsrc`](./gst-zed-src): acquires camera color image and depth map and pushes them in a GStreamer pipeline.
* [`zedmeta`](./gst-zed-meta): GStreamer library to define and handle the ZED metadata (Positional Tracking data, Sensors data, Detected Object data, Detected Skeletons data).
* [`zeddemux`](./gst-zed-demux): receives a composite `zedsrc` stream (`color left + color right` data or `color left + depth map` + metadata),
  processes the eventual depth data and pushes them in two separated new streams named `src_left` and `src_aux`. A third source pad is created for metadata to be externally processed.
* [`zeddatamux`](./gst-zed-data-mux): receive a video stream compatible with ZED caps and a ZED Data Stream generated by the `zeddemux` and adds metadata to the video stream. This is useful if metadata are removed by a filter that does not automatically propagate metadata
* [`zeddatacsvsink`](./gst-zed-data-csv-sink): example sink plugin that receives ZED metadata, extracts the Positional Tracking and the Sensors Data and save them in a CSV file.
* [`zedodoverlay`](./gst-zed-od-overlay): example transform filter plugin that receives ZED combined stream with metadata, extracts Object Detection information and draws the overlays on the oncoming filter
* [`RTSP Server`](./gst-zed-rtsp-server): application for Linux that instantiates an RTSP server from a text launch pipeline "gst-launch" like.

## Build

### Prerequisites

 * Stereo camera: [ZED 2](https://www.stereolabs.com/zed-2/), [ZED](https://www.stereolabs.com/zed/), [ZED Mini](https://www.stereolabs.com/zed-mini/)
 * CMake (v3.1+)
 * GStreamer 1.0

### Windows installation

 * Install the latest ZED SDK v3.3.x from the [official download page](https://www.stereolabs.com/developers/release/) [Optional to compile the `zedsrc` plugin to acquire data from a ZED camera device]
 * Install [Git](https://git-scm.com/) or download a ZIP archive
 * Install [CMake](https://cmake.org/)
 * Install a [GStreamer distribution (**both `runtime` and `development` installers**)](https://gstreamer.freedesktop.org/download/).
 * The installer should set the installation path via the `GSTREAMER_1_0_ROOT_X86_64` environment variable.
 * Add the path `%GSTREAMER_1_0_ROOT_X86_64%\bin` to the system variable `PATH`
 * Run the following commands from a terminal or command prompt, assuming CMake and Git are in your `PATH`.

     ```
     git clone https://github.com/stereolabs/zed-gstreamer.git
     cd zed-gstreamer
     mkdir build
     cd build
     cmake -G "Visual Studio 16 2019" ..
     cmake --build . --target INSTALL --config Release
     ```   

### Linux installation

#### Install prerequisites

* Install the latest ZED SDK v3.3.x from the [official download page](https://www.stereolabs.com/developers/release/)

* Update list of `apt` available packages

     `$ sudo apt update`

* Install GCC compiler and build tools

     `$ sudo apt install build-essential`

* Install CMake build system

     `$ sudo apt install cmake`

* Install GStreamer, the development packages and useful tools:

     `$ sudo apt install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav libgstrtspserver-1.0-0 gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev  libgstrtspserver-1.0-dev`

* [Optional] Install OpenCV to build the `zedodverlay` filter

     `$ sudo apt install libopencv-dev libopencv-contrib-dev`

#### Clone the repository

    $ git clone https://github.com/stereolabs/zed-gstreamer.git
    $ cd zed-gstreamer

#### Build

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release ..
    $ make # Note: do not use the `-j` flag because parallel build is not supported
    $ sudo make install

### Installation test

 * Check `ZED Video Source Plugin` installation inspecting its properties:

      `gst-inspect-1.0 zedsrc`

 * Check `ZED Video Demuxer` installation inspecting its properties:

      `gst-inspect-1.0 zeddemux`

* Check `ZED Data Mux Plugin` installation inspecting its properties:

     `gst-inspect-1.0 zeddatamux`

 * Check `ZED CSV Sink Plugin` installation inspecting its properties:

      `gst-inspect-1.0 zeddatacsvsink`

 * Check `ZED Object Detection Overlay Plugin` installation inspecting its properties:

      `gst-inspect-1.0 zedodoverlay`

## Plugins parameters

### `ZED Video Source Plugin` parameters
Most of the parameters follow the same name as the C++ API. Except that `_` is replaced by `-` to follow gstreamer common formatting.

 * `resolution`: stream resolution - {VGA (3), HD270 (2), HD1080 (1), HD2K (0)}
 * `framerate`: stream framerate - {15, 30, 60, 100}
 * `stream-type`: type of video stream - {Left image (0), Right image (1), Stereo couple (2), 16 bit depth (3), Left+Depth (4)}
 * `verbose`: SDK verbose mode - {TRUE, FALSE}
 * `flip`: flip streams vertically - {TRUE, FALSE, AUTO}
 * `camera-id`: camera ID - [0, 256]
 * `camera-sn`: camera serial number
 * `svo-file-path`: SVO file path for SVO input
 * `in-stream-ip-addr`: device(sender) IP address when using streaming input from ZED SDK
 * `in-stream-port `: IP port when using streaming input from ZED SDK
 * `min-depth`: Minimum depth value
 * `max-depth`: Maximum depth value
 * `disable-self-calib`: Disable the self calibration processing when the camera is opened - {TRUE, FALSE}
 * `depth-stability`: Enable depth stabilization - {TRUE, FALSE}
 * `pos-tracking`: Enable positional tracking - {TRUE, FALSE}
 * `cam-static `: Set to TRUE if the camera is static - {TRUE, FALSE}
 * `coord-system`: ZED SDK Coordinate System - {Image (0) - Left handed, Y up (1) - Right handed, Y up (2) - Right handed, Z up (3) - Left handed, Z up (4) - Right handed, Z up, X fwd (5)}
 * `od-enabled`: Enable Object Detection - {TRUE, FALSE}
 * `od-tracking`: Enable tracking for the detected objects - {TRUE, FALSE}
 * `od-detection-model`: Object Detection Model - {Multi class (0), Human Body Tracking FAST (1), Human Body Tracking ACCURATE (2)}
 * `od-confidence`: Minimum Detection Confidence - [0,100]

### `ZED Video Demuxer Plugin` parameters

 * `is-depth`: indicates if the bottom stream of a composite `stream-type` of the `ZED Video Source Plugin` is a color image (Right image) or a depth map.
 * `stream-data`: Enable binary data streaming on `src_data` pad - {TRUE, FALSE}

### `ZED Data CSV sink Plugin` parameters

 * `location`: Location of the CSV file to write
 * `append`: Append data to an already existing CSV file

## Metadata

The `zedsrc` plugin add metadata to the video stream containing information about the original frame size,
the camera position and orientatio, the sensors data and the object and skeleton detected by the Object Detection
module.
The `zeddatacsvsink` and `zedodoverlay` elements demonstrate how to handle, respectively, the sensors data and the
detected object data.
The `GstZedSrcMeta` structure is provided to handle the `zedmeta` metadata and it is available in the `gstzedmeta` library.

### GstZedSrcMeta structure

The GstZedSrcMeta is subdivided in four sub-structures:
 * `ZedInfo`: info about camera model, stream type and original stream size
 * `ZedPose`: position and orientation of the camera if positional tracking is enabled
 * `ZedSensors`: sensors data (only ZED-Mini and ZED2)
 * `ZedObjectData`: detected object information (only ZED2)

More details about the sub-structures are available in the [`gstzedmeta.h` file](./gst-zed-meta/gstzedmeta.h)

## Pipeline examples

### Local RGB stream + RGB rendering

* Linux: [`simple-fps_rendering.sh`](./scripts/linux/simple-fps_rendering.sh)
* Windows: [`simple-fps_rendering.bat`](./scripts/windows/simple-fps_rendering.bat)

```
    gst-launch-1.0 zedsrc ! queue ! autovideoconvert ! queue ! fpsdisplaysink
```

### Local 16 bit Depth stream + Depth rendering

* Linux: [`simple-depth-fps_rendering.sh`](./scripts/linux/simple-depth-fps_rendering.sh)
* Windows: [`simple-depth-fps_rendering.bat`](./scripts/windows/simple-depth-fps_rendering.bat)

```
    gst-launch-1.0 zedsrc stream-type=3 ! queue ! autovideoconvert ! queue ! fpsdisplaysink
```

### Local Left/Right stream + demux + double RGB rendering

* Linux: [`local-rgb_left_right-fps_rendering.sh`](./scripts/linux/local-rgb_left_right-fps_rendering.sh)
* Windows: [`local-rgb_left_right-fps_rendering.bat`](./scripts/windows/local-rgb_left_right-fps_rendering.bat)

```
    gst-launch-1.0 \
    zedsrc stream-type=2 ! queue ! \
    zeddemux is-depth=false name=demux \
    demux.src_left ! queue ! autovideoconvert ! fpsdisplaysink \
    demux.src_aux ! queue ! autovideoconvert ! fpsdisplaysink
```

### Local Left/Depth stream + demux + double streams rendering

* Linux: [`local-rgb_left_depth-fps_rendering.sh`](./scripts/linux/local-rgb_left_depth-fps_rendering.sh)
* Windows: [`local-rgb_left_depth-fps_rendering.bat`](./scripts/windows/local-rgb_left_depth-fps_rendering.bat)

```
    gst-launch-1.0 \
    zedsrc stream-type=4 ! queue ! \
    zeddemux name=demux \
    demux.src_left ! queue ! autovideoconvert ! fpsdisplaysink \
    demux.src_aux ! queue ! autovideoconvert ! fpsdisplaysink
```

### Local Left/Depth stream + demux + double streams rendering + data saving on CSV file

* Linux: [`local-rgb-depth-sens-csv.sh`](./scripts/linux/local-rgb-depth-sens-csv.sh)
* Windows: [`local-rgb-depth-sens-csv.bat`](./scripts/windows/local-rgb-depth-sens-csv.bat)

```
    gst-launch-1.0 \
    zedsrc stream-type=4 ! \
    zeddemux stream-data=TRUE name=demux \
    demux.src_left ! queue ! autovideoconvert ! fpsdisplaysink \
    demux.src_aux ! queue ! autovideoconvert ! fpsdisplaysink \
    demux.src_data ! queue ! zeddatacsvsink location="${HOME}/test_csv.csv" append=FALSE
```

### Local Left/Right stream + Multiclass Object Detection result displaying

* Linux: [`local-rgb-od_multi-overlay.sh`](./scripts/linux/local-rgb-od_multi-overlay.sh)
* Windows: [`local-rgb-od_multi-overlay.bat`](./scripts/windows/local-rgb-od_multi-overlay.bat)

```    
    gst-launch-1.0 \
    zedsrc stream-type=0 od-enabled=true od-detection-model=0 resolution=2 framerate=30 ! queue ! \
    zedodoverlay ! queue ! \
    autovideoconvert ! fpsdisplaysink
```

### Local Left/Right stream + Fast Skeleton Tracking result displaying

* Linux: [`local-rgb-skel_fast-overlay.sh`](./scripts/linux/local-rgb-skel_fast-overlay.sh)
* Windows: [`local-rgb-skel_fast-overlay.bat`](./scripts/windows/local-rgb-skel_fast-overlay.bat)

```    
    gst-launch-1.0 \
    zedsrc stream-type=2 od-enabled=true od-detection-model=1 resolution=0 framerate=15 ! queue ! \
    zedodoverlay ! queue ! \
    autovideoconvert ! fpsdisplaysink
```

### Local Left RGB stream + Accurate Skeleton Tracking result displaying

* Linux: [`local-rgb-skel_accurate-overlay.sh`](./scripts/linux/local-rgb-skel_accurate-overlay.sh)
* Windows: [`local-rgb-skel_accurate-overlay.bat`](./scripts/windows/local-rgb-skel_accurate-overlay.bat)

```
    gst-launch-1.0 \
    zedsrc stream-type=0 od-enabled=true od-detection-model=2 resolution=0 framerate=15  ! queue ! \
    zedodoverlay ! queue ! \
    autovideoconvert ! fpsdisplaysink
```

### Local Left/Depth stream + Fast Skeleton Tracking result displaying + demux + Skeleton Tracking result displaying + Depth displaying with FPS

* Linux: [`local-od-fps_overlay.sh`](./scripts/linux/local-od-fps_overlay.sh)
* Windows: [`local-od-fps_overlay.bat`](./scripts/windows/local-od-fps_overlay.bat)

```
    gst-launch-1.0 \
    zedsrc stream-type=4 resolution=2 framerate=30 od-enabled=true od-detection-model=1 ! \
    zeddemux name=demux \
    demux.src_left ! queue ! zedodoverlay ! queue ! autovideoconvert ! fpsdisplaysink \
    demux.src_aux ! queue ! autovideoconvert ! fpsdisplaysink
```

### Local Left/Depth stream + Fast Skeleton Tracking result displaying + demux + rescaling + remux + Skeleton Tracking result displaying + Depth displaying with FPS

* Linux: [`local-rgb-rescale-od-overlay.sh`](./scripts/linux/local-rgb-rescale-od-overlay.sh)
* Windows: [`local-rgb-rescale-od-overlay.bat`](./scripts/windows/local-rgb-rescale-od-overlay.bat)

```
    gst-launch-1.0 \
    zeddatamux name=mux \
    zedsrc stream-type=4 resolution=0 framerate=15 od-enabled=true od-detection-model=1 ! \
    zeddemux stream-data=true is-depth=true name=demux \
    demux.src_aux ! queue ! autovideoconvert ! videoscale ! video/x-raw,width=672,height=376 ! queue ! fpsdisplaysink \
    demux.src_data ! mux.sink_data \
    demux.src_left ! queue ! videoscale ! video/x-raw,width=672,height=376 ! mux.sink_video \
    mux.src ! queue ! zedodoverlay ! queue ! \
    autovideoconvert ! fpsdisplaysink
```

## RTSP Server

*Available only for Linux*

An application to start an RTSP server from a text pipeline (using the same sintax of the CLI command [`gst-launch-1.0`](https://gstreamer.freedesktop.org/documentation/tools/gst-launch.html)) is provided.

Usage:

```
   gst-zed-rtsp-launch [OPTION?] PIPELINE-DESCRIPTION
```

Help Options:
*  `-h`, `--help` -> Show help options.
*  `--help-all` -> Show all help options.
*  `--help-gst` -> Show GStreamer Options.

Application Options:
*  `-p`, `--port=PORT` -> Port to listen on (default: 8554).
*  `-a`, `--address=HOST` -> Host address (default: 127.0.0.1).

Example: 

```
   gst-zed-rtsp-launch zedsrc ! videoconvert ! 'video/x-raw, format=(string)I420' ! x264enc ! rtph264pay pt=96 name=pay0
```

It is mandatory to define at least one payload named `pay0`; it is possible to define multiple payloads using an increasing index (i.e. `pay1`, `pay2`, ...).

## Ready-To-Use scripts

Ready to use scripts are available in the scripts/ folder for windows and linux.

- local-od-fps_overlay : Left and Depth image rendering with object detection (FAST HUMAN BODY TRACKING) data (overlay).
- local-rgb-depth-sens-csv : Left and Depth image rendering and sensors data saved in csv file.
- local-rgb_left_depth-fps_rendering : Left and Depth image rendering.
- local-rgb_left_right-fps_rendering : Left and Right image rendering.
- local-rgb-od_multi-overlay : Left image rendering with object detection on overlay (MULTI_CLASS)
- local-rgb-rescale-od-overlay : Left and Depth image rendering with object detection with rescaling filter
- local-rgb-skel_accurate-overlay : Left image rendering with human body pose ACCURATE overlay
- local-rgb-skel_fast-overlay : Left/Right in top/bottom image rendering with human body pose FAST overlay

- [Linux only] udp and rtsp sender/receiver.

## Related

- [Stereolabs](https://www.stereolabs.com)
- [ZED 2 multi-sensor camera](https://www.stereolabs.com/zed-2/)
- [ZED SDK](https://www.stereolabs.com/developers/)

## License

This library is licensed under the LGPL License.

## Support
If you need assistance go to our Community site at https://community.stereolabs.com/
