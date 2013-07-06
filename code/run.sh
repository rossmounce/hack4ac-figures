#! /bin/bash

g++ `pkg-config opencv --libs --cflags` --std=c++11 main.cpp && ./a.out $1
