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

qtlib_install()
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
echo "Installing 'xinit' package..."
apt install $XORG_DIR/xinit_1.3.4-3+b1_armhf.deb
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

lib_fix()
{
echo "Fixing libraries symlinks..."
cd /usr/lib
rm libGLES_CM.so libGLES_CM.so.1 libglslcompiler.so libglslcompiler.so.1 libpvr2d.so libpvr2d.so.1 libPVRScopeServices.so libPVRScopeServices.so.1 libsrv_um.so libsrv_um.so.1 libpvr_wlegl.so libpvr_wlegl.so.1 libsrv_init.so libsrv_init.so.1 libpvrws_WAYLAND.so libpvrws_WAYLAND.so.1 libpvrDRMWSEGL.so libpvrDRMWSEGL.so.1 libdbm.so libdbm.so.1 libQt5Charts.so libQt5Charts.so.5 libQt5Charts.so.5.7 libEGL.so libEGL.so.1 libusc.so libusc.so.1 libpvrGBMWSEGL.so libpvrGBMWSEGL.so.1 libIMGegl.so libIMGegl.so.1 libgbm.so libgbm.so.2
ln -s libGLES_CM.so.1.14.3699939 libGLES_CM.so
ln -s libGLES_CM.so.1.14.3699939 libGLES_CM.so.1
ln -s libglslcompiler.1.14.3699939 libglslcompiler.so
ln -s libglslcompiler.1.14.3699939 libglslcompiler.so.1
ln -s libpvr2d.so.1.14.3699939 libpvr2d.so
ln -s libpvr2d.so.1.14.3699939 libpvr2d.so.1
ln -s libPVRScopeServices.so.1.14.3699939 libPVRScopeServices.so
ln -s libPVRScopeServices.so.1.14.3699939 libPVRScopeServices.so.1
ln -s libsrv_um.so.1.14.3699939 libsrv_um.so
ln -s libsrv_um.so.1.14.3699939 libsrv_um.so.1
ln -s libpvr_wlegl.so.1.14.3699939 libpvr_wlegl.so
ln -s libpvr_wlegl.so.1.14.3699939 libpvr_wlegl.so.1
ln -s libsrv_init.so.1.14.3699939 libsrv_init.so
ln -s libsrv_init.so.1.14.3699939 libsrv_init.so.1
ln -s libpvrws_WAYLAND.so.1.14.3699939 libpvrws_WAYLAND.so
ln -s libpvrws_WAYLAND.so.1.14.3699939 libpvrws_WAYLAND.so.1
ln -s libpvrDRMWSEGL.so.1.14.3699939 libpvrDRMWSEGL.so
ln -s libpvrDRMWSEGL.so.1.14.3699939 libpvrDRMWSEGL.so.1
ln -s libdbm.so.1.14.3699939 libdbm.so
ln -s libdbm.so.1.14.3699939 libdbm.so.1
ln -s libQt5Charts.so.5.7.1 libQt5Charts.so
ln -s libQt5Charts.so.5.7.1 libQt5Charts.so.5
ln -s libQt5Charts.so.5.7.1 libQt5Charts.so.5.7
ln -s libEGL.so.1.14.3699939 libEGL.so
ln -s libEGL.so.1.14.3699939 libEGL.so.1
ln -s libusc.so.1.14.3699939 libusc.so
ln -s libusc.so.1.14.3699939 libusc.so.1
ln -s libpvrGBMWSEGL.so.1.14.3699939 libpvrGBMWSEGL.so
ln -s libpvrGBMWSEGL.so.1.14.3699939 libpvrGBMWSEGL.so.1
ln -s libIMGegl.so.1.14.3699939 libIMGegl.so
ln -s libIMGegl.so.1.14.3699939 libIMGegl.so.1
ln -s libgbm.so.2.0.0 libgbm.so
ln -s libgbm.so.2.0.0 libgbm.so.2
echo "Libraries symlinks successfully fixed."
echo ""
}

get_disk_space()
{
echo "Freeing some disk space..."
rm -rf /opt/source
rm -rf /opt/cloud9
echo "Disk space successfully freed."
echo ""
}

weston_install()
{
echo "---- Weston support ----"
echo "Configuring environment..."
WLD=/usr
LD_LIBRARY_PATH=$WLD/lib
PKG_CONFIG_PATH=$WLD/lib/pkgconfig/:$WLD/share/pkgconfig/
PATH=$WLD/bin:$PATH
ACLOCAL_PATH=$WLD/share/aclocal
ACLOCAL="aclocal -I $ACLOCAL_PATH"
mkdir -p $WLD/share/aclocal
echo "Environment successfully configured."
echo ""
echo "Installing dependencies..."
apt-get update
apt-get install libffi-dev libxml2-dev libgles2-mesa-dev libxcb-composite0-dev libxcursor-dev libcairo2-dev libgbm-dev libpam0g-dev libmtdev-dev libudev-dev libevdev-dev libwacom-dev libgtk-3-dev check libunwind-dev flex python-pip libyaml-dev bison
pip install prettytable Mako pyaml dateutils --upgrade
echo "Dependencies successfully installed."
echo ""
echo "Installing Wayland libraries..."
cd $WESTON_DIR/wayland-master
./autogen.sh --prefix=$WLD --libdir=/usr/lib --sysconfdir=/etc --disable-documentation
make && make install
rm -rf $WESTON_DIR/wayland-master
echo "Wayland libraries successfully installed."
echo ""
echo "Installing Wayland protocols..."
cd $WESTON_DIR/wayland-protocols-1.9
./configure --prefix=$WLD
mkdir m4
make && make install
rm -rf $WESTON_DIR/wayland-protocols-1.9
echo "Wayland protocols successfully installed."
echo ""
echo "Installing libinput..."
cd $WESTON_DIR/libinput-1.8.0
./configure --prefix=$WLD --disable-documentation
make && make install
rm -rf $WESTON_DIR/libinput-1.8.0
echo "libinput successfully installed."
echo ""
cd $WESTON_DIR/weston-2.0.0
echo "Installing Weston 2.0 ..."
./configure --prefix=$WLD --libdir=/usr/lib --disable-setuid-install
ln -s /usr/bin/automake /usr/bin/automake-1.14
autoreconf --force --install
make && make install
rm -rf $WESTON_DIR/weston-2.0.0
cd $WESTON_DIR
cp .profile /root/.profile && cp .profile /home/debian/.profile
mkdir -p /root/.config && mkdir -p /home/debian/.config
cp weston.ini /root/.config/ && cp weston.ini /home/debian/.config/
groupadd weston-launch
usermod -a -G weston-launch $USER
chown root /usr/bin/weston-launch
chmod +s /usr/bin/weston-launch
echo "Weston 2.0 successfully installed."
echo ""
echo "Installing Mesa support..."
cd $WESTON_DIR/mesa-17.1.4
SKIP="--disable-largefile --disable-driglx-direct --without-gallium-drivers --without-dri-drivers --without-vulkan-drivers"
ENABLE="--enable-gbm --with-platforms=x11,drm,wayland"
./configure $ENABLE $SKIP
make && make install
rm -rf $WESTON_DIR/mesa-17.1.4
echo "Mesa support successfully installed."
rm -rf $WESTON_DIR
}

main()
{
echo ""
chmod 777 $DEPLOY_DIR
mv $DEPLOY_DIR/xorg/resolv.conf /etc/
overlay_install $mode
service_install
scope_install
qtlib_install
xorg_install
sgx_install
lib_fix
get_disk_space
weston_install
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
LIB_DIR=$DEPLOY_DIR/qtlib
XORG_DIR=$DEPLOY_DIR/xorg
SGX_DIR=$DEPLOY_DIR/sgx
WESTON_DIR=$DEPLOY_DIR/weston
main

