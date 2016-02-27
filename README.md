# mercator
Program that runs as a daemon on a beaglebone black and calculates it's position

It is designed for the TI,BeagleBone Black with Linux version 3.8.13-bone79
Files like /boot/uEnv.txt and the location of slots may vary on other versions.

It requires the fftw library, available from http://www.fftw.org/

The Device Three Overlay EBB-PRU-ADC should be compiled and loaded on your BeagleBone.
The building is included in ./build. It can allso be achieved by executing 

      $ dtc -O dtb -o EBB-PRU-ADC-00A0.dtbo -b 0 -@ EBB-PRU-ADC.dts

You should place EBB-PRU-ADC-00A0.dtbo in /lib/firmware so the system can find this overlay.
Before you can load the overlay you have to disable HDMI.
This can be achieved changing /boot/uEnv.txt

There should be a line like : 

      ##Disable HDMI (v3.8.x)
      #cape_disable=capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN

to disable it, remove the #.:

      ##Disable HDMI (v3.8.x)
      cape_disable=capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN
  
If you have another version there should be a similar example available in this file.

Now you can load the DTO:

      # echo EBB-PRU-ADC > /sys/devices/bone_capemgr.9/slots

To enable the DTO being loaded at boot, edit /boot/uEnv.txt. You should find an example like :

      ##Example v3.8.x
      #cape_disable=capemgr.disable_partno=
      #cape_enable=capemgr.enable_partno=
  
Add the DTO here:

      ##Example v3.8.x
      #cape_disable=capemgr.disable_partno=
      cape_enable=capemgr.enable_partno=EBB-PRU-ADC

When adding a custom DTO to this file, you should allso add that DTO to /etc/default/capemgr
Add the following line.

      CAPE=EBB-PRU-ADC

You can now check weither the DTO is loaded by executing

      $ cat /sys/devices/bone_capemgr.9/slots

The result should look similar to this:
       0: 54:PF--- 
       1: 55:PF--- 
       2: 56:PF--- 
       3: 57:PF--- 
       4: ff:P-O-L Bone-LT-eMMC-2G,00A0,Texas Instrument,BB-BONE-EMMC-2G
       5: ff:P-O-- Bone-Black-HDMI,00A0,Texas Instrument,BB-BONELT-HDMI
       6: ff:P-O-- Bone-Black-HDMIN,00A0,Texas Instrument,BB-BONELT-HDMIN
       7: ff:P-O-L Override Board Name,00A0,Override Manuf,EBB-PRU-ADC
