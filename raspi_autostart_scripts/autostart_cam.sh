echo "cam start"
cd ~/mjpg-streamer/mjpg-streamer-experimental/
export LD_LIBRARY_PATH=.
./mjpg_streamer -o "output_http.so -n -p 8080 -w /usr/local/www" -i "input_raspicam.so -x 640 -y 480 -q 6 -fps 10 -ex night -br 50 -co 40"

