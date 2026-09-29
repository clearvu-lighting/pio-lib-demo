esptool.py --chip esp32s3 merge_bin \
-o ../bin/factory-firmware.bin \
0x0000 .pio/build/heltec-wsl-v3/bootloader.bin \
0x8000 .pio/build/heltec-wsl-v3/partitions.bin \
0xe000 /home/rkane/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin \
0x10000 .pio/build/heltec-wsl-v3/firmware.bin 
