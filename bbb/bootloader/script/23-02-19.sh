#!/bin/sh -

run()
{
echo 66 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio66/direction
echo 0 > /sys/class/gpio/gpio66/value
echo 67 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio67/direction
echo 1 > /sys/class/gpio/gpio67/value
echo 68 > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio68/direction
cd /home/root/stomp
./load.sh -r pru1 pru_adc.out
./load.sh -r pru0 pru_osc.out
killall weston
./scope -platform eglfs
}

stop()
{
killall scope
cd /home/root/stomp
./load.sh -s pru0
./load.sh -s pru1

if test -d "/sys/class/gpio/gpio66"; 
	then
	echo 66 > /sys/class/gpio/unexport
else
	echo "gpio66 already unexported"
fi

if test -d "/sys/class/gpio/gpio67"; 
	then
	echo 67 > /sys/class/gpio/unexport	
else	
	echo "gpio67 already unexported"
fi
	
if test -d "/sys/class/gpio/gpio68"; 
	then
	echo 68 > /sys/class/gpio/unexport
else
	echo "gpio68 already unexported"
fi
}

main()
{
if test "$action" = '-r';
	then
	run
elif test "$action" = '-s';
	then
	stop
fi
}

action=$1
main

