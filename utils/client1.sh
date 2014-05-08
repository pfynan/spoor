gst-launch-0.10 -v filesrc location="video/real1.mkv" ! \
    matroskademux ! \
    rtpjpegpay ! \
    udpsink host=127.0.0.1 port=4000


