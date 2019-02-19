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
export SKIP="--disable-largefile --disable-driglx-direct --without-gallium-drivers --without-dri-drivers --without-vulkan-drivers"
export ENABLE="--enable-gbm --with-platforms=x11,drm,wayland"
./configure $ENABLE $SKIP
make && make install
rm -rf $WESTON_DIR/mesa-17.1.4
echo "Mesa support successfully installed."
rm -rf $WESTON_DIR
}

main()
{
#lib_fix
#get_disk_space
weston_install
}

DEPLOY_DIR=/home/debian/stomp
WESTON_DIR=$DEPLOY_DIR/weston
main

