# Compile
mkdir build
cd build
cmake ..
make

# Program board
Press both reset and boot button
copy build/src/quarkcnc_quad_driver.uf2 to rp2040 usb drive

Here is an example for MacOS:
cp src/quarkcnc_quad_driver.uf2 /volumes/RPI-RP2/

# Configure current
Current can be set with the DIP switch. For now, current is limited below 4.0 amps.
+0.0 | +0.5
+0.0 | +1.0
+0.0 | +2.0
+0.0 | +4.0
