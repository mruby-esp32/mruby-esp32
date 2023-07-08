# Example of mruby on the ESP32

Before you get started, you will need to follow the setup documentation from
the [esp-idf](https://docs.espressif.com/projects/esp-idf/en/release-v4.4/esp32/index.html) project,
for your specific operating system.

This has been tested on macOS and Ubuntu Linux, using [esp-idf](https://github.com/espressif/esp-idf/tree/release/v4.4) version 4.4.
It will not work on newer IDF versions, and this branch is intended as a legacy release.

This example includes mruby 3.2.0. You may be able to update that to a newer version, but it isn't guaranteed.
If you want a more up-to-date version of this example, which works with IDF version 5+, please see the 0.5 branch/release.

## Installation

Recursively clone this repo to ensure the mruby submodule is downloaded:

```
git clone --recursive https://github.com/mruby-esp32/mruby-esp32.git
```

The main ruby program can be found in the `main/simplest_mrb.rb` file. The
makefile configuration in `main/component.mk` and the main entry point source
file `mruby_main.c` will also be of interest if you want to change the name of
the ruby script. The examples included are very simple scripts that only print
to the ESP32's debug console.

I'm assuming you have followed all the steps in the install documentation and
are at least somewhat familiar with the building steps. With that in mind you
can do something like the following and see the example running:

```
cp main/examples/$(YOU_WISH_TO_TRY_FILE) main/spiffs/main.rb
idf.py build
idf.py -p $(YOUR_SERIAL_PORT) flash monitor
```

The variable `YOU_WISH_TO_TRY_FILE` can be replaced with one of the following:

  * _simplest_mrb.rb_ - Simply prints two strings
  * _gpio.rb_ - An example of using GPIO
  * _wifi_example_mrb.rb_ - An example of connecting to WiFi, you will need to
    modify this file to include your SSID and password
  * _system_mrb.rb_ - Examples of most of the system APIs

The clean command will clean both the ESP32 build and the mruby build:

```
idf.py fullclean
```

There are multiple GEMS that can be turned on and off via the mruby
configuration file found in
`components/mruby_component/esp32_build_config.rb`:

* _mruby-esp32-system_ - ESP32 system calls
* _mruby-esp32-wifi_ - ESP32 WiFi
* _mruby-socket_ - BSD sockets, `TCPSocket` class
* _mruby-esp32-gpio_ - ESP32 GPIO access
