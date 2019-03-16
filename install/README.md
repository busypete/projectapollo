1. Grab an SD card and plug it on your PC using a SD card adapter.

2. Download the latest release from ProjectApollo repo:

https://github.com/busypete/projectapollo/releases/download/v2.0.0/install.zip

3. Extract the 'install.zip' file you downloaded and enter into the 'install' directory just extracted:

cd install

4. Run the create-sdcard.sh script to flash your SD card with the image used in project Apollo:

chmod u+x create-sdcard.sh
sudo ./create-sdcard.sh

NOTE: When prompted for which hard drive to flash, type the number that matches your SD card size and hit Enter.
NOTE: When prompted for number of partitions, type "2" and hit Enter.
NOTE: When prompted for erasing the filesystem previously mounted on your SD card, type "y" and hit Enter.
NOTE: When prompted for file path to install from, type "2" and hit Enter.
NOTE: When prompted for boot partition path, type "./img/boot/" and hit Enter.
NOTE: When prompted to confirm the boot partition path, type "y" and hit Enter.
NOTE: When prompted for kernel image and device tree files option, type "2" and hit Enter.
NOTE: When prompted for kernel image and device tree files path "./img/kernel/"
NOTE: When prompted to confirm kernel image and device tree files path, type "y" and hit Enter.
NOTE: When prompted for rootfs path, type "./img/rootfs/tisdk-rootfs-image-am335x-evm.tar.xz" and hit Enter.

5. Unplug the SD card from the PC and insert it into the board adapter. Holding the S1 button, plug in the board in your PC using the USB cable.

NOTE: It is necessary that you keep holding the button until the blue LEDs in your board light up.

6. Execute the installation script.

chmod u+x install_script.sh
./install_script.sh

NOTE: When prompted for the authenticity of the host, type "yes" and hit Enter.

7. Enjoy! 
