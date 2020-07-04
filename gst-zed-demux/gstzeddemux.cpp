/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) YEAR AUTHOR_NAME AUTHOR_EMAIL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-plugin
 *
 * FIXME:Describe plugin here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! plugin ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/gstbuffer.h>
#include <gst/gstcaps.h>

#include "gstzeddemux.h"

GST_DEBUG_CATEGORY_STATIC (gst_zeddemux_debug);
#define GST_CAT_DEFAULT gst_zeddemux_debug

/* Filter signals and args */
enum
{
    /* FILL ME */
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_IS_DEPTH
};

#define DEFAULT_PROP_IS_DEPTH    TRUE

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
                                                                    GST_PAD_SINK,
                                                                    GST_PAD_ALWAYS,
                                                                    GST_STATIC_CAPS( ("video/x-raw, " // Double stream
                                                                                      "format = (string) { BGRA }, "
                                                                                      "width = (int) { 672, 1280, 1920, 2208 } , "
                                                                                      "height =  (int) { 752, 1440, 2160, 2484 } , "
                                                                                      "framerate =  (fraction) { 15, 30, 60, 100 }") ) );

static GstStaticPadTemplate src_left_factory = GST_STATIC_PAD_TEMPLATE ("src_left",
                                                                        GST_PAD_SRC,
                                                                        GST_PAD_ALWAYS,
                                                                        GST_STATIC_CAPS( ("video/x-raw, "
                                                                                          "format = (string) { BGRA }, "
                                                                                          "width = (int) { 672, 1280, 1920, 2208 } , "
                                                                                          "height =  (int) { 376, 720, 1080, 1242 } , "
                                                                                          "framerate =  (fraction)  { 15, 30, 60, 100 }") ) );


static GstStaticPadTemplate src_aux_factory = GST_STATIC_PAD_TEMPLATE ("src_aux",
                                                                       GST_PAD_SRC,
                                                                       GST_PAD_ALWAYS,
                                                                       GST_STATIC_CAPS( ("video/x-raw, "
                                                                                         "format = (string) { GRAY16_LE }, "
                                                                                         "width = (int) { 672, 1280, 1920, 2208 } , "
                                                                                         "height =  (int) { 376, 720, 1080, 1242 } , "
                                                                                         "framerate =  (fraction)  { 15, 30, 60, 100 }"
                                                                                         ";"
                                                                                         "video/x-raw, "
                                                                                         "format = (string) { BGRA }, "
                                                                                         "width = (int) { 672, 1280, 1920, 2208 } , "
                                                                                         "height =  (int) { 376, 720, 1080, 1242 } , "
                                                                                         "framerate =  (fraction) { 15, 30, 60, 100 }") ) );


/* class initialization */
G_DEFINE_TYPE(GstZedDemux, gst_zeddemux, GST_TYPE_ELEMENT);

static void gst_zeddemux_set_property (GObject * object, guint prop_id,
                                       const GValue * value, GParamSpec * pspec);
static void gst_zeddemux_get_property (GObject * object, guint prop_id,
                                       GValue * value, GParamSpec * pspec);

static gboolean gst_zeddemux_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_zeddemux_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);

/* GObject vmethod implementations */

/* initialize the plugin's class */
static void
gst_zeddemux_class_init (GstZedDemuxClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);

    GST_DEBUG_OBJECT( gobject_class, "Class Init" );

    gobject_class->set_property = gst_zeddemux_set_property;
    gobject_class->get_property = gst_zeddemux_get_property;

    g_object_class_install_property (gobject_class, PROP_IS_DEPTH,
                                     g_param_spec_boolean ("is-depth", "Depth", "Aux source is GRAY16 depth",
                                                           DEFAULT_PROP_IS_DEPTH, G_PARAM_READWRITE));

    gst_element_class_set_static_metadata (gstelement_class,
                                           "ZED Video Demuxer",
                                           "Demuxer/Video",
                                           "Stereolabs ZED video demuxer",
                                           "Stereolabs <support@stereolabs.com>");

    gst_element_class_add_pad_template (gstelement_class,
                                        gst_static_pad_template_get (&src_left_factory));
    gst_element_class_add_pad_template (gstelement_class,
                                        gst_static_pad_template_get (&src_aux_factory));
    gst_element_class_add_pad_template (gstelement_class,
                                        gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void gst_zeddemux_init (GstZedDemux *filter)
{
    GST_DEBUG_OBJECT( filter, "Filter Init" );

    filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

    filter->srcpad_left = gst_pad_new_from_static_template( &src_left_factory, "src_left" );
    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad_left);

    filter->srcpad_aux = gst_pad_new_from_static_template( &src_aux_factory, "src_aux" );
    gst_element_add_pad(GST_ELEMENT (filter), filter->srcpad_aux);

    gst_pad_set_event_function( filter->sinkpad, GST_DEBUG_FUNCPTR(gst_zeddemux_sink_event) );
    gst_pad_set_chain_function( filter->sinkpad, GST_DEBUG_FUNCPTR(gst_zeddemux_chain) );

    filter->is_depth = DEFAULT_PROP_IS_DEPTH;
    filter->caps_left = nullptr;
    filter->caps_aux = nullptr;
    filter->buf_left = nullptr;
    filter->buf_aux = nullptr;
}

static void
gst_zeddemux_set_property (GObject * object, guint prop_id,
                           const GValue * value, GParamSpec * pspec)
{
    GstZedDemux *filter = GST_ZEDDEMUX (object);

    GST_DEBUG_OBJECT( filter, "Set property" );

    switch (prop_id) {
    case PROP_IS_DEPTH:
        filter->is_depth = g_value_get_boolean (value);
        GST_DEBUG( "Depth mode: %d", filter->is_depth );
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gst_zeddemux_get_property (GObject * object, guint prop_id,
                           GValue * value, GParamSpec * pspec)
{
    GstZedDemux *filter = GST_ZEDDEMUX (object);

    GST_DEBUG_OBJECT( filter, "Get property" );

    switch (prop_id) {
    case PROP_IS_DEPTH:
        g_value_set_boolean (value, filter->is_depth);
        GST_DEBUG( "Depth mode: %d", filter->is_depth );
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

/* GstElement vmethod implementations */

static gboolean set_out_caps( GstZedDemux* filter, GstCaps* sink_caps )
{
    GstVideoInfo vinfo_in;
    GstVideoInfo vinfo_left;
    GstVideoInfo vinfo_aux;

    GST_DEBUG_OBJECT( filter, "Sink caps %" GST_PTR_FORMAT, sink_caps);

    // ----> Caps left source
    if (filter->caps_left) {
        gst_caps_unref (filter->caps_left);
    }

    gst_video_info_from_caps(&vinfo_in, sink_caps);

    gst_video_info_init( &vinfo_left );
    gst_video_info_set_format( &vinfo_left, GST_VIDEO_FORMAT_BGRA,
                               vinfo_in.width, vinfo_in.height/2 );
    vinfo_left.fps_d = vinfo_in.fps_d;
    vinfo_left.fps_n = vinfo_in.fps_n;
    filter->caps_left = gst_video_info_to_caps(&vinfo_left);

    GST_DEBUG_OBJECT( filter, "Created left caps %" GST_PTR_FORMAT, filter->caps_left );
    if(gst_pad_set_caps(filter->srcpad_left, filter->caps_left)==FALSE)
    {
        return false;
    }

    //    filter->left_framesize = (guint) GST_VIDEO_INFO_SIZE (&vinfo_left);
    //    filter->buf_left = gst_buffer_new_and_alloc(filter->left_framesize);
    // <---- Caps left source

    // ----> Caps aux source
    if (filter->caps_aux) {
        gst_caps_unref (filter->caps_aux);
    }

    gst_video_info_from_caps(&vinfo_in, sink_caps);

    gst_video_info_init( &vinfo_aux );
    if(filter->is_depth)
    {
        gst_video_info_set_format( &vinfo_aux, GST_VIDEO_FORMAT_GRAY16_LE,
                                   vinfo_in.width, vinfo_in.height/2 );
    }
    else
    {
        gst_video_info_set_format( &vinfo_aux, GST_VIDEO_FORMAT_BGRA,
                                   vinfo_in.width, vinfo_in.height/2 );
    }
    vinfo_aux.fps_d = vinfo_in.fps_d;
    vinfo_aux.fps_n = vinfo_in.fps_n;
    filter->caps_aux = gst_video_info_to_caps(&vinfo_aux);

    GST_DEBUG_OBJECT( filter, "Created aux caps %" GST_PTR_FORMAT, filter->caps_aux );
    if(gst_pad_set_caps(filter->srcpad_aux, filter->caps_aux)==FALSE)
    {
        return false;
    }

    //    if(filter->buf_aux)
    //    {
    //        gst_buffer_unref(filter->buf_aux);
    //    }

    //    filter->aux_framesize = (guint) GST_VIDEO_INFO_SIZE (&vinfo_aux);
    //    filter->buf_aux = gst_buffer_new_and_alloc(filter->aux_framesize);
    // <---- Caps aux source

    return TRUE;
}

/* this function handles sink events */
static gboolean
gst_zeddemux_sink_event(GstPad* pad, GstObject* parent, GstEvent* event)
{
    GstZedDemux *filter;
    gboolean ret;

    filter = GST_ZEDDEMUX (parent);

    GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
                    GST_EVENT_TYPE_NAME (event), event);

    switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
        GST_DEBUG_OBJECT( filter, "Event CAPS" );
        GstCaps* caps;

        gst_event_parse_caps(event, &caps);
        /* do something with the caps */

        ret = set_out_caps( filter, caps );

        /* and forward */
        //ret = gst_pad_event_default (pad, parent, event);
        break;
    }
    default:
        ret = gst_pad_event_default (pad, parent, event);
        break;
    }
    return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn gst_zeddemux_chain(GstPad* pad, GstObject * parent, GstBuffer* buf )
{
    GstZedDemux *filter;

    filter = GST_ZEDDEMUX (parent);

    GST_DEBUG_OBJECT( filter, "Chain" );

    GstMapInfo map_in;
    GstMapInfo map_out_left;
    GstMapInfo map_out_aux;

    GstFlowReturn ret_left = GST_FLOW_ERROR;
    GstFlowReturn ret_aux = GST_FLOW_ERROR;

    GstClockTime timestamp = GST_CLOCK_TIME_NONE;

    timestamp = GST_BUFFER_TIMESTAMP (buf);
    GST_DEBUG ("timestamp %" GST_TIME_FORMAT, GST_TIME_ARGS (timestamp));

    GST_DEBUG_OBJECT( filter, "Processing ..." );
    if(gst_buffer_map(buf, &map_in, GST_MAP_READ))
    {
        gsize left_buf_size = map_in.size/2;
        gsize aux_buf_size = left_buf_size;
        if(filter->is_depth==TRUE)
        {
            GST_DEBUG ("Depth mode!" );
            aux_buf_size /=2; // RGBA -> GREY16 conversion required
        }

        GST_DEBUG ("Input buffer size %lu B", left_buf_size*2 );

        if( !filter->buf_left)
        {
            //filter->buf_left = gst_buffer_new_and_alloc(LEFT_buf_size);
            filter->buf_left = gst_buffer_new_allocate(NULL, left_buf_size, NULL ); // v1.0
        }

        filter->buf_left = gst_buffer_make_writable( filter->buf_left );

        if( !GST_IS_BUFFER(filter->buf_left) )
        {
            GST_DEBUG ("Left buffer not writable");
            return GST_FLOW_ERROR;
        }

        if(filter->buf_left)
        {
            if( gst_buffer_map(filter->buf_left, &map_out_left, GST_MAP_WRITE) )
            {
                GST_DEBUG ("Copying left buffer %lu B", map_out_left.size );
                memcpy(map_out_left.data, map_in.data, left_buf_size);

                GST_DEBUG ("Left buffer set timestamp" );
                GST_BUFFER_PTS(filter->buf_left) = GST_BUFFER_PTS (buf);
                GST_BUFFER_DTS(filter->buf_left) = GST_BUFFER_DTS (buf);
                GST_BUFFER_TIMESTAMP(filter->buf_left) = GST_BUFFER_TIMESTAMP (buf);

                GST_DEBUG ("Left buffer push" );
                ret_left = gst_pad_push(filter->srcpad_left, filter->buf_left);

                if( ret_left != GST_FLOW_OK )
                {
                    GST_DEBUG_OBJECT( filter, "Error pushing left buffer: %s", gst_flow_get_name (ret_left));
                    return ret_left;
                }

                GST_DEBUG ("Left buffer unmap" );
                gst_buffer_unmap(filter->buf_left, &map_out_left);
            }
        }

        if( !filter->buf_aux)
        {
            //filter->buf_aux = gst_buffer_new_and_alloc(aux_buf_size);
            filter->buf_aux = gst_buffer_new_allocate(NULL, aux_buf_size, NULL ); // v1.0
        }

        filter->buf_aux = gst_buffer_make_writable( filter->buf_aux );

        if( !GST_IS_BUFFER(filter->buf_aux) )
        {
            GST_DEBUG ("Aux buffer not writable");
            return GST_FLOW_ERROR;
        }

        if( gst_buffer_map(filter->buf_aux, &map_out_aux, GST_MAP_WRITE) )
        {
            if( filter->is_depth == FALSE )
            {
                GST_DEBUG ("Copying aux buffer %lu B", map_out_aux.size );
                memcpy(map_out_aux.data, map_in.data+map_out_left.size, aux_buf_size);
            }
            else
            {
                GST_DEBUG ("Converting aux buffer %lu B", map_out_aux.size );

                guint32* gst_in_data = (guint32*)(map_in.data + map_out_left.size);
                guint16* gst_out_data = (guint16*)(map_out_aux.data);

                for (unsigned long i = 0; i < map_out_aux.size/2; i++)
                {
                    *(gst_out_data++) = static_cast<guint16>(*(gst_in_data++));
                }
            }

            GST_DEBUG ("Aux buffer set timestamp" );
            GST_BUFFER_PTS(filter->buf_aux) = GST_BUFFER_PTS (buf);
            GST_BUFFER_DTS(filter->buf_aux) = GST_BUFFER_DTS (buf);
            GST_BUFFER_TIMESTAMP(filter->buf_aux) = GST_BUFFER_TIMESTAMP (buf);


            GST_DEBUG ("Aux buffer push" );
            ret_aux = gst_pad_push(filter->srcpad_aux, filter->buf_aux);

            if( ret_aux != GST_FLOW_OK )
            {
                GST_DEBUG_OBJECT( filter, "Error pushing aux buffer: %s", gst_flow_get_name (ret_aux));
                return ret_aux;
            }

            GST_DEBUG ("Aux buffer unmap" );
            gst_buffer_unmap(filter->buf_aux, &map_out_aux);
        }
    }
    GST_DEBUG ("... processed" );

    if(ret_left==GST_FLOW_OK && ret_aux==GST_FLOW_OK)
    {
        GST_DEBUG_OBJECT( filter, "Chain OK" );
        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean plugin_init (GstPlugin * plugin)
{
    /* debug category for fltering log messages
   *
   * exchange the string 'Template plugin' with your description
   */
    GST_DEBUG_CATEGORY_INIT (gst_zeddemux_debug, "zeddemux",
                             0, "debug category for zeddemux element");

    gst_element_register (plugin, "zeddemux", GST_RANK_NONE,
                          gst_zeddemux_get_type());

    return TRUE;
}

/* gstreamer looks for this structure to register plugins
 *
 * exchange the string 'Template plugin' with your plugin description
 */
GST_PLUGIN_DEFINE( GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   zeddemux,
                   "ZED stream demuxer",
                   plugin_init,
                   GST_PACKAGE_VERSION,
                   GST_PACKAGE_LICENSE,
                   GST_PACKAGE_NAME,
                   GST_PACKAGE_ORIGIN)