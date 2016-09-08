# Makefile for Clip
# Targets are:
#
# all			  Build and link everything
# install		  Install clip to /usr/bin

# Default build targets, according to OS
ifeq ($(OS),Windows_NT)
BUILD_TARGET = x86_windows
else
ifeq ($(shell uname), Darwin)
BUILD_TARGET = ppc_osx
else
BUILD_TARGET = x86_linux
endif
endif

all:
	mkdir -p bin/$(BUILD_TARGET)
	gcc -O2 src/clip.c -o bin/$(BUILD_TARGET)/clip

install:
	cp bin/$(BUILD_TARGET)/clip /usr/bin/clip