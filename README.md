# pad

Pad is a simple program that adds padding to a string

## Building

First copy the file for your os and arch to config.mk

E.g. for linux on amd64 do:

``` cp configs/linux-amd64 config.mk ```

Then just type make (or make install).

Note: For checking with valgrind copy linux-amd64-debug
or define _PAD_DEBUG. Otherwise valgrind will fail, due
to the seccomp filter.

## Known Bugs

There are no known bugs at the moment
