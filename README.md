# Project doc

## Hints

1. If working on wsl on Windows 10 and running usbpid to forward the USB-JTAG device then you must on Ubuntu run `sudo service udev restart` and then replug the USB-JTAG interface. This is required to apply `plugdev` group for this device (/dev/ttyACM) for flashing purposes.

helper: usbpid command: `usbipd wsl attach -a -b 2-1`

1. If starting openocd server reports strange error like 'ftdi' not found or similar while using `*-building.cfg` then in `settings.json` file modify once a `"board/esp32s3-builtin.cfg"` path to fake one, try start opoenocd from VSCode, ignore an error. Then modify this setting for right one and start openocd. Now it should work. It seems like it needs to flush caches or something. 



## Hardware connection

#### GPIO
For gpio assignments see the `bsp` component.

#### Socket for sensors connection

|PIN number|Signal name| Cable color |
|------------|------------|----------|
|1|Vcc (3V)| Red |
|2|GND| Brown |
|3| OneWire signal for temperatures sensors - DS18B20. Note that on case side there is 4.5k pullup resistor| Yellow |
