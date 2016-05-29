#!/bin/bash
sudo chmod a+x /home/pi/autostart*
amixer cset numid=3 1
amixer cset numid=1 -- 0
amixer -c 1 sset 'Speaker' 100%
aplay ~/bastel/sound/MacStartup.wav -D plughw:CARD=Device,DEV=0
amixer -c 1 sset 'Speaker' 90%

screen -dmS cam /home/pi/autostart_cam.sh
screen -dmS controller /home/pi/autostart_rbot.sh
