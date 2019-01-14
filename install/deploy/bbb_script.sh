overlay_install()
{
echo "---- Overlays ----"
echo "Installing overlays..."
mv $OVERLAY_DIR/uEnv.txt /boot/
mv $OVERLAY_DIR/*.dts /lib/firmware
cd /lib/firmware
local mode=$1
if test "$mode" = 'verbose';
	then
	dtc -O dtb -I dts -o /lib/firmware/PRU0-00A0.dtbo -b 0 -@ PRU0-00A0.dts
	dtc -O dtb -I dts -o /lib/firmware/PRU1-00A0.dtbo -b 0 -@ PRU1-00A0.dts
	dtc -O dtb -I dts -o /lib/firmware/ENCODER-00A0.dtbo -b 0 -@ ENCODER-00A0.dts
	dtc -O dtb -I dts -o /lib/firmware/DISPLAY-00A0.dtbo -b 0 -@ DISPLAY-00A0.dts
	dtc -O dtb -I dts -o /lib/firmware/SGX-00A0.dtbo -b 0 -@ SGX-00A0.dts
elif test "$mode" = 'quiet';
	then
	dtc -W no-unit_address_vs_reg -O dtb -I dts -o /lib/firmware/PRU0-00A0.dtbo -b 0 -@ PRU0-00A0.dts
	dtc -W no-unit_address_vs_reg -O dtb -I dts -o /lib/firmware/PRU1-00A0.dtbo -b 0 -@ PRU1-00A0.dts
	dtc -W no-unit_address_vs_reg -O dtb -I dts -o /lib/firmware/ENCODER-00A0.dtbo -b 0 -@ ENCODER-00A0.dts
	dtc -W no-unit_address_vs_reg -O dtb -I dts -o /lib/firmware/DISPLAY-00A0.dtbo -b 0 -@ DISPLAY-00A0.dts
	dtc -W no-unit_address_vs_reg -O dtb -I dts -o /lib/firmware/SGX-00A0.dtbo -b 0 -@ SGX-00A0.dts
fi
echo "Overlays sucessfully installed."
echo ""
rm -rf $OVERLAY_DIR
}

service_install()
{
echo "---- Services ----"
echo "Installing bootloader services..."
chmod u+x $SERVICE_DIR/*.sh
mv $SERVICE_DIR/*.sh /usr/bin/
mv $SERVICE_DIR/*.service /lib/systemd/system
ln -s /lib/systemd/system/stomploader.service /etc/systemd/system/stomploader.service
ln -s /lib/systemd/system/sgx.service /etc/systemd/system/sgx.service
systemctl daemon-reload
systemctl enable stomploader.service
systemctl enable sgx.service
echo "Bootloader services successfully installed."
echo ""
rm -rf $SERVICE_DIR
}

scope_install()
{
echo "---- Scope application ----"
echo "Installing the Scope application..."
mv $SCOPE_DIR/load.sh $REMOTE_HOME_DIR
mv $SCOPE_DIR/pru_adc.out $REMOTE_HOME_DIR
mv $SCOPE_DIR/pru_osc.out $REMOTE_HOME_DIR
mv $SCOPE_DIR/scope $REMOTE_HOME_DIR
chmod u+x $REMOTE_HOME_DIR/load.sh
rm -rf $SCOPE_DIR
echo "Scope application successfully installed."
echo ""
}

lib_install()
{
echo "---- Userspace libaries ----"
echo "Installing userspace libraries..."
mv $LIB_DIR/* /usr/lib/
rm -rf $LIB_DIR
echo "Userspace libraries successfully installed."
echo ""
}

xorg_install()
{
echo "---- X11 server ----"
echo "Updating packages list..."
apt-get update
echo "Packages list successfully updated."
echo ""
echo "Installing 'xinit' package..."
apt-get install xinit
echo "Package successfully installed."
echo ""
echo "Installing X11 configuration file and scripts..."
mv $XORG_DIR/xorg.conf /etc/X11/
mv $XORG_DIR/xinitrc /etc/X11/xinit/
rm -rf $XORG_DIR
echo "X11 server successfully configured."
echo ""
}

sgx_install()
{
echo "---- SGX support ----"
echo "Installing SGX kernel drivers..."
apt install $SGX_DIR/drivers/ti-sgx-ti335x-modules-4.9.78-ti-r94_1stretch_armhf.deb
depmod -a
echo "Kernel drivers successfully installed."
echo ""
echo "Installing SGX userspace libraries..."
cd $SGX_DIR
make install
sed -i '$ a export LD_LIBRARY_PATH="/usr/lib/"' /root/.bashrc
echo "SGX userspace libraries successfully installed."
echo ""
rm -rf $SGX_DIR
}

main()
{
echo ""
chmod 777 $DEPLOY_DIR
mv $DEPLOY_DIR/xorg/resolv.conf /etc/
overlay_install $mode
service_install
scope_install
lib_install
xorg_install
sgx_install
rm -rf $DEPLOY_DIR
echo "Default password: temppwd"
reboot
}

mode=$1
DEPLOY_DIR=$2
REMOTE_HOME_DIR=$3
OVERLAY_DIR=$DEPLOY_DIR/overlay
SERVICE_DIR=$DEPLOY_DIR/service
SCOPE_DIR=$DEPLOY_DIR/scope
LIB_DIR=$DEPLOY_DIR/lib
XORG_DIR=$DEPLOY_DIR/xorg
SGX_DIR=$DEPLOY_DIR/sgx
main

