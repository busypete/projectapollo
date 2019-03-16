overlay_install()
{
echo "---- Overlays ----"
echo "Installing overlays..."
mv $OVERLAY_DIR/am335x-boneblack.dtb /boot/
rm /boot/devicetree-zImage-am335x-boneblack.dtb
ln -fs /boot/am335x-boneblack.dtb /boot/devicetree-zImage-am335x-boneblack.dtb
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
mv $SERVICE_DIR/systemd-sysv-install /lib/systemd/
ln -s /lib/systemd/system/stomploader.service /etc/systemd/system/stomploader.service
systemctl daemon-reload
systemctl enable stomploader.service
systemctl disable matrix-gui-2.0.service
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


lib_fix()
{
echo "Fixing libraries symlinks..."
cd /usr/lib
ln -s libQt5Charts.so.5.7.1 libQt5Charts.so
ln -s libQt5Charts.so.5.7.1 libQt5Charts.so.5
ln -s libQt5Charts.so.5.7.1 libQt5Charts.so.5.7
echo "Libraries symlinks successfully fixed."
echo ""
}

lib_install()
{
echo "---- Userspace libaries ----"
echo "Installing userspace libraries..."
mv $LIB_DIR/* /usr/lib/
lib_fix
rm -rf $LIB_DIR
echo "Userspace libraries successfully installed."
echo ""
}

ip_install()
{
echo "---- IP address ----"
echo "Configuring static IP address..."
mv $IP_DIR/60-usb.network /etc/systemd/network
echo "Static IP address successfully configured."
echo ""
}

main()
{
overlay_install
service_install
scope_install
lib_install
ip_install
rm -rf $DEPLOY_DIR
echo ""
echo "---- Instalation completed successfully ----"
echo ""
/sbin/reboot
}

DEPLOY_DIR=$1
REMOTE_HOME_DIR=$2
OVERLAY_DIR=$DEPLOY_DIR/overlay
SERVICE_DIR=$DEPLOY_DIR/service
SCOPE_DIR=$DEPLOY_DIR/scope
LIB_DIR=$DEPLOY_DIR/lib
IP_DIR=$DEPLOY_DIR/ip
main

