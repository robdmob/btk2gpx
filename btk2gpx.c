#include "btk2gpx.h"

int main(int argc, char *argv[]) {

	char *filename = "Upload files.btk";
	if (argc > 1) filename = argv[1];
	btkfile = fopen(filename,"rb");

#ifdef _WIN32
	char driveLetter = 0;
	if (!btkfile) driveLetter = findBackTrack();
#endif

	if (!btkfile) {
		fprintf(stderr, "File \"%s\" not found.\n", filename);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < sizeof(btkheader); i++) {
		if (fgetc(btkfile) != btkheader[i]) {
			fclose(btkfile);
			fprintf(stderr, "File \"%s\" is not a valid BackTrack file.\n", filename);
			exit(EXIT_FAILURE);
		}
	}

	struct btkdata data;

	while (fread(&data, sizeof(data), 1, btkfile)) {

		if (data.type == 3) writeHeader(&data);

		else if ((data.type == 2) || (data.type == 4)) writePoint(&data);
		
		else {
			fprintf(stderr, "Error reading \"%s\".\n", filename);
			if (gpxfile) fclose(gpxfile);
			break;
		}

	}

	fclose(btkfile);

#ifdef _WIN32
	if (driveLetter) clearBackTrack(driveLetter);
#endif

	exit(EXIT_SUCCESS);

}

void writeHeader(struct btkdata *start) {

	char filename[24];
	sprintf(filename, "%d-%.2d-%.2d %.2d.%.2d.%.2d.gpx", start->year, start->month, start->day, start->hour, start->minute, start->second);

	gpxfile = fopen(filename,"w");

	if (gpxfile == NULL) {
		fclose(btkfile);
		fprintf(stderr, "Couldn't create file \"%s\".\n", filename);
		exit(EXIT_FAILURE);
	}

	fprintf(gpxfile, "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n<gpx version=\"1.1\" creator=\"btk2gpx\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" xmlns:ns3=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\">\n <trk>\n  <trkseg>\n");

	printf("Created file \"%s\".\n", filename);

	struct tm timeinfo;

	timeinfo.tm_year = start->year - 1900;
	timeinfo.tm_mon = start->month - 1;
	timeinfo.tm_mday = start->day;
	timeinfo.tm_hour = start->hour;
	timeinfo.tm_min = start->minute;
	timeinfo.tm_sec = start->second;
	timeinfo.tm_isdst = 0;

	timestamp = mktime(&timeinfo);

}

void writePoint(struct btkdata *point) {

	if((point->latitude > -200) && (point->longitude > -100)) {

		time_t offset = timestamp + point->count * 2;

		char timebuf[21];
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&offset));

		fprintf(gpxfile, "   <trkpt lat=\"%.13f\" lon=\"%.13f\">\n", point->latitude, point->longitude);
		fprintf(gpxfile, "    <ele>%.1f</ele>\n    <time>%s</time>\n", point->elevation / 10.0, timebuf);
		fprintf(gpxfile, "    <extensions>\n     <ns3:TrackPointExtension>\n");
		fprintf(gpxfile, "      <ns3:atemp>%.1f</ns3:atemp>\n", point->temperature / 2.0 + 20.0);
		fprintf(gpxfile, "     </ns3:TrackPointExtension>\n    </extensions>\n");
		fprintf(gpxfile, "   </trkpt>\n");

	}

	if (point->type == 4) {
		fprintf(gpxfile, "  </trkseg>\n </trk>\n</gpx>\n");
		fclose(gpxfile);
		gpxfile = NULL;
	}

}

#ifdef _WIN32
char findBackTrack() {

    printf("Searching for BackTrack...\n");

    DWORD drivelist = GetLogicalDrives();
    char filename[] = "a:\\Upload files.btk";

    for (int i = 0; i < 26; i++) {
        if ((drivelist >> i & 1) == 0) continue;
		filename[0] = 'A' + i;
		btkfile = fopen(filename, "rb");
		if (!btkfile) continue;
		printf("BackTrack found on drive %c:\n", filename[0]);
		return filename[0];
	}

	fprintf(stderr, "BackTrack not found.\n");
	return 0;

}

void clearBackTrack(char drive) {

	printf("Delete routes from BackTrack? (y/N)\n");

	if (_getch() != 'y') return;

	char filename[] = "a:\\Upload files.btk";
	filename[0] = drive;

	if (remove(filename) == 0) printf("Routes deleted.\n");
	else {
		fprintf(stderr, "Error deleting routes.\n");
		exit(EXIT_FAILURE);
	}
	
}
#endif