setup_ssh_keys()
{
echo "---- Setup ssh keys ----"
echo "Just press ENTER in the next prompts. If ~/.ssh/id_rsa already exists, overwrite it typing 'y' when asked."
echo ""
ssh-keygen -t rsa
echo ""
echo "Created ssh keys successfully."
echo ""
echo "Creating remote directory to receive ssh keys..."
echo "Default password is: temppwd"
ssh debian@beaglebone.local mkdir -p /home/debian/.ssh
echo "Remote directory successfully created."
echo ""
echo "Transfering ssh keys to remote target..."
echo "Default password is: temppwd"
cat ~/.ssh/id_rsa.pub | ssh debian@beaglebone.local 'cat >> /home/debian/.ssh/authorized_keys'
echo "Transferred ssh keys successfully."
echo ""
}

setup_remote_wi_fi()
{
echo "---- Setup remote Wi-Fi ----"
echo "Type your user password."
echo ""
echo "Configuring host's IP tables..."
sudo ifconfig enx38d269576fbb 192.168.6.1
sudo iptables --table nat --append POSTROUTING --out-interface wlp3s0f0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface enx38d269576fbb -j ACCEPT
sudo bash -c 'echo 1 > /proc/sys/net/ipv4/ip_forward'
echo "Host's IP tables successfully configured."
echo ""
echo "Configuring remote route..."
echo "Default password is: temppwd"
ssh debian@beaglebone.local -t "sudo /sbin/route add default gw 192.168.6.1"
echo "Remote route sucessfully configured."
echo ""
}

deploy_files()
{
echo "---- Deploy files ----"
echo "Changing remote deploy directory permissions..."
echo "Default password is: temppwd"
ssh debian@beaglebone.local -t "mkdir -p $REMOTE_HOME_DIR; sudo chmod 777 $REMOTE_HOME_DIR"
echo "Permissions sucessfully changed."
echo ""
if test "$mode" = 'verbose';
	then
	echo "Deploying files..."
	echo "Default password is: temppwd"
	scp -r $DOWNLOAD_DIR/deploy debian@beaglebone.local:$REMOTE_HOME_DIR
	echo "Files deployed."
elif test "$mode" = 'quiet';
	then
	echo "Deploying files..."
	scp -q -r $DOWNLOAD_DIR/deploy debian@beaglebone.local:$REMOTE_HOME_DIR
	echo "Files deployed."
fi
echo ""
}

login_as_bbb_root()
{
echo "---- Remote script ----"
local mode=$1
echo "Changing remote deploy script permissions..."
echo "Default password is: temppwd"
ssh debian@beaglebone.local -t "sudo chmod u+x $DEPLOY_DIR/bbb_script.sh" 
echo "Permissions sucessfully changed."
echo ""
echo "Executing remote deploy script as root..."
echo "Default password is: temppwd"
echo ""
ssh debian@beaglebone.local -t "cd $DEPLOY_DIR; sudo ./bbb_script.sh $mode $DEPLOY_DIR $REMOTE_HOME_DIR" 
}

run_install()
{
local mode=$1 
#setup_ssh_keys
setup_remote_wi_fi $mode
deploy_files $mode
login_as_bbb_root $mode
}

menu()
{
if test "$args_number" -eq 1;
	then
	if test "$action" = '--help'; 
		then
		echo "This script will install the ProjectApollo on your device. It may take a few minutes."
		echo "First argument:  verbosity level" 
		echo "     -- options --     -v: verbose installation"
		echo "                       -q: quiet installatoin"
	elif test "$action" = '-v'; 
		then
		run_install verbose 
	elif test "$action" = '-q'; 
		then
		run_install quiet
	else	
		error
	fi
elif test "$args_number" -eq 0;
	then
	run_install silent
else
	error
fi
}

error()
{
echo "Invalid command. Type './install_script.sh --help' for help on using this script."
}

main()
{
menu
}

action=$1
args_number=$#
REMOTE_HOME_DIR=/home/debian/stomp
DEPLOY_DIR=$REMOTE_HOME_DIR/deploy
DOWNLOAD_DIR=$(pwd)
main

