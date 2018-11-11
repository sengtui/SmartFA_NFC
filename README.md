This is a NFC reader sample code for PN532 chip on a Openwrt SoC.
Before compile please make sure you have setup your environment correctly, and revise the makefile accordingly

Required package: libsnap7, libmraa, libpthread

Cross-Compile environment:
on .bashrc, you may want to add following lines (directory name according to your SDK/Toolpath path)<br>

export STAGING_DIR=SDK_Path/staging_dir/target-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2<br>
export TOOLCHAIN_DIR=Toolchain_Path/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2<br>
export CPPFLAGS=-I$STAGING_DIR/usr/include<br>
export LD_LIBRARY_PATH=$STAGING_DIR/usr/lib<br>
export PATH=$PATH:$TOOLCHAIN_DIR/bin<br>

