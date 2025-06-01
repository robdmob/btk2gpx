#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <conio.h>
#include <Windows.h>
#endif

#pragma pack(1)

const uint8_t btkheader[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x0a, 0x87, 0x40, 0xcd, 0x8c, 0x57, 0x44, 0x66, 0x66, 0xa6, 0x3f, 0x01, 0x00, 0x05, 0x00};

struct btkdata {
	uint8_t type;
	union {
		struct {
			uint8_t timezone;
			uint16_t year;
			uint8_t month;
			uint8_t day;
			uint8_t hour;
			uint8_t minute;
			uint8_t second;			
		};
		struct {
			int8_t temperature;
			uint16_t count;
			int32_t elevation;
			double latitude;
			double longitude;			
		};
	};
};

FILE *btkfile;
FILE *gpxfile;
time_t timestamp;

void writeHeader(struct btkdata *start);
void writePoint(struct btkdata *point);
#ifdef _WIN32
char findBackTrack();
void clearBackTrack(char drive);
#endif
