#!/bin/sh -
echo 66 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio66/direction
echo 0 > /sys/class/gpio/gpio66/value
echo 67 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio67/direction
echo 1 > /sys/class/gpio/gpio67/value
echo 68 > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio68/direction
echo 12 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio12/direction
echo 13 > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio13/direction

cd /home/debian/stomp
./load.sh -s pru1
./load.sh -s pru0
./load.sh -r pru1 pru_adc.out
./load.sh -r pru0 pru_osc.out
cd /etc/X11/xinit
startx
