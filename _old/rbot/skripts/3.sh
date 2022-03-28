aplay ~/bastel/sound/overload.wav -D plughw:CARD=Device,DEV=0 &
killall mjpg_streamer &
screen -dmS cam /home/pi/autostart_cam.sh &

