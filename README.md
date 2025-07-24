# LCD2USB-OrangePi

This code called sys2lcd, design to display system information via the LCD2USB like this

```bash
CPU: 12.3% Temp:47C
RAM: 55.2% Uptime:1h
```

This is based on this code https://github.com/harbaum/LCD2USB

# Installation

1. Install the libusb
```
sudo apt update
sudo apt install libusb-dev
```

2. Clone the code
```
git clone https://github.com/thanhtantran/LCD2USB-OrangePi
cd LCD2USB-OrangePi
```

3. Compile the code and test
```
make
```
then test
```
sudo ./sys2lcd
```
if you see the LCD has system information then it is OK

4. Set up the LCD2USB to run without sudo
When run `lsusb` you should have this line
```
Bus 003 Device 002: ID 0403:c630 Future Technology Devices International, Ltd lcd2usb interface
```
Make the rule
```
sudo nano /etc/udev/rules.d/99-lcd2usb.rules
```
paste this line
```
SUBSYSTEM=="usb", ATTR{idVendor}=="0403", ATTR{idProduct}=="c630", MODE="0666"
```
then activate
```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

5. Create service so the app will run at start up
```
sudo nano /etc/systemd/system/sys2lcd.service
```
add this
```
[Unit]
Description=System info to LCD2USB
After=network.target

[Service]
ExecStart=/home/orangepi/sys2lcd/sys2lcd
WorkingDirectory=/home/orangepi/sys2lcd
User=orangepi
Group=orangepi
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```
then activate it
```
sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable sys2lcd.service
sudo systemctl start sys2lcd.service
```
# Demo with Orange PI CM5 on Orange PI CM5 Tablet Baseboard

![sys2lcd](https://github.com/user-attachments/assets/51680674-bdca-4533-be21-97c2d9e7726d)
