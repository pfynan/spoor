CAPS="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)JPEG, payload=(int)96" 

gst-launch-0.10 -v -e udpsrc port=5000 caps="$CAPS" ! \
        rtpjpegdepay ! \
        jpegdec ! autovideosink
        #jpegdec ! ffmpegcolorspace ! ffenc_mpeg4 ! \
#        avimux ! filesink location=out.avi

