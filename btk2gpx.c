#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
#include <conio.h>
#include <Windows.h>
#endif

const char gpxheader[] = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n<gpx version=\"1.1\" creator=\"btk2gpx\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" xmlns:ns3=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\">\n<trk>\n<trkseg>\n";
const unsigned char btkheader[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x0a, 0x87, 0x40, 0xcd, 0x8c, 0x57, 0x44, 0x66, 0x66, 0xa6, 0x3f, 0x01, 0x00, 0x05, 0x00};

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
		exit(1);
	}

	for (uint8_t i = 0; i < sizeof(btkheader); i++) {
		if (fgetc(btkfile) != btkheader[i]) {
			fprintf(stderr, "File \"%s\" not valid BTK file.\n", argv[1]);
			fclose(btkfile);
			exit(1);
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

	exit(0);

}

void writeHeader() {

	fseek(btkfile, 1, SEEK_CUR);

	uint16_t year;
	fread(&year, sizeof(year), 1, btkfile);

	struct tm timeinfo;
	timeinfo.tm_year = year - 1900;
	timeinfo.tm_mon = fgetc(btkfile) - 1;
	timeinfo.tm_mday = fgetc(btkfile);
	timeinfo.tm_hour = fgetc(btkfile);
	timeinfo.tm_min = fgetc(btkfile);
	timeinfo.tm_sec = fgetc(btkfile);

	timestamp = mktime(&timeinfo);

	fseek(btkfile, 15, SEEK_CUR);

	char filename[24];
	strftime(filename, sizeof(filename), "%Y-%m-%d %H.%M.%S.gpx", &timeinfo);

	gpxfile = fopen(filename,"w");

	if (gpxfile == NULL) {
		fprintf(stderr, "Couldn't create file \"%s\".\n", filename);
		fclose(btkfile);
		exit(1);
	}

	fprintf(gpxfile, gpxheader);

	printf("Created file \"%s\".\n", filename);

}

void writeFooter() {

	fprintf(gpxfile, "</trkseg>\n</trk>\n</gpx>\n");
	fclose(gpxfile);

}

void writePoint() {

	int8_t temp;
	uint16_t count;
	int32_t elev;
	double lat, lon;

	fread(&temp, sizeof(temp), 1, btkfile);
	fread(&count, sizeof(count), 1, btkfile);
	fread(&elev, sizeof(elev), 1, btkfile);
	fread(&lat, sizeof(lat), 1, btkfile);
	fread(&lon, sizeof(lon), 1, btkfile);

	if((lat > -200) && (lon > -100)) {

		time_t offset = timestamp + count * 2;

		char timebuf[21];
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&offset));

		fprintf(gpxfile, "<trkpt lat=\"%.13f\" lon=\"%.13f\">\n", lat, lon);
		fprintf(gpxfile, "<ele>%.1f</ele>\n<time>%s</time>\n", elev / 10.0, timebuf);
		fprintf(gpxfile, "<extensions>\n<ns3:TrackPointExtension>\n");
		fprintf(gpxfile, "<ns3:atemp>%.1f</ns3:atemp>\n", temp / 2.0 + 20.0);
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
			exit(1);
		}

	}

}
#endif
