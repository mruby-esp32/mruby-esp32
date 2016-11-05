# Example of mruby on the ESP32

Before you get started you will need to follow the setup documentation from
the [esp-idf](https://github.com/espressif/esp-idf/tree/master/docs) project
for your specific operating system.

I have only tested this on macOS and using the most recent version of
[esp-idf](https://github.com/espressif/esp-idf/tree/abecab7525e7edb1fde16ab5d8cf7b368b1d332c).

You will need to recursively clone this project with the recursive flag
because it includes mruby as a submodule:

```
git clone --recursive https://github.com/carsonmcdonald/mruby-esp32.git
```

The main ruby program can be found in the `main/simplest_mrb.rb` file. The
makefile configuration in `main/component.mk` and the main entry point source 
file `mruby_main.c` will also be of interest if you want to change the name of
the ruby script. This is a very simple script that only prints to the ESP32's
debug console.

I'm assuming you have followed all the steps in the install documentation and
are at least somewhat familure with the building steps. With that in mind you
can do something like the following and see the example running:

```
make menuconfig
make MRUBY_EXAMPLE=simplest_mrb.rb
make MRUBY_EXAMPLE=simplest_mrb.rb flash
screen /dev/tty.SLAB_USBtoUART 115200
```

The flag `MRUBY_EXAMPLE` can be replaced with one of the following:

  * simplest_mrb.rb - Simply prints two strings
  * wifi_example_mrb.rb - An example of connecting to WiFi, you will need to
    modify this file to include your SSID and password
  * system_mrb.rb - Examples of most of the system APIs

The clean command will clean both the ESP32 build and the mruby build:

```
make clean
```

There are multiple GEMS that can be turned on and off via the mruby
configuration file found in
`components/mruby_component/esp32_build_config.rb`:

* mruby-esp32-system - ESP32 system calls
* mruby-esp32-wifi - ESP32 WiFi


