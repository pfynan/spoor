/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2008, 2011, Nils Hasler, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

// Author: Nils Hasler <hasler@mpi-inf.mpg.de>
//
//         Max-Planck-Institut Informatik
//
// this implementation was inspired by gnash's gstreamer interface

// Butchered up to work with network streams by Phillip Fynan

//
// use GStreamer to read a video
//

#include "cap_gstreamer.h"
#include <unistd.h>
#include <string.h>
#include <map>

#ifdef NDEBUG
#define CV_WARN(message)
#else
#define CV_WARN(message) fprintf(stderr, "warning: %s (%s:%d)\n", message, __FILE__, __LINE__)
#endif

static cv::Mutex gst_initializer_mutex;

class gst_initializer
{
public:
    static void init()
    {
        gst_initializer_mutex.lock();
        static gst_initializer init;
        gst_initializer_mutex.unlock();
    }
private:
    gst_initializer()
    {
        if(!gst_is_initialized())
            gst_init(NULL, NULL);
        gst_debug_set_default_threshold(GST_LEVEL_WARNING);
    }
};

/* Functions below print the Capabilities in a human-friendly format */
static gboolean print_field (GQuark field, const GValue * value, gpointer pfx) {
  gchar *str = gst_value_serialize (value);
   
  g_print ("%s  %15s: %s\n", (gchar *) pfx, g_quark_to_string (field), str);
  g_free (str);
  return TRUE;
}
   
static void print_caps (const GstCaps * caps, const gchar * pfx) {
  guint i;
   
  g_return_if_fail (caps != NULL);
   
  if (gst_caps_is_any (caps)) {
    g_print ("%sANY\n", pfx);
    return;
  }
  if (gst_caps_is_empty (caps)) {
    g_print ("%sEMPTY\n", pfx);
    return;
  }
   
  for (i = 0; i < gst_caps_get_size (caps); i++) {
    GstStructure *structure = gst_caps_get_structure (caps, i);
     
    g_print ("%s%s\n", pfx, gst_structure_get_name (structure));
    gst_structure_foreach (structure, print_field, (gpointer) pfx);
  }
}
   
/* Prints information about a Pad Template, including its Capabilities */
static void print_pad_templates_information (GstElementFactory * factory) {
  const GList *pads;
  GstStaticPadTemplate *padtemplate;
   
  g_print ("Pad Templates for %s:\n", gst_element_factory_get_longname (factory));
  if (!factory->numpadtemplates) {
    g_print ("  none\n");
    return;
  }
   
  pads = factory->staticpadtemplates;
  while (pads) {
    padtemplate = (GstStaticPadTemplate *) (pads->data);
    pads = g_list_next (pads);
     
    if (padtemplate->direction == GST_PAD_SRC)
      g_print ("  SRC template: '%s'\n", padtemplate->name_template);
    else if (padtemplate->direction == GST_PAD_SINK)
      g_print ("  SINK template: '%s'\n", padtemplate->name_template);
    else
      g_print ("  UNKNOWN!!! template: '%s'\n", padtemplate->name_template);
     
    if (padtemplate->presence == GST_PAD_ALWAYS)
      g_print ("    Availability: Always\n");
    else if (padtemplate->presence == GST_PAD_SOMETIMES)
      g_print ("    Availability: Sometimes\n");
    else if (padtemplate->presence == GST_PAD_REQUEST) {
      g_print ("    Availability: On request\n");
    } else
      g_print ("    Availability: UNKNOWN!!!\n");
     
    if (padtemplate->static_caps.string) {
      g_print ("    Capabilities:\n");
      print_caps (gst_static_caps_get (&padtemplate->static_caps), "      ");
    }
     
    g_print ("\n");
  }
}
   
/* Shows the CURRENT capabilities of the requested pad in the given element */
static void print_pad_capabilities (GstElement *element, gchar *pad_name) {
  GstPad *pad = NULL;
  GstCaps *caps = NULL;
   
  /* Retrieve pad */
  pad = gst_element_get_static_pad (element, pad_name);
  if (!pad) {
    g_printerr ("Could not retrieve pad '%s'\n", pad_name);
    return;
  }
   
  /* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) */
  caps = gst_pad_get_negotiated_caps (pad);
  if (!caps)
    caps = gst_pad_get_caps_reffed (pad);
   
  /* Print and free */
  g_print ("Caps for the %s pad:\n", pad_name);
  print_caps (caps, "      ");
  gst_caps_unref (caps);
  gst_object_unref (pad);
}

void CvCapture_GStreamer::init()
{
    pipeline=0;
    frame=0;
    buffer=0;
    frame=0;

}

void CvCapture_GStreamer::handleMessage()
{
    GstBus* bus = gst_element_get_bus(pipeline);

    while(gst_bus_have_pending(bus)) {
        GstMessage* msg = gst_bus_pop(bus);

//        printf("Got %s message\n", GST_MESSAGE_TYPE_NAME(msg));

        switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_STATE_CHANGED:
            GstState oldstate, newstate, pendstate;
            gst_message_parse_state_changed(msg, &oldstate, &newstate, &pendstate);
//            printf("state changed from %d to %d (%d)\n", oldstate, newstate, pendstate);
            break;
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(msg, &err, &debug);

            fprintf(stderr, "GStreamer Plugin: Embedded video playback halted; module %s reported: %s\n",
                  gst_element_get_name(GST_MESSAGE_SRC (msg)), err->message);

            g_error_free(err);
            g_free(debug);

            gst_element_set_state(pipeline, GST_STATE_NULL);

            break;
            }
        case GST_MESSAGE_EOS:
//            CV_WARN("NetStream has reached the end of the stream.");

            break;
        default:
//            CV_WARN("unhandled message\n");
            break;
        }

        gst_message_unref(msg);
    }

    gst_object_unref(GST_OBJECT(bus));
}

//
// start the pipeline, grab a buffer, and pause again
//
bool CvCapture_GStreamer::grabFrame()
{

    if(!pipeline)
        return false;

    if(gst_app_sink_is_eos(GST_APP_SINK(sink)))
        return false;

    if(buffer)
        gst_buffer_unref(buffer);
    handleMessage();

    buffer = gst_app_sink_pull_buffer(GST_APP_SINK(sink));


    return true;
}

//
// decode buffer
//
IplImage * CvCapture_GStreamer::retrieveFrame(int)
{
    if(!buffer)
        return 0;

    if(!frame) {
        gint height, width;
        GstCaps *buff_caps = gst_buffer_get_caps(buffer);
        assert(gst_caps_get_size(buff_caps) == 1);
        GstStructure* structure = gst_caps_get_structure(buff_caps, 0);

        if(!gst_structure_get_int(structure, "width", &width) ||
           !gst_structure_get_int(structure, "height", &height))
            return 0;

        frame = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 3);
        gst_caps_unref(buff_caps);
    }

    // no need to memcpy, just use gstreamer's buffer :-)
    frame->imageData = (char *)GST_BUFFER_DATA(buffer);
    //memcpy (frame->imageData, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE (buffer));
    //gst_buffer_unref(buffer);
    //buffer = 0;
    return frame;
}

void CvCapture_GStreamer::restartPipeline()
{
    CV_FUNCNAME("icvRestartPipeline");

    __BEGIN__;

    printf("restarting pipeline, going to ready\n");

    if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_READY) ==
       GST_STATE_CHANGE_FAILURE) {
        CV_ERROR(CV_StsError, "GStreamer: unable to start pipeline\n");
        return;
    }

    printf("ready, relinking\n");

    gst_element_unlink(uridecodebin, color);
    printf("filtering with %s\n", gst_caps_to_string(caps));
    gst_element_link_filtered(uridecodebin, color, caps);

    printf("relinked, pausing\n");

    if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING) ==
       GST_STATE_CHANGE_FAILURE) {
        CV_ERROR(CV_StsError, "GStreamer: unable to start pipeline\n");
        return;
    }

    printf("state now paused\n");

     __END__;
}

void CvCapture_GStreamer::setFilter(const char *property, int type, int v1, int v2)
{

    if(!caps) {
        if(type == G_TYPE_INT)
            caps = gst_caps_new_simple("video/x-raw-rgb", property, type, v1, NULL);
        else
            caps = gst_caps_new_simple("video/x-raw-rgb", property, type, v1, v2, NULL);
    } else {
        //printf("caps before setting %s\n", gst_caps_to_string(caps));
        if(type == G_TYPE_INT)
            gst_caps_set_simple(caps, "video/x-raw-rgb", property, type, v1, NULL);
        else
            gst_caps_set_simple(caps, "video/x-raw-rgb", property, type, v1, v2, NULL);
    }

    restartPipeline();
}

void CvCapture_GStreamer::removeFilter(const char *filter)
{
    if(!caps)
        return;

    GstStructure *s = gst_caps_get_structure(caps, 0);
    gst_structure_remove_field(s, filter);

    restartPipeline();
}


//
// connect uridecodebin dynamically created source pads to colourconverter
//
void CvCapture_GStreamer::newPad(GstElement * /*uridecodebin*/,
                             GstPad     *pad,
                             gpointer    data)
{
    GstPad *sinkpad;
    GstElement *color = (GstElement *) data;


    sinkpad = gst_element_get_static_pad (color, "sink");

//  printf("linking dynamic pad to colourconverter %p %p\n", uridecodebin, pad);

    gst_pad_link (pad, sinkpad);

    gst_object_unref (sinkpad);
}

bool CvCapture_GStreamer::open( int type, const char* filename )
{
    close();
    CV_FUNCNAME("cvCaptureFromCAM_GStreamer");

    __BEGIN__;

    gst_initializer::init();


//    if(!isInited) {
//        printf("gst_init\n");
//        gst_init (NULL, NULL);

//        gst_debug_set_active(TRUE);
//        gst_debug_set_colored(TRUE);
//        gst_debug_set_default_threshold(GST_LEVEL_WARNING);

//        isInited = true;
//    }
    bool stream = false;
    bool manualpipeline = false;
    char *uri = NULL;
    uridecodebin = NULL;
    
    /*
    if(type != CV_CAP_GSTREAMER_FILE) {
        close();
        return false;
    }
    */

    if(!gst_uri_is_valid(filename)) {
        uri = realpath(filename, NULL);
        stream=false;
        if(uri) {
            uri = g_filename_to_uri(uri, NULL, NULL);
            if(!uri) {
                CV_WARN("GStreamer: Error opening file\n");
                close();
                return false;
            }
        } else {
            GError *err = NULL;
            //uridecodebin = gst_parse_bin_from_description(filename, FALSE, &err);
            uridecodebin = gst_parse_launch(filename, &err);
            if(!uridecodebin) {
                CV_WARN("GStreamer: Error opening bin\n");
                close();
                return false;
            }
            stream = true;
            manualpipeline = true;
        }
    } else {
        stream = true;
        uri = g_strdup(filename);
    }

    if(!uridecodebin) {
        uridecodebin = gst_element_factory_make ("uridecodebin", NULL);
        g_object_set(G_OBJECT(uridecodebin),"uri",uri, NULL);
        if(!uridecodebin) {
            CV_WARN("GStreamer: Failed to create uridecodebin\n");
            close();
            return false;
        }
    }

    if(manualpipeline) {
        GstIterator *it = gst_bin_iterate_sinks(GST_BIN(uridecodebin));
        if(gst_iterator_next(it, (gpointer *)&sink) != GST_ITERATOR_OK) {
        CV_ERROR(CV_StsError, "GStreamer: cannot find appsink in manual pipeline\n");
        return false;
        }

    pipeline = uridecodebin;
    } else {
    pipeline = gst_pipeline_new (NULL);

        color = gst_element_factory_make("ffmpegcolorspace", NULL);
        sink = gst_element_factory_make("appsink", NULL);

        gst_bin_add_many(GST_BIN(pipeline), uridecodebin, color, sink, NULL);
        g_signal_connect(uridecodebin, "pad-added", G_CALLBACK(newPad), color);

        if(!gst_element_link(color, sink)) {
            CV_ERROR(CV_StsError, "GStreamer: cannot link color -> sink\n");
            gst_object_unref(pipeline);
            return false;
        }
    }

    gst_app_sink_set_max_buffers (GST_APP_SINK(sink), 1);
    gst_app_sink_set_drop (GST_APP_SINK(sink), stream);
    caps = gst_caps_new_simple("video/x-raw-rgb",
                               "red_mask",   G_TYPE_INT, 0x0000FF,
                               "green_mask", G_TYPE_INT, 0x00FF00,
                               "blue_mask",  G_TYPE_INT, 0xFF0000,
                               NULL);
    gst_app_sink_set_caps(GST_APP_SINK(sink), caps);
    gst_caps_unref(caps);

    if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_READY) ==
       GST_STATE_CHANGE_FAILURE) {
        CV_WARN("GStreamer: unable to set pipeline to ready\n");
        gst_object_unref(pipeline);
        return false;
    }

    if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING) ==
       GST_STATE_CHANGE_FAILURE) {
        gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
        CV_WARN("GStreamer: unable to set pipeline to playing\n");
        gst_object_unref(pipeline);
        return false;
    }



    handleMessage();

    __END__;

    return true;
}


void CvVideoWriter_GStreamer::init()
{
    encs[CV_FOURCC('D','R','A','C')]=(char*)"diracenc";
    encs[CV_FOURCC('H','F','Y','U')]=(char*)"ffenc_huffyuv";
    encs[CV_FOURCC('J','P','E','G')]=(char*)"jpegenc";
    encs[CV_FOURCC('M','J','P','G')]=(char*)"jpegenc";
    encs[CV_FOURCC('M','P','1','V')]=(char*)"mpeg2enc";
    encs[CV_FOURCC('M','P','2','V')]=(char*)"mpeg2enc";
    encs[CV_FOURCC('T','H','E','O')]=(char*)"theoraenc";
    encs[CV_FOURCC('V','P','8','0')]=(char*)"vp8enc";
    encs[CV_FOURCC('H','2','6','4')]=(char*)"x264enc";
    encs[CV_FOURCC('X','2','6','4')]=(char*)"x264enc";
    encs[CV_FOURCC('X','V','I','D')]=(char*)"xvidenc";
    encs[CV_FOURCC('F','F','Y','U')]=(char*)"y4menc";
    //encs[CV_FOURCC('H','F','Y','U')]=(char*)"y4menc";
    pipeline=0;
    buffer=0;
}
void CvVideoWriter_GStreamer::close()
{
    if (pipeline) {
        gst_app_src_end_of_stream(GST_APP_SRC(source));
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (pipeline));
    }
}
bool CvVideoWriter_GStreamer::open( const char * pipeline_string,
        double fps, CvSize frameSize)
{
    CV_FUNCNAME("CvVideoWriter_GStreamer::open");

    __BEGIN__;

    assert (pipeline_string);
    assert (fps > 0);
    assert (frameSize.width > 0  &&  frameSize.height > 0);
    std::map<int,char*>::iterator encit;

    gst_initializer::init();

    close();


    pipeline = gst_parse_launch(pipeline_string, NULL);

    GstIterator *it = gst_bin_iterate_sources(GST_BIN(pipeline));


    while(1) {
        // I feel dirty...
        if(gst_iterator_next(it, (gpointer *)&source) != GST_ITERATOR_OK) {
            CV_ERROR(CV_StsError, "GStreamer: cannot find appsrc in manual pipeline\n");
            return false;
        }

        if(GST_IS_APP_SRC(source))
            break;
    }


    GstCaps* caps;
    //if (is_color) {
        input_pix_fmt=1;
        caps= gst_video_format_new_caps(GST_VIDEO_FORMAT_BGR,
                                        frameSize.width,
                                        frameSize.height,
                                        (int) (fps * 1000),
                                        1000,
                                        1,
                                        1);
/*
    }
    else  {
        input_pix_fmt=0;
        caps= gst_caps_new_simple("video/x-raw-gray",
                                  "width", G_TYPE_INT, frameSize.width,
                                  "height", G_TYPE_INT, frameSize.height,
                                  "framerate", GST_TYPE_FRACTION, int(fps),1,
                                  "bpp",G_TYPE_INT,8,
                                  "depth",G_TYPE_INT,8,
                                  NULL);
    }
*/

    gst_app_src_set_caps(GST_APP_SRC(source), caps);


    if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {
            CV_ERROR(CV_StsError, "GStreamer: cannot put pipeline to play\n");
    }


    __END__;
    return true;
}
bool CvVideoWriter_GStreamer::writeFrame( const IplImage * image )
{

    CV_FUNCNAME("CvVideoWriter_GStreamer::writerFrame");

    __BEGIN__;
    if (input_pix_fmt == 1) {
        if (image->nChannels != 3 || image->depth != IPL_DEPTH_8U) {
            CV_ERROR(CV_StsUnsupportedFormat, "cvWriteFrame() needs images with depth = IPL_DEPTH_8U and nChannels = 3.");
        }
    }
    else if (input_pix_fmt == 0) {
        if (image->nChannels != 1 || image->depth != IPL_DEPTH_8U) {
            CV_ERROR(CV_StsUnsupportedFormat, "cvWriteFrame() needs images with depth = IPL_DEPTH_8U and nChannels = 1.");
        }
    }
    else {
        assert(false);
    }

    int size;
    size = image->imageSize;
    buffer = gst_buffer_new_and_alloc (size);
    //gst_buffer_set_data (buffer,(guint8*)image->imageData, size);
    memcpy (GST_BUFFER_DATA(buffer),image->imageData, size);
    if(gst_app_src_push_buffer(GST_APP_SRC(source),buffer) != GST_FLOW_OK)
        CV_ERROR(CV_StsError, "Shit just went down!");
    //gst_buffer_unref(buffer);
    //buffer = 0;
    __END__;
    return true;
}

void CvCapture_GStreamer::close()
{
    if(pipeline) {
        gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(pipeline));
    }
    if(buffer)
        gst_buffer_unref(buffer);
    if(frame) {
        frame->imageData = 0;
        cvReleaseImage(&frame);
    }
}

double CvCapture_GStreamer::getProperty( int propId )
{
    GstFormat format;
    //GstQuery q;
    gint64 value;

    if(!pipeline) {
        CV_WARN("GStreamer: no pipeline");
        return false;
    }

    switch(propId) {
    case CV_CAP_PROP_POS_MSEC:
        format = GST_FORMAT_TIME;
        if(!gst_element_query_position(sink, &format, &value)) {
            CV_WARN("GStreamer: unable to query position of stream");
            return false;
        }
        return value * 1e-6; // nano seconds to milli seconds
    case CV_CAP_PROP_POS_FRAMES:
        format = GST_FORMAT_DEFAULT;
        if(!gst_element_query_position(sink, &format, &value)) {
            CV_WARN("GStreamer: unable to query position of stream");
            return false;
        }
        return value;
    case CV_CAP_PROP_POS_AVI_RATIO:
        format = GST_FORMAT_PERCENT;
        if(!gst_element_query_position(pipeline, &format, &value)) {
            CV_WARN("GStreamer: unable to query position of stream");
            return false;
        }
        return ((double) value) / GST_FORMAT_PERCENT_MAX;
    case CV_CAP_PROP_FRAME_WIDTH:
    case CV_CAP_PROP_FRAME_HEIGHT:
    case CV_CAP_PROP_FPS:
    case CV_CAP_PROP_FOURCC:
        break;
    case CV_CAP_PROP_FRAME_COUNT:
        format = GST_FORMAT_DEFAULT;
        if(!gst_element_query_duration(pipeline, &format, &value)) {
            CV_WARN("GStreamer: unable to query position of stream");
            return false;
        }
        return value;
    case CV_CAP_PROP_FORMAT:
    case CV_CAP_PROP_MODE:
    case CV_CAP_PROP_BRIGHTNESS:
    case CV_CAP_PROP_CONTRAST:
    case CV_CAP_PROP_SATURATION:
    case CV_CAP_PROP_HUE:
    case CV_CAP_PROP_GAIN:
    case CV_CAP_PROP_CONVERT_RGB:
        break;
    case CV_CAP_GSTREAMER_QUEUE_LENGTH:
        if(!sink) {
                CV_WARN("GStreamer: there is no sink yet");
                return false;
        }
        return gst_app_sink_get_max_buffers(GST_APP_SINK(sink));
    default:
        CV_WARN("GStreamer: unhandled property");
        break;
    }
    return false;
}

bool CvCapture_GStreamer::setProperty( int propId, double value )
{
       GstFormat format;
    GstSeekFlags flags;

    if(!pipeline) {
        CV_WARN("GStreamer: no pipeline");
        return false;
    }

    switch(propId) {
    case CV_CAP_PROP_POS_MSEC:
        format = GST_FORMAT_TIME;
        flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_ACCURATE);
        if(!gst_element_seek_simple(GST_ELEMENT(pipeline), format,
                        flags, (gint64) (value * GST_MSECOND))) {
            CV_WARN("GStreamer: unable to seek");
        }
        break;
    case CV_CAP_PROP_POS_FRAMES:
        format = GST_FORMAT_DEFAULT;
        flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_ACCURATE);
        if(!gst_element_seek_simple(GST_ELEMENT(pipeline), format,
                        flags, (gint64) value)) {
            CV_WARN("GStreamer: unable to seek");
        }
        break;
    case CV_CAP_PROP_POS_AVI_RATIO:
        format = GST_FORMAT_PERCENT;
        flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_ACCURATE);
        if(!gst_element_seek_simple(GST_ELEMENT(pipeline), format,
                        flags, (gint64) (value * GST_FORMAT_PERCENT_MAX))) {
            CV_WARN("GStreamer: unable to seek");
        }
        break;
    case CV_CAP_PROP_FRAME_WIDTH:
        if(value > 0)
            setFilter("width", G_TYPE_INT, (int) value, 0);
        else
            removeFilter("width");
        break;
    case CV_CAP_PROP_FRAME_HEIGHT:
        if(value > 0)
            setFilter("height", G_TYPE_INT, (int) value, 0);
        else
            removeFilter("height");
        break;
    case CV_CAP_PROP_FPS:
        if(value > 0) {
            int num, denom;
            num = (int) value;
            if(value != num) { // FIXME this supports only fractions x/1 and x/2
                num = (int) (value * 2);
                denom = 2;
            } else
                denom = 1;

            setFilter("framerate", GST_TYPE_FRACTION, num, denom);
        } else
            removeFilter("framerate");
        break;
    case CV_CAP_PROP_FOURCC:
    case CV_CAP_PROP_FRAME_COUNT:
    case CV_CAP_PROP_FORMAT:
    case CV_CAP_PROP_MODE:
    case CV_CAP_PROP_BRIGHTNESS:
    case CV_CAP_PROP_CONTRAST:
    case CV_CAP_PROP_SATURATION:
    case CV_CAP_PROP_HUE:
    case CV_CAP_PROP_GAIN:
    case CV_CAP_PROP_CONVERT_RGB:
        break;
    case CV_CAP_GSTREAMER_QUEUE_LENGTH:
        if(!sink)
            break;
        gst_app_sink_set_max_buffers(GST_APP_SINK(sink), (guint) value);
        break;
    default:
        CV_WARN("GStreamer: unhandled property");
    }
    return false;
}

