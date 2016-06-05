tar -czf bup/rbot_`date -u +%Y%m%d-%H%M%S`.tgz --exclude='sites/jq/*' --exclude='bup/*' *
gcc main.c mongoose.c -o rbot -ldl -pthread -lrt -lm -lwiringPi -lfreenect_sync

