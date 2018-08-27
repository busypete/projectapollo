check_module()
{
local module="$1"
if lsmod | grep "$module" &> /dev/null ; 
	then
    	return 1
else
    	return 0
fi
}

init_module()
{
local module="$1"
check_module "$module"
local state_module=$?
if [[ ($state_module -eq 0) ]]
	then
	modprobe "$module"
fi
}

update_firmware()
{
cp /home/debian/stomp/pru0.out /lib/firmware/am335x-pru0-fw
cp /home/debian/stomp/pru1.out /lib/firmware/am335x-pru1-fw

if [[ (($state_pru0 == "offline")&&($state_pru1 == "offline")) ]]
	then
	echo /lib/firmware/am335x-pru0-fw > /sys/class/remoteproc/remoteproc1/firmware
	echo /lib/firmware/am335x-pru1-fw > /sys/class/remoteproc/remoteproc2/firmware
elif [[ (($state_pru0 == "offline")&&($state_pru1 == "running")) ]];
	then
	echo /lib/firmware/am335x-pru0-fw > /sys/class/remoteproc/remoteproc1/firmware
elif [[ (($state_pru0 == "running")&&($state_pru1 == "offline")) ]];
	then
	echo /lib/firmware/am335x-pru1-fw > /sys/class/remoteproc/remoteproc2/firmware
fi
}

menu()
{
if [ $action == "-h" ]
	then
	echo "First argument: action" 
	echo "     -- options --     -r: run"
	echo "                       -s: stop"
	echo "                       -c: cat state"
	echo "Last argument:  core in which the action will take place"
	echo "     -- options --     pru0"
	echo "                       pru1"
elif [[ (($action == "-r")&&($core == pru0)) ]];
	then
	run_pru0	
elif [[ (($action == "-r")&&($core == pru1)) ]];
	then
	run_pru1
elif [[ (($action == "-s")&&($core == pru0)) ]];
	then
	stop_pru0
elif [[ (($action == "-s")&&($core == pru1)) ]];
	then
	stop_pru1
elif [[ (($action == "-c")&&($core == pru0)) ]];
	then
	cat_pru0 verbose
elif [[ (($action == "-c")&&($core == pru1)) ]];
	then
	cat_pru1 verbose
else
	error
fi
}

run_pru0()
{	
cat_pru0 silent
local state_pru0=$state
cat_pru1 silent
local state_pru1=$state

if [[ (($state_pru0 == "offline")&&($state_pru1 == "offline")) ]]
	then
	rmmod pru_rproc
	modprobe pru_rproc
	echo 'start' > /sys/class/remoteproc/remoteproc1/state
elif [[ (($state_pru0 == "offline")&&($state_pru1 == "running")) ]];
	then
	echo 'start' > /sys/class/remoteproc/remoteproc1/state
elif [[ (($state_pru0 == "running")&&($state_pru1 == "offline")) ]];
	then
	echo "pru0 already running"
elif [[ (($state_pru0 == "running")&&($state_pru1 == "running")) ]];
	then
	echo "pru0 and pru1 already running"
fi


}

run_pru1()
{
cat_pru0 silent
local state_pru0=$state
cat_pru1 silent
local state_pru1=$state

if [[ (($state_pru0 == "offline")&&($state_pru1 == "offline")) ]]
	then
	rmmod pru_rproc
	modprobe pru_rproc
	echo 'start' > /sys/class/remoteproc/remoteproc2/state
elif [[ (($state_pru0 == "offline")&&($state_pru1 == "running")) ]];
	then
	echo "pru1 already running"
elif [[ (($state_pru0 == "running")&&($state_pru1 == "offline")) ]];
	then
	echo 'start' > /sys/class/remoteproc/remoteproc2/state
elif [[ (($state_pru0 == "running")&&($state_pru1 == "running")) ]];
	then
	echo "pru0 and pru1 already running"
fi

}

stop_pru0()
{
cat_pru0 silent
local state_pru0=$state
if [[ ($state_pru0 == "offline") ]]
	then
	echo "pru0 already halted"
elif [[ ($state_pru0 == "running") ]]
	then
	echo 'stop' > /sys/class/remoteproc/remoteproc1/state
fi
}

stop_pru1()
{
cat_pru1 silent
local state_pru1=$state
if [[ ($state_pru1 == "offline") ]]
	then
	echo "pru1 already halted"
elif [[ ($state_pru1 == "running") ]]
	then
	echo 'stop' > /sys/class/remoteproc/remoteproc2/state
fi
}

cat_pru0()
{
local mode=$1
state=$(cat /sys/class/remoteproc/remoteproc1/state)
if [[ ($mode == "verbose") ]]
	then
	echo $state
fi
}

cat_pru1()
{
local mode=$1
state=$(cat /sys/class/remoteproc/remoteproc2/state)
if [[ ($mode == "verbose") ]]
	then
	echo $state
fi
}

error()
{
echo "Invalid command. Type './load.sh -h' for help on using this script."
}

main()
{
init_module pru_rproc
update_firmware
menu
}

action=$1
core=$2
main


