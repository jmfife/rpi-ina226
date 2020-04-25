# RPi-INA226

Use INA226 chip in conjunction with a Raspberry Pi to measure DC voltage and current.  The default response
is JSON and it can be piped into the [BigBird](https://github.com/jmfife/bigbird) PV + battery system monitor.

Repo: https://github.com/jmfife/rpi-ina226

Adapted from: https://github.com/MarioAriasGa/raspberry-pi-ina226

## How to Compile

Enable I2C using `raspi-config` or make sure that this line is uncommented in the `/boot/config.txt` and reboot if necessary.

```dtparam=i2c_arm=on```

```
$ sudo apt-get install wiringpi i2c-tools libi2c-dev
$ make
```

## Hardware Set-Up

This should work with any Raspberry Pi Model.

Connect GND, SDA, SCL pins from the INA226 chip to the corresponding i2c Raspberry Pi pins. See https://pinout.xyz/

Connect Vcc to one of the 3.3v pins of the Raspberry (Note: the chip can handle 5V but the pull-ups would give the 
raspberry pi 5V which is not desirable, even through a resistor)

Connect the V+ pin to our Power supply (Make sure not to put more than 36V DC!! Never AC!)

Connect the V- pin to the + of the load to be measured.

Connect VBus to V- pin to measure voltage of the load.

Connect GND of the load to GND of the Pi (and the INA226 chip).

## How to Configure

The INA226 chip needs a shunt resistor, a very small resistor that goes in series of the load between 
pins V+ and V-. Check where it is in your PCB and find the ohms from the number written (Tip: it is usually 
the largest size in volume and smallest in value) or measure the resistance between V+ and V- with a multimeter 
with everything else disconnected. In my case it was 0.1 Ohm.

The INA226 chip measures the voltage drop between V+ and V- to calculate the current by using Ohm's law.

Some parameters you may want to change in the code:
* I2C address of the chip, 0x40 by default. Use `i2cdetect -y 1` to find out.
* Shunt resistor value.
* Max expected current, to set the resolution of the ADC.
* Time measuring the Bus Voltage (Load) and Shunt Voltage ( Current ). More time is smoother and less is quicker.
* How many values to average between 1 and 1024 (use the constants, not numbers). More averages has less noise but 
takes longer.
* Sleep value in the for loop.

## Example

The output is continuous JSON with Voltage in V, Current in mA.

For example:
```
$ ./ina226 --sph=720
{"ts": 1587848635.003850, "Voltage_V": 13.286, "Current_mA": 30.564}
{"ts": 1587848640.003581, "Voltage_V": 13.286, "Current_mA": 30.564}
{"ts": 1587848645.003553, "Voltage_V": 13.285, "Current_mA": 30.564}
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.