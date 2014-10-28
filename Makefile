# Makefile for OS Project1: osm.c
TAR = ex1.tar
TAR_CMD = tar cvf
CC = gcc -Wall -O0

all: lib

lib: measure.o
	ar rcs libosm.a measure.o

measure: measure.o
	$(CC) measure.o -o measure

measure.o: osm.c osm.h
	$(CC) -c osm.c -o measure.o
	
clean:
	rm -f $(TAR) measure.o libosm.a measure 

tar: osm.c Makefile README
	$(TAR_CMD) $(TAR) osm.c README Makefile