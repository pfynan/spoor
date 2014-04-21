#pragma once

#include <cv.h>
#include <highgui.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/riff/riff-media.h>

class CvCapture_GStreamer
{
public:
    CvCapture_GStreamer() { init(); }
    virtual ~CvCapture_GStreamer() { close(); }

    virtual bool open( int type, const char* filename );
    virtual void close();

    virtual double getProperty(int);
    virtual bool setProperty(int, double);
    virtual bool grabFrame();
    virtual IplImage* retrieveFrame(int);

protected:
    void init();
    bool reopen();
    void handleMessage();
    void restartPipeline();
    void setFilter(const char*, int, int, int);
    void removeFilter(const char *filter);
    void static newPad(GstElement *myelement,
                             GstPad     *pad,
                             gpointer    data);
    GstElement           *pipeline;
    GstElement           *uridecodebin;
    GstElement           *color;
    GstElement           *sink;

    GstBuffer           *buffer;
    GstCaps            *caps;
    IplImage           *frame;
};

//
//
// gstreamer image sequence writer
//
//
class CvVideoWriter_GStreamer
{
public:
    CvVideoWriter_GStreamer() { init(); }
    virtual ~CvVideoWriter_GStreamer() { close(); }

    virtual bool open( const char* filename, 
                       double fps, CvSize frameSize);
    virtual void close();
    virtual bool writeFrame( const IplImage* image );
protected:
    void init();
    std::map<int, char*> encs;
    GstElement* source;
    GstBuffer* buffer;
    GstElement* pipeline;
    int input_pix_fmt;
};
