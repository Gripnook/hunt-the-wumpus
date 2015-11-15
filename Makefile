SHELL := /bin/bash
CXX := clang++
CXXFLAGS := -std=c++1y -Wall -O3
PROGNAME := hunt_the_wumpus

all: ${PROGNAME}

${PROGNAME}: main.o cave.o
	${CXX} ${CXXFLAGS} $^ -o ${PROGNAME}

main.o: main.cpp cave.h
cave.o: cave.cpp cave.h random.h

clean:
	rm -rf *.o ${PROGNAME}

