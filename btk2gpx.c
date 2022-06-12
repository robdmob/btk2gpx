#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#ifdef _WIN32
#include <conio.h>
#include <Windows.h>
#endif

const char gpxheader[] = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n<gpx version=\"1.1\" creator=\"btk2gpx\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" xmlns:ns3=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\">\n<trk>\n<trkseg>\n";
const unsigned char btkheader[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x0a, 0x87, 0x40, 0xcd, 0x8c, 0x57, 0x44, 0x66, 0x66, 0xa6, 0x3f, 0x01, 0x00, 0x05, 0x00};

struct btkstart {
	unsigned char type;
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	char padding[15];
};

struct btkpoint {
	unsigned char type;
	char temperature;
	unsigned short count;
	int elevation;
	double latitude;
	double longitude;
};

FILE *btkfile;
FILE *gpxfile;
time_t timestamp;
#ifdef _WIN32
char driveLetter = 0;
#endif

void writeHeader();
void writePoint();
void writeFooter();
#ifdef _WIN32
void findBackTrack();
void clearBackTrack();
#endif

int main(int argc, char *argv[]) {

	if (argc < 2) argv[1] = "Upload files.btk";

	btkfile = fopen(argv[1],"rb");

#ifdef _WIN32
	if ((btkfile == NULL) && (argc < 2)) findBackTrack();
#endif

	if (btkfile == NULL) {
		fprintf(stderr, "File \"%s\" not found.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	for (unsigned char i = 0; i < sizeof(btkheader); i++) {
		if (fgetc(btkfile) != btkheader[i]) {
			fclose(btkfile);
			fprintf(stderr, "File \"%s\" not valid BTK file.\n", argv[1]);
			exit(EXIT_FAILURE);
		}
	}

	while (!feof(btkfile)) {
		switch(fgetc(btkfile)) {
			case 2:
				writePoint();
				break;
			case 3:
				writeHeader();
				break;
			case 4:
				writePoint();
				writeFooter();
				break;

		}
	}

	fclose(btkfile);

#ifdef _WIN32
	if (driveLetter != 0) clearBackTrack();
#endif

	exit(EXIT_SUCCESS);

}

void writeHeader() {

	fseek(btkfile, -1, SEEK_CUR);

	struct btkstart start;

	fread(&start, sizeof(start), 1, btkfile);

	char filename[24];
	sprintf(filename, "%d-%.2d-%.2d %.2d.%.2d.%.2d.gpx", start.year, start.month, start.day, start.hour, start.minute, start.second);

	gpxfile = fopen(filename,"w");

	if (gpxfile == NULL) {
		fclose(btkfile);
		fprintf(stderr, "Couldn't create file \"%s\".\n", filename);
		exit(EXIT_FAILURE);
	}

	fprintf(gpxfile, gpxheader);

	printf("Created file \"%s\".\n", filename);

	struct tm timeinfo;
	timeinfo.tm_year = start.year - 1900;
	timeinfo.tm_mon = start.month - 1;
	timeinfo.tm_mday = start.day;
	timeinfo.tm_hour = start.hour;
	timeinfo.tm_min = start.minute;
	timeinfo.tm_sec = start.second;
	timeinfo.tm_isdst = 0;

	timestamp = mktime(&timeinfo);

}

void writeFooter() {

	fprintf(gpxfile, "</trkseg>\n</trk>\n</gpx>\n");
	fclose(gpxfile);

}

void writePoint() {

	fseek(btkfile, -1, SEEK_CUR);

	struct btkpoint point;

	fread(&point, sizeof(point), 1, btkfile);

	if((point.latitude > -200) && (point.longitude > -100)) {

		time_t offset = timestamp + point.count * 2;

		char timebuf[21];
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&offset));

		fprintf(gpxfile, "<trkpt lat=\"%.13f\" lon=\"%.13f\">\n", point.latitude, point.longitude);
		fprintf(gpxfile, "<ele>%.1f</ele>\n<time>%s</time>\n", point.elevation / 10.0, timebuf);
		fprintf(gpxfile, "<extensions>\n<ns3:TrackPointExtension>\n");
		fprintf(gpxfile, "<ns3:atemp>%.1f</ns3:atemp>\n", point.temperature / 2.0 + 20.0);
		fprintf(gpxfile, "</ns3:TrackPointExtension>\n</extensions>\n");
		fprintf(gpxfile, "</trkpt>\n");

	}

}

#ifdef _WIN32
void findBackTrack() {

    char filename[20] = "a:\\Upload files.btk";

    printf("Searching for BackTrack...\n");

    DWORD drivelist = GetLogicalDrives();

    for (int i = 0; i < 26; i++) {
        if (drivelist >> i & 1) {
            filename[0] = 'A' + i;
            if (btkfile = fopen(filename, "rb")) {
				driveLetter = filename[0];
				break;
			}
        }
    }

	if (btkfile != NULL) printf("BackTrack found on drive %c:\n", driveLetter);

}

void clearBackTrack() {

	printf("Delete routes from BackTrack? (Y/N)\n");

	if (_getch() == 'y') {

		char filename[20] = "a:\\Upload files.btk";

		filename[0] = driveLetter;

		if (remove(filename) != 0) {
			printf("Deleted \"%s\".\n", filename);

		}
		else {
			fprintf(stderr, "Delete file \"%s\" failed.\n", filename);
			exit(EXIT_FAILURE);
		}

	}

}
#endif
