gst-launch-0.10 -v gstrtpbin name=rtpbin filesrc location="video/low1.avi" \
    ! avidemux ! mpeg4videoparse ! ffdec_mpeg4 \
    ! videorate ! videoscale ! ffmpegcolorspace ! \
    'video/x-raw-yuv, width=(int)320, height=(int)240, framerate=(fraction)15/1' !  \
    rtpvrawpay ! rtpbin.send_rtp_sink_0 rtpbin.send_rtp_src_0 ! \
    multiudpsink clients="127.0.0.1:9996" rtpbin.send_rtcp_src_0 ! \
    multiudpsink clients="127.0.0.1:9997" sync=false async=false udpsrc port=10000 ! rtpbin.recv_rtcp_sink_0

cat > /dev/null << END

END

