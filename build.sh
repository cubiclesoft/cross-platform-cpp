#!/bin/bash
gcc -m64 -std=c++0x -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -pthread -O3 convert/*.cpp security/*.cpp sync/*.cpp templates/*.cpp utf8/*.cpp test_suite.cpp -o test_suite -lstdc++ -lrt
