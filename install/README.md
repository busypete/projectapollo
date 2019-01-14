1.  Grab an SD card, plug it on your PC using a SD card adapter and format it following the guide below:

https://www.pcworld.com/article/3176712/linux/how-to-format-an-sd-card-in-linux.html

2. Download the Debian image used in the ProjectApollo, from the link below:

http://debian.beagleboard.org/images/bone-debian-9.3-lxqt-armhf-2018-01-28-4gb.img.xz

3. Download the software Etcher, from the link below:

https://www.balena.io/etcher/

4. Mount the image into the SD card, using Etcher:

Launch the Etcher program.
Click 'Select Image' and browse to the 'bone-debian-9.3-lxqt-armhf-2018-01-28-4gb.img.xz' you downloaded at Step 2.
Click 'Flash!'.

5. Unplug the SD card adapter from your PC and then download the latest release from ProjectApollo repo:

https://github.com/busypete/projectapollo/releases/download/v1.0.4/install.zip

6. Extract the 'install.zip' file you downloaded and enter into the 'install' directory just extracted:

cd install

7. Insert the SD card into the board adapter. Holding the S1 button, plug in the board in your PC using the USB cable.

NOTE: It is necessary that you keep holding the button until the blue LEDs in your board light up.

8. Execute the installation script.

chmod u+x install_script.sh
./install_script.sh -v

9. Wait until the board is turned off (the LEDs will be steady off). It's all ready. Plug out the board from your PC and plug out the SD card from your board. You won't need it anymore, unless you want to restore your device. Next time you apply power to your board, it will boot right on the Scope application. 

NOTE: If anything wrong happened, please let us know and send us the installation log file, which is located at 'install' folder, under the name 'install.log'. An example of a successful installation log file is provided at 'right_install.log'. 

NOTE: If you ever want to restore your device to the factory settings, repeat step 7.

NOTE: The SGX demos can only run if the Xorg processes are off. So, before running any SGX demo, run the following commands (default password is 'temppwd'):

	ssh-keygen -f "~/.ssh/known_hosts" -R beaglebone.local
	ssh debian@beaglebone.local
	sudo -i root_password=$(sudo cat /etc/shadow | grep root | awk -F ':' '{print $2}')
	systemctl stop stomploader.service
	killall Xorg	

10. Enjoy! 
