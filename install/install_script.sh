setup_ssh_hosts()
{
echo "---- SSH setup ----"
echo "Resolving hostnames..."
ssh-keygen -f "/home/bruno/.ssh/known_hosts" -R $TARGET_IP
sed -i '3d' ~/.ssh/known_hosts	
echo "Hostnames resolved."
echo ""
}

deploy_files()
{
echo "---- Deploy files ----"
echo "Changing remote deploy directory permissions..."
ssh $TARGET_USER@$TARGET_IP -t "mkdir -p $REMOTE_HOME_DIR; sudo chmod 777 $REMOTE_HOME_DIR"
echo "Permissions sucessfully changed."
echo ""
echo "Deploying files..."
scp -r $DOWNLOAD_DIR/deploy $TARGET_USER@$TARGET_IP:$REMOTE_HOME_DIR
echo "Files deployed."
echo ""
}

login_as_bbb_root()
{
echo "---- Remote script ----"
echo "Changing remote deploy script permissions..."
ssh $TARGET_USER@$TARGET_IP "sudo chmod u+x $DEPLOY_DIR/bbb_script.sh" 
echo "Permissions sucessfully changed."
echo ""
echo "Executing remote deploy script as root..."
echo ""
ssh $TARGET_USER@$TARGET_IP -t "cd $DEPLOY_DIR; sudo ./bbb_script.sh $DEPLOY_DIR $REMOTE_HOME_DIR" 
}

run_install()
{
setup_ssh_hosts
deploy_files
login_as_bbb_root
}


help_message()
{
echo "This script will install the ProjectApollo on your device. It may take a few minutes."
echo "Usage:  ./install_script.sh" 
}

error_message()
{
echo "Invalid command. Type './install_script.sh --help' for help on using this script."
}

menu()
{
if test "$args_number" -eq 1;
	then
	if test "$action" = '--help'; 
		then
		help_message
	else	
		error_message
	fi
elif test "$args_number" -eq 0;
	then
	run_install
else
	error
fi
}

main()
{
menu
}

action=$1
args_number=$#
TARGET_IP=am335x-evm.local
TARGET_USER=root
REMOTE_HOME_DIR=/home/root/stomp
DEPLOY_DIR=$REMOTE_HOME_DIR/deploy
DOWNLOAD_DIR=$(pwd)
main 2>&1 | tee install.log

