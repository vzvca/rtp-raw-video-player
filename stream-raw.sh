gst-launch-1.0 videotestsrc  ! videorate ! videoscale ! videoconvert ! video/x-raw, width=720, height=576, framerate=25/1 ! queue ! rtpvrawpay mtu=1450 ! queue ! udpsink host=239.192.77.10 port=5004 auto-multicast=true multicast-iface=lo
#sudo gst-launch-1.0 -v v4l2src ! videorate ! videoscale ! videoconvert ! video/x-raw, width=720, height=576, framerate=25/1 ! queue ! rtpvrawpay mtu=1450 ! queue ! udpsink host=239.192.77.10 port=5004 auto-multicast=true multicast-iface=enp3s0
#gst-launch-1.0 -v v4l2src ! videorate ! videoscale ! videoconvert ! video/x-raw, width=720, height=576, framerate=25/1 ! imxipuvideotransform ! video/x-raw, width=1280, height=720 ! queue ! rtpvrawpay mtu=1450 ! queue ! udpsink host=239.0.0.5 port=5004 auto-multicast=true multicast-iface=ethO
