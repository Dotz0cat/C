#! /bin/bash

 clang -Weverything -g `pkg-config --cflags gtk+-3.0 gstreamer-1.0` ./main.c ./station_reader.c ./parser.c ./g-bus.c -o ./rajio `pkg-config --libs gtk+-3.0 sqlite3 gstreamer-1.0`