# RPi-INA226

Use INA226 chip in conjunction with a Raspberry Pi to measure 
DC voltage and current.  The default response is JSON and it 
can be piped into [EMCNET](https://github.com/jmfife/emcnet).

Adapted from [Linux_I2C](https://github.com/craigpeacock/Linux_I2C) by Craig Peacock.

## Setup

Enable I2C using `raspi-config` or make sure that this line is uncommented in the `/boot/config.txt` and reboot if necessary.

```dtparam=i2c_arm=on```

```
$ sudo apt-get install i2c-tools libi2c-dev
$ git clone https://github.com/jmfife/rpi-ina226
$ cd rpi-ina226
$ make
```

Confirm the executable is working by:

```
$ ./ina226_monitor --help
```

## Hardware Set-Up

This should work with any Raspberry Pi Model.

Connect GND, SDA, SCL pins from the INA226 chip to the corresponding i2c Raspberry Pi pins.

Connect Vcc to one of the 3.3v pins of the Raspberry (Note: the chip can handle 5V but the pull-ups would give the 
raspberry pi 5V which is not desirable, even through a resistor)

Connect the V+ pin to our Power supply (Make sure not to put more than 36V DC!! Never AC!)

Connect the V- pin to the + of the load to be measured.

Connect VBus to V- pin to measure voltage of the load.

Connect GND of the load to GND of the Pi (and the INA226 chip).

## Operation

The INA226 chip needs a shunt, a very small-value resistor
that goes in series of the load between pins V+ and V-. 
Check where it is in your PCB and find the ohms from the number
written (Tip: it is usually the largest size in volume and 
smallest in value) or measure the resistance between V+ and V- 
with a multimeter with everything else disconnected. 
In my case it was ~1mOhm.

The INA226 chip measures the voltage drop between V+ and V- to
calculate the current by using Ohm's law.

Some parameters you may want to change in the code:
* I2C address of the chip, 0x40 by default. Use `i2cdetect -y 1`
(from `i2c-tools` which you installed in an earlier step above) to 
find out.
* Shunt resistor value.
* current_lsb, to set the resolution of the ADC.  (See INA226 data sheet.)

The output is continuous JSON with Voltage in V, Current in A, and
Power in W. Positive current, power are normally consumptive
(charging), but it depends on how you connect the shunt.  You can
change it by reversing the shunt V+ and V- leads.

## Example

```
$ ./ina226_monitor
{"time":1674069620002733824,"fields":{"V":13.345,"I":0.23,"P":3.1},"measurement":"ina226_0001"}
{"time":1674069630002486016,"fields":{"V":13.345,"I":0.234,"P":3.1},"measurement":"ina226_0001"}
{"time":1674069640002721024,"fields":{"V":13.35,"I":0.23,"P":3.1},"measurement":"ina226_0001"}
```

## How to Compile and run the emulator (does not require any special hardware)

On MacOS, argp-standalone is needed:

```
$ brew install argp-standalone
```

Then, build and run:

```
$ make ina226_emulate
gcc -c -o ina226-emulate.o ina226-emulate.c 
gcc -c -o AccumAvg.o AccumAvg.c
gcc -o ina226-emulate ina226-emulate.o AccumAvg.o -lm -largp -L /usr/local/opt/argp-standalone/lib 
./ina226_emulate
{"time": 1674070290000130816, "fields": {"V": 12.288, "I": 2.970, "P": 36.5}}
{"time": 1674070292000098048, "fields": {"V": 12.240, "I": 3.026, "P": 37.0}}
{"time": 1674070294000125952, "fields": {"V": 11.590, "I": 2.976, "P": 34.5}}
...
```

## How to Run Tests

```
$ make test
./test_accumavg 
OK!
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.