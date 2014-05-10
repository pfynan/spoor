gst-launch-0.10 -v v4l2src ! \
    "image/jpeg,width=640,height=480,framerate=25/1" ! \
    rtpjpegpay ! \
    udpsink host=127.0.0.1 port=4000


