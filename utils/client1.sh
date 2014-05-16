gst-launch-0.10 -v -e filesrc location="$1" ! \
	matroskademux ! \
    rtpjpegpay ! \
    udpsink host=127.0.0.1 port=4000


    # "image/jpeg,width=640,height=480,framerate=15/1" ! \
