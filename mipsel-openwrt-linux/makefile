##
## LINUX barebone makefile for c++ examples : good for all platforms
##
## Simply run make or make clean
##
## Intend this makefile only as a "batch examples updater" after library modification.
##

IP	:= 192.168.101.126
SHELL	:= /bin/bash
Libs     := -ldl -lrt -pthread -lsnap7 -lmraa
CXX      := mipsel-openwrt-linux-g++
CC       := mipsel-openwrt-linux-gcc
CXXFLAGS :=-O3
CFLAGS   :=
.PHONY: all clean

%.o: ../%.cpp
	$(TOOLCHAIN_DIR)/bin/$(CXX) $(CXXFLAGS) -c $<


CardReader: pn532.o snap7.o nfc.o CardReader.o
	$(TOOLCHAIN_DIR)/bin/$(CXX) $(CXXFLAGS) -o CardReader CardReader.o pn532.o nfc.o snap7.o $(Libs)


all: CardReader

clean:
	$(RM) CardReader *.o

install: CardReader
	scp CardReader root@$(IP):/root/CardReader

