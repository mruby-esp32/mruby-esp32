# Family mruby

"Family mruby" is a project to run mruby on the original board(Narya board).

This project is under way, not officially released.

## How to use

- You need a [Narya development board](https://github.com/kishima/narya_board).
- ESP-IDF v3.2.2 is required. (Other version is not cofirmed)

1. Clone this repository

~~~
$ git clone --recursive https://github.com/kishima/family_mruby
~~~

2. Set serial port in sdkconfig(the board shall be connected to the host PC by a USB cable)

~~~
$ cd family_mruby
$ make menuconfig
(then set the serial port of the board)
~~~

3. Make and flash

~~~
$ make flash
~~~

In case you see following error message, please try `make` again.

~~~
make[1]: *** No rule to make target `cores/esp32/libb64/cencode.o', needed by `libarduino.a'.  Stop.
~~~

## Note

The mruby port is forked from https://github.com/mruby-esp32/mruby-esp32

