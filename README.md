# Example of mruby on the ESP32

Before you get started you will need to follow the setup documentation from
the [esp-idf](https://github.com/espressif/esp-idf/tree/master/docs) project
for your specific operating system.

I have only tested this on macOS and using a certain version of
[esp-idf](https://github.com/espressif/esp-idf/tree/abecab7525e7edb1fde16ab5d8cf7b368b1d332c).
You should try to use [more recent version](https://github.com/espressif/esp-idf#setting-up-esp-idf) if you have failed.

You will need to recursively clone this project with the recursive flag
because it includes mruby as a submodule:

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
make menuconfig
make MRUBY_EXAMPLE=simplest_mrb.rb
make MRUBY_EXAMPLE=simplest_mrb.rb flash monitor
```

The flag `MRUBY_EXAMPLE` can be replaced with one of the following:

  * _simplest_mrb.rb_ - Simply prints two strings
  * _wifi_example_mrb.rb_ - An example of connecting to WiFi, you will need to
    modify this file to include your SSID and password
  * _system_mrb.rb_ - Examples of most of the system APIs

## wifi\_example\_mrb.rb stack overflow

If you experience a stack overflow during execution of the WiFi example, please
adjust the stack size on file `main/mruby_main.c` from 8192 to 32768.

```
void app_main()
{
  nvs_flash_init();
  xTaskCreate(&mruby_task, "mruby_task", 32768, NULL, 5, NULL);
}

```

Also adjust the configured stack size using `make menuconfig` from the default
one (which may be either 2048 or 4096) to 32768.

```
make menuconfig
Component config ---> ESP32-specific ---> Event loop task stack size
```

References: Issue [#11](https://github.com/mruby-esp32/mruby-esp32/issues/11)
and
[mruby-esp32\/mruby-socket](https://github.com/mruby-esp32/mruby-socket)

---

The clean command will clean both the ESP32 build and the mruby build:

```
make clean
```

There are multiple GEMS that can be turned on and off via the mruby
configuration file found in
`components/mruby_component/esp32_build_config.rb`:

* _mruby-esp32-system_ - ESP32 system calls
* _mruby-esp32-wifi_ - ESP32 WiFi


