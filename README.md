# mruby on the ESP32

This is an ESP-IDF project template running mruby on the ESP32 microcontroller.

To get started, you need to install the ESP-IDF, by following the instructions
[here](https://docs.espressif.com/projects/esp-idf/en/release-v5.1/esp32/get-started/index.html),
for your operating system.

This has been tested on macOS and Ubuntu Linux, using 
[ESP-IDF 5.1](https://github.com/espressif/esp-idf/tree/release/v5.1).

## Usage

Recursively clone this repo to ensure the mruby (3.2.0) submodule gets downloaded:

```
git clone --recursive https://github.com/mruby-esp32/mruby-esp32.git
```

The makefile configuration is in `main/component.mk`. The entry point source
file is `mruby_main.c`. Once that starts, it looks for `storage/main.rb` and runs it in mruby.
You can change the expected filename in `mruby_main.c`, or simply save your scripts as `main.rb`
inside the `storage` subfolder.

### First Build

```
idf.py build
idf.py -p $(YOUR_SERIAL_PORT) flash monitor
```
Your ESP32 should write 2 lines of numbers to the console.

### Examples

The folder `main/examples` includes simple scripts demonstrating functionality.

Once you are familiar with the build process, try them with:

```
cp main/examples/$(EXAMPLE_FILENAME) main/storage/main.rb
idf.py build
idf.py -p $(YOUR_SERIAL_PORT) flash monitor
```

Replace `EXAMPLE_FILENAME` with one of the following:

  * `simplest.rb` - Prints two strings
  * `system.rb` - Demonstrates most system APIs
  * `gpio.rb` - GPIO blink example
  * `wifi_connect.rb` - Connects to WiFi
  * `wifi_socket.rb` - Connects to WiFi and makes a TCPSocket connection
  * `mqtt_publish.rb` - Connects to WiFi and publishes to MQTT broker
  * `filesystem.rb` - Write/append/read a file on the virtual filesystem
  * `ledc_breathe.rb` - Gradually fades the brightness of an LED up and down
  * `ledc_buzzer.rb` - Plays a melody on a piezo-electric buzzer
  * `ledc_servo.rb` - Controls position of a 180 degree hobby servo motor
  
**Note**: Edit GPIO numbers to match ones you are connected to, insert your WiFI credentials, customize MQTT settings etc.

### Build Customization

There are multiple gems that can be turned on and off via the mruby
configuration file found in
`components/mruby_component/esp32_build_config.rb`:

* [_mruby-fileio_](https://github.com/mruby-esp32/mruby-fileio/tree/0.5) - ESP32 File class (Not much space, but good for config data)
* [_mruby-socket_](https://github.com/mruby-esp32/mruby-socket/tree/0.5) - ESP32 Socket library (Modified from mruby. IDF 5.1 minimum)
* [_mruby-esp32-system_](https://github.com/mruby-esp32/mruby-esp32-system/tree/0.5) - ESP32 system calls
* [_mruby-esp32-wifi_](https://github.com/mruby-esp32/mruby-esp32-wifi/tree/0.5) - ESP32 WiFi
* [_mruby-esp32-mqtt_](https://github.com/mruby-esp32/mruby-esp32-mqtt/tree/0.5) - ESP32 MQTT library
* [_mruby-esp32-gpio_](https://github.com/mruby-esp32/mruby-esp32-gpio/tree/0.5) - ESP32 GPIO library
* [_mruby-esp32-ledc_](https://github.com/mruby-esp32/mruby-esp32-ledc/tree/0.5) - ESP32 LEDC (PWM) library

To get gem changes to reflect in the build, `fullclean` the previous build, then build again:

```
idf.py fullclean
idf.py build
```

All gems are enabled by default, so you can try out the examples, but it's a good idea to disable ones you don't need.

## Hardware

Everything works on:
- Original ESP32: `idf.py set-target esp32`

Everything except gpio gem works on:
- ESP32-S2: `idf.py set-target esp32s2`
- ESP32-S3: `idf.py set-target esp32s3`

If you followed the IDF installation instructions correctly for your chip,
you can switch the project target with the corresponding command above.

You will not be able to build again until the partition table is reset to `partitions.csv`. To do this:

```
idf.py menuconfig
# Partition Table -> Partition Table (1st option) -> Custom partition Table CSV (Last options)
# Enter to select. Q to exit. Y to save
```

You may also want to manually change other settings like CPU clock speed, watchdog timers, stack sizes etc. Changing the target resets everything to default.

## Troubleshooting

The following files and folders are safe to delete when trying to solve build issues:
- `build`
- `components/mruby_component/build`
- `components/mruby_component/esp32_build_config.rb.lock`
- `managed_components`
- `dependencies.lock`
