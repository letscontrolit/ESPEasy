# Sensirion Gas Index Algorithm Arduino Library

Sensirion's Gas Index Algorithm software provides a VOC and an NOx Index output signal calculated from the SGP40/41 raw
signal inputs `SRAW_VOC` and `SRAW_NOX`. Note: for SGP40, only `SRAW_VOC` is available. This algorithm enables robust detection of
ambient changes of VOCs and NOx with minimal sensor-to-sensor variation. The algorithm is based on a statistical gain-offset
normalization and adapts both parameters constantly applying an exponentially decaying function of the learned parameters to
be able to adapt to changing environments.

For the VOC Index output, the software must be instanced as VOC Algorithm while for the NOx Index output, the software must
be instanced as NOx Algorithm. It is important to feed the raw signals to the corresponding algorithm (i.e., `SRAW_VOC` to the
VOC Algorithm and `SRAW_NOX` to the NOx Algorithm) at a constant sampling interval which must coincide with the sampling
interval that is used to read out the raw signals from the SGP40/41 sensor. The default sampling interval applied in the
algorithm is 1 s. In case, a different sampling interval should be used the definition of the sampling interval in the h.file
of the algorithm must be changed, too.

The algorithm calculates the VOC and NOx Index signals recursively from a single raw tick value of `SRAW_VOC` and `SRAW_NOX`,
respectively, which are both measured by the SGP40/41 sensor at each time step, as well as internal states that are updated
at each time step. These internal states are most importantly the recursively estimated mean and variance of the
corresponding `SRAW` signal as well as some additional internal states such as uptime and other counters. After estimating the
states, the algorithm converts the raw signals in ticks into either VOC or NOx Index, respectively, and applies an adaptive
low-pass filter.

# Installation

To install, download the latest release as .zip file and add it to your
[Arduino IDE](http://www.arduino.cc/en/main/software) via

	Sketch => Include Library => Add .ZIP Library...

# Dependencies

The dependency to the Sensirion I2C drivers are only needed to run the example, which uses a SGP40 resp SGP41 and a SHT4x sensor.

* [Sensirion I2C SGP41](https://github.com/Sensirion/arduino-i2c-sgp41)
* [Sensirion I2C SGP40](https://github.com/Sensirion/arduino-i2c-sgp40)
* [Sensirion I2C SHT4x](https://github.com/Sensirion/arduino-i2c-sht4x)


## Quick Start to run the example

The example measures VOC and NOx ticks with a SGP41 sensor using a SHT4x to compensate temperature and humidity.
The raw VOC and NOx measurement signals are then processed with the gas index algorithm to get VOC Index and NOx Index values.

For more details about the sensors and breakout boards check out http://sensirion.com/my-sgp-ek/.

1. **Install the driver dependencies** listed above the same way as you installed this library (via `Add .ZIP Library`)


2. Connect a SGP41 and SHT4x Sensor over I2C to your Arduino


3. Open the `exampleUsage` sample project within the Arduino IDE

        File => Examples => Sensirion Gas Index Algorithm => exampleUsage

4. Click the `Upload` button in the Arduino IDE or

        Sketch => Upload

5. When the upload process has finished, open the `Serial Monitor` or `Serial
   Plotter` via the `Tools` menu to observe the measurement values and calculated 
   Gas Index value. Note that the `Baud Rate` in the corresponding window has to be set to `115200 baud`.


### Low power example (SGP40)
The provided low power example demonstrate how to run the SGP40 sensor in low power mode and apply the VOC index algorithm to the acquired data. 

Reduced power consumption is achieved by turning off the heater after each measurement. The heater is then turned back on by calling for a first ignored measurement that preceeds the actual measurement call by 170ms.

The following two low power modes have been tested:
Duty cycle | Sampling interval | Average  power  consumption at 1.8V
 --- | --- | --- 
Continuous | 1 s | 6.3mW
20%| 1 s | <2.0mW
2%| 10 s | <0.2mW




# Contributing

**Contributions are welcome!**

We develop and test this algorithm using our company internal tools (version
control, continuous integration, code review etc.) and automatically
synchronize the master branch with GitHub. But this doesn't mean that we don't
respond to issues or don't accept pull requests on GitHub. In fact, you're very
welcome to open issues or create pull requests :)

This Sensirion library uses
[`clang-format`](https://releases.llvm.org/download.html) to standardize the
formatting of all our `.cpp` and `.h` files. Make sure your contributions are
formatted accordingly:

The `-i` flag will apply the format changes to the files listed.

```bash
clang-format -i src/*.cpp src/*.h
```

Note that differences from this formatting will result in a failed build until
they are fixed.

# License

See [LICENSE](LICENSE).
