CAPS="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)JPEG, payload=(int)96" 

gst-launch-0.10 -e udpsrc port=5000 caps="$CAPS" ! \
        rtpjpegdepay ! jpegdec ! \
        video/x-raw-yuv ! \
        autovideosink force-aspect-ratio=true
        #videorate ! video/x-raw-yuv, framerate=25/1 ! \
        #jpegenc ! \
        #matroskamux ! filesink location=out.mkv

