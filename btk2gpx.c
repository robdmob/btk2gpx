#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

const char gpxheader[] = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n<gpx version=\"1.1\" creator=\"Robbie\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" xmlns:ns3=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\">\n<trk>\n<trkseg>\n";

FILE *btkfile;
FILE *gpxfile;
time_t timestamp;

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

	char timebuf[24];
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H%M%S.gpx", &timeinfo);

	gpxfile = fopen(timebuf,"w");

	fprintf(gpxfile, gpxheader);

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

int main(int argc, char *argv[]) {

	btkfile = fopen("Upload files.btk","r");

	fseek(btkfile, 24, SEEK_SET);

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

	exit(0);

}
