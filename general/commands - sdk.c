// Device tree
/*******************************************************/
// Compile .dts file (first cd into /lib/firmware)
dtc -O dtb -I dts -o /lib/firmware/PRU0-00A0.dtbo -b 0 -@ PRU0-00A0.dts
dtc -O dtb -I dts -o /lib/firmware/PRU1-00A0.dtbo -b 0 -@ PRU1-00A0.dts
dtc -O dtb -I dts -o /lib/firmware/ENCODER-00A0.dtbo -b 0 -@ ENCODER-00A0.dts
dtc -O dtb -I dts -o /lib/firmware/DISPLAY-00A0.dtbo -b 0 -@ DISPLAY-00A0.dts
dtc -O dtb -I dts -o /lib/firmware/SGX-00A0.dtbo -b 0 -@ SGX-00A0.dts

// Decompile .dtbo file
dtc -I dtb -O dts am335x-boneblack.dtb > am335x-boneblack.dts
/*******************************************************/

// SSH 
/*******************************************************/
ssh debian@192.168.7.2
ssh debian@am335x-evm.local
senha: 
sed -i '3d' ~/.ssh/known_hosts						// Remove offending key at 3rd line
/*******************************************************/

// SCP
/*******************************************************/
// BBB to HOST
scp /home/debian/stomp/pru0.out bruno@192.168.7.1:/home/bruno/Downloads

// HOST TO BBB
scp /home/bruno/Downloads/pru0.out root@am335x-evm.local:/home/root/stomp
/*******************************************************/

// Change to 'root' user
/*******************************************************/
sudo -i root_password=$(sudo cat /etc/shadow | grep root | awk -F ':' '{print $2}')
/*******************************************************/

// Capture pinmux information
/*******************************************************/
cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pinmux-pins 
/*******************************************************/

// Find stuff
/*******************************************************/
find /path/to/search/all/subdirectories -iname "filename"		// File
grep --include=\*.{c,h,dts} -rnw '/' -e "P8.15"				// File with certain extension and content
find /path/to/search/all/subdirectories -type d -iname "foldername"	// Folder
/*******************************************************/

// Firmware
/*******************************************************/
cp /home/debian/stomp/pru0.out /lib/firmware/am335x-pru0-fw
cp /home/debian/stomp/pru1.out /lib/firmware/am335x-pru1-fw
echo /lib/firmware/am335x-pru0-fw > /sys/class/remoteproc/remoteproc1/firmware
echo /lib/firmware/am335x-pru1-fw > /sys/class/remoteproc/remoteproc2/firmware
rmmod pru_rproc
modprobe pru_rproc
echo 'start' > /sys/class/remoteproc/remoteproc1/state
echo 'start' > /sys/class/remoteproc/remoteproc2/state
echo 'stop' > /sys/class/remoteproc/remoteproc1/state
echo 'stop' > /sys/class/remoteproc/remoteproc2/state
/*******************************************************/

// Wi-Fi over-USB
/*******************************************************/

// IP do HOST na Ethernet-over-USB: 192.168.7.1
// Interface Ethernet-over-USB do BBB: enx38d269576fb9
// Interface Wi-Fi do roteador do HOST: wlp3s0f0

// BBB side
/*******************************************************/
/sbin/route add default gw 192.168.7.1 // Refazer esse comando após cada reboot e checar se o IP do HOST de fato é 192.168.6.1
				       // Se não for, alternar entre as duas conexões Ethernet disponíveis para o BBB
vi /etc/resolv.conf 		       // Apagar tudo q tiver no arquivo e então colar as linhas abaixo
	domain localdomain
	search localdomain
	nameserver 8.8.8.8
	nameserver 8.8.4.4
/*******************************************************/

// HOST side
/*******************************************************/
sudo ifconfig enx38d269576fb9 192.168.7.1
sudo iptables --table nat --append POSTROUTING --out-interface wlp3s0f0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface enx38d269576fb9 -j ACCEPT
sudo bash -c 'echo 1 > /proc/sys/net/ipv4/ip_forward'
/*******************************************************/

/*******************************************************/


// QT
/*******************************************************/
source /home/bruno/ti-processor-sdk-linux-am335x-evm-04.02.00.09/linux-devkit/environment-setup
cd /home/bruno/Qt/Tools/QtCreator/bin
./qtcreator

// libEGL authenticate warning fix:
ln -fs /usr/lib/libEGL.so.1 /usr/lib/arm-linux-eabihf/libEGL.so.1


/*******************************************************/


// Service
/*******************************************************/

// Service name: stomploader.service
// Script that the service will execute: stomploader.sh

cp /home/root/stomp/stomploader.sh /usr/bin
chmod u+x /usr/bin/stomploader.sh

cp /home/root/stomp/stomploader.service /lib/systemd/system
cd /etc/systemd/system
ln -s /lib/systemd/system/stomploader.service stomploader.service

systemctl daemon-reload
systemctl start stomploader.service
systemctl enable stomploader.service
systemctl disable matrix-gui-2.0.service
systemctl status stomploader.service
/*******************************************************/

// Flash eMMC
/*******************************************************/

// BBB side
/*******************************************************/
// Place this line (or uncomment if it's already placed) at the last line of /boot/uEnv.txt file
cmdline=init=/opt/scripts/tools/eMMC/init-eMMC-flasher-v3.sh
// Reboot
reboot
// The eMMC will be flashed and the BBB will shut down after the flashing script completes. Now, 
// if you remove the SD card and apply power again, the board will boot on the image just flashed
// to the eMMC. 
/*******************************************************/

// HOST side
/*******************************************************/
// Remember to comment the line uncommented before. If you don't, the flashing script will run
// every time you boot from the SD card. 
sudo gedit /media/bruno/rootfs/boot/uEnv.txt
/*******************************************************/

/*******************************************************/

// SSH keys
/*******************************************************/

// Everything is done from HOST side

// Setup keys
ssh-keygen -t rsa
ssh debian@beaglebone.local mkdir -p /home/debian/.ssh
cat .ssh/id_rsa.pub | ssh debian@beaglebone.local 'cat >> .ssh/authorized_keys'

// Login without password
ssh debian@beaglebone.local

// Scp using private key
scp -i /home/bruno/.ssh/id_rsa /home/bruno/Downloads/kernel.c debian@beaglebone.local:/home/debian/stomp
/*******************************************************/
