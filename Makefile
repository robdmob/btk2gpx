CC=gcc
CFLAGS=-I.

btk2gpx: btk2gpx.o
	$(CC) -o btk2gpx btk2gpx.o
