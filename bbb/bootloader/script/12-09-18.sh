#!/bin/sh -
cd /home/debian/stomp
./load.sh -s pru1
./load.sh -s pru0
./load.sh -r pru1 pru_adc.out
./load.sh -r pru0 pru_osc.out
cd /etc/X11/xinit
startx
