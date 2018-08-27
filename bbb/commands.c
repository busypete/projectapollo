// Device tree
/*******************************************************/
// Compile .dts file
dtc -O dtb -I dts -o /lib/firmware/PRU0-00A0.dtbo -b 0 -@ PRU0-00A0.dts
dtc -O dtb -I dts -o /lib/firmware/PRU1-00A0.dtbo -b 0 -@ PRU1-00A0.dts

// Decompile .dtbo file
dtc -I dtb -O dts am335x-boneblack.dtb > am335x-boneblack.dts
/*******************************************************/

// SSH 
/*******************************************************/
ssh debian@192.168.7.2
ssh debian@192.168.6.2
ssh debian@beaglebone.local
senha: temppwd
sed -i '10d' ~/.ssh/known_hosts						// Remove offending key at 10th line
/*******************************************************/

// SCP
/*******************************************************/
// BBB to HOST
scp /home/debian/stomp/pru0.out bruno@192.168.7.1:/home/bruno/Downloads

// HOST TO BBB
scp /home/bruno/Downloads/pru0.out debian@beaglebone.local:/home/debian/stomp
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
// Interface Ethernet-over-USB do BBB: enx38d269576fb8
// Interface Wi-Fi: wlp3s0f0

// BBB side
/*******************************************************/
/sbin/route add default gw 192.168.7.1 // Refazer esse comando após cada reboot e checar se o IP do HOST de fato é 192.168.7.1
				       // Se não for, alternar entre as duas conexões Ethernet disponíveis para o BBB
vi /etc/resolv.conf 		       // Apagar tudo q tiver no arquivo e então colar as linhas abaixo
	domain localdomain
	search localdomain
	nameserver 8.8.8.8
	nameserver 8.8.4.4
/*******************************************************/

// HOST side
/*******************************************************/
sudo ifconfig enx38d269576fb8 192.168.7.1
sudo iptables --table nat --append POSTROUTING --out-interface wlp3s0f0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface enx38d269576fb8 -j ACCEPT
sudo bash -c 'echo 1 > /proc/sys/net/ipv4/ip_forward'
/*******************************************************/

/*******************************************************/


// QT
/*******************************************************/
source /home/bruno/ti-processor-sdk-linux-am335x-evm-04.02.00.09/linux-devkit/environment-setup
cd /home/bruno/Qt/Tools/QtCreator/bin
./qtcreator
/*******************************************************/


// Service
/*******************************************************/

// Service name: stomploader.service
// Script that the service will execute: stomploader.sh

cp /home/debian/stomp/stomploader.sh /usr/bin
cd /usr/bin
chmod u+x stomploader.sh

cp /home/debian/stomp/stomploader.service /lib/systemd/system
cd /etc/systemd/system
ln -s /lib/systemd/system/stomploader.service stomploader.service

systemctl daemon-reload
systemctl start teste.service
systemctl enable teste.service
systemctl status teste.service
/*******************************************************/

