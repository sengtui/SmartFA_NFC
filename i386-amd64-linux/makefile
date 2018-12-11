
SHELL	:= /bin/bash
LIBS     :=  -lsnap7 
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=-O3
CFLAGS   :=
.PHONY: all clean

server.o : ../server.cpp
	$(CXX) $(CXXFLAGS) -c ../server.cpp

snap7.o : ../snap7.cpp
	$(CXX) $(CXXFLAGS) -c ../snap7.cpp

s7simulator: snap7.o server.o
	$(CXX)  $(CXXFLAGS) -o s7simulator snap7.o server.o $(LIBS)


all:  s7simulator

clean:
	$(RM) s7simulator *.o

install: s7simulator
	sudo chown root s7simulator; sudo chmod 4750 s7simulator

