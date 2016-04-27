#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>

// Fail2banMapper - v.0.1.2 (c) Matthew Wilson. 2015. 
// Plots a world map of IPs banned today by Fail2ban.
// Requires: Curl (to access ipinfo.io for Whois) and a map marker image.
// Update OUTPUTMAP variable to change location of saved map html files.
// Uses OpenLayers js API in map html files.
// License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
// No warranty. Software provided as is.

char* MARKERIMAGE="./marker.png"; // name of marker image file used on map

thefunc(int argcc, char*argvv[]) {

char* THELOGFILE="/var/log/fail2ban.log"; // usually /var/log/fail2ban.log
FILE* fp; // for log file
FILE* Sfp; // shell file pointer (for whois return)
char line[999]; // line from log
char* unparLin[1024]; // unparsed line
char* pars_IP[1024]; // parsed IP
char inc_IP[15]; // incoming IP
char inc_T[30]; // incoming time
char inc_S[7]; // incoming Ban or Unban
char* pars_T[1024]; // parsed time
char inc_LOC[30]; // incoming lat and long from whois
char* pars_LOC[1024]; // parsed location
char* pars_LAT[1024]; // parsed lat
char* pars_LONG[1024]; // parse long
char kw[4]="Ban"; // keyword to search for in log file

time_t rawtime;
struct tm* TI;
time(&rawtime);
TI=localtime(&rawtime);
int Year = TI->tm_year+1900;
int Month = TI->tm_mon+1;
int Today = TI->tm_mday;
int incyear, incmonth, incday;
char OUTPUTMAP[25]; // name of html output files. f2bmap.yy.mm.dd.htm

snprintf(OUTPUTMAP, sizeof OUTPUTMAP, "f2bmap.%d.%d.%d.htm", Year, Month, Today);

// program depends on curling ipinfo.io for whois info

if (access("/usr/bin/curl", F_OK) == -1) {
	printf("Error: Curl must be installed to run\n");
	exit(1);
} 

int res=0;
res = system("curl -s ipinfo.io > /dev/null 2>&1");

if (res != 0) {
	printf("Error: Curl unable to connect to network\n");
	exit(1);
}

// read lines from fail2ban log

fp=fopen(THELOGFILE, "r");

if (fp == NULL) {
	printf("Error: unable to read log file: %s\n", THELOGFILE);
	exit(1);
}

int x=0;

while (fgets(line, sizeof (line)-1, fp) != NULL) {
	unparLin[x]=malloc(strlen(line)+1);	
	strcpy(unparLin[x], line);
	unparLin[x][strcspn(unparLin[x], "\n")] = 0;
	x++;
}

fclose(fp);

int a;
int cnt=0;

// only keep lines that occur today and contain "Ban"

for (a=0; a<x; a++) {
	sscanf(unparLin[a], "%s %*s %*s %*s %*s %s %s", inc_T, inc_S, inc_IP);
	
	// get the time from line
	sscanf(inc_T, "%d-%d-%d", &incyear, &incmonth, &incday);	

	// if the incoming day = Today, and inc_S = Ban, then list. 
	// change line below to process all items: if strcmp(inc_S, kw) == 0) {
 	
	if ((incday==Today) && strcmp(inc_S, kw) == 0) {
		pars_T[cnt]=malloc(strlen(inc_T)+1);
		strcpy(pars_T[cnt], inc_T);	
		pars_IP[cnt]=malloc(strlen(inc_IP)+1);  
		strcpy(pars_IP[cnt], inc_IP);
		printf("%d: %s %s\n", cnt, pars_T[cnt], pars_IP[cnt]);
		cnt++;
	}
}

if (cnt == 0) {
	printf("no matching items in log file - ex:\'Today\'\n");
	exit(1);
}

int b;

// now run whois on all parsed IP's and store in pars_LOC[b]

char embuff[50];

for (b=0; b<cnt; b++) {
	snprintf(embuff, sizeof embuff, "curl -s ipinfo.io/%s/loc", pars_IP[b]);

	if ((Sfp = popen(embuff, "r")) == NULL) {
		printf("curl error\n");
		exit(1);
	}
	// get the incoming location from whois
	while (fgets(inc_LOC, sizeof inc_LOC, Sfp) != NULL) {
		pars_LOC[b]=malloc(strlen(inc_LOC)+1);	
		strcpy(pars_LOC[b], inc_LOC);
		pars_LOC[b][strcspn(pars_LOC[b], "\n")] = 0;
	}
}

pclose(Sfp);

// finally tokenize pars_LOC[b] into their own arrays for use in map

char lat[30];
char llong[30];

for (b=0; b<cnt; b++) {
	sscanf(pars_LOC[b], "%[^','], %s", lat, llong); 
	pars_LAT[b]=malloc(strlen(lat)+1);
	strcpy(pars_LAT[b], lat);	
	pars_LONG[b]=malloc(strlen(llong)+1);  
	strcpy(pars_LONG[b], llong);
	printf("%d: %s %s\n", b, pars_LONG[b], pars_LAT[b]);
}

// now generate the html map

mapgen(OUTPUTMAP, cnt, pars_LONG, pars_LAT, pars_IP);

exit(0);
}

// function to generate the maps
mapgen(char OUTPUTMAP[25], int cntr, char*pars_LONGG[], char*pars_LATT[], char* pars_IPP[]) { 

int x;
FILE* fp1;
char top_page[650];
char dropin[650];
char bottom_page[650];

// create the output map. remove and replace today's map

if (access(OUTPUTMAP, F_OK) == 0) {
	remove(OUTPUTMAP);
}

fp1 = fopen(OUTPUTMAP, "a");

if (fp1 == NULL) {
	printf("error creating output map html\n");
	exit(1);
}

printf("%s\n", OUTPUTMAP);

// this is the top of the html file

for (x=0; x<1; x++) {

snprintf(top_page, sizeof top_page, "<html>\n<head>\n<title>FAIL2BAN LOG MAP - %s</title>\n</head>\n<body>\n<div id=\"mapdiv\"></div>\n<script src=\"http://www.openlayers.org/api/OpenLayers.js\">\n</script>\n<script>\nmap=new OpenLayers.Map(\"mapdiv\");\nmap.addLayer(new OpenLayers.Layer.OSM());\nepsg4326=new OpenLayers.Projection(\"EPSG:4326\");\nprojectTo=map.getProjectionObject();\nvar lonLat=new OpenLayers.LonLat( 0,0 ).transform(epsg4326, projectTo);\nvar zoom=0;\nmap.setCenter (lonLat, zoom);\nvar vectorLayer=new OpenLayers.Layer.Vector(\"Overlay\");\n", OUTPUTMAP);

	fputs(top_page, fp1);
}

// fill in the lat and longs here

for (x=0; x<cntr; x++) {

snprintf(dropin, sizeof dropin, "\nvar feature=new OpenLayers.Feature.Vector(\nnew OpenLayers.Geometry.Point(%s, %s).transform(epsg4326, projectTo),\n{description:\' %s \'},\n{externalGraphic: \'%s\', graphicHeight:25, graphicWidth:21, graphicXOffset:-12, graphicYOffset:-25}\n);\nvectorLayer.addFeatures(feature);\n", pars_LONGG[x], pars_LATT[x], pars_IPP[x], MARKERIMAGE);
	
	fputs(dropin, fp1);
}

// fill in bottom lines of the html template

for (x=0; x<1; x++) {

snprintf(bottom_page, sizeof bottom_page, "\nvar controls={selector: new OpenLayers.Control.SelectFeature(vectorLayer, { onSelect: createPopup, onUnselect: destroyPopup })};\nfunction createPopup(feature) {feature.popup=new OpenLayers.Popup.FramedCloud(\"pop\", feature.geometry.getBounds().getCenterLonLat(),null,\'<div class=\"markerContent\">\'+feature.attributes.description+\'</div>\',null,true,function() { controls[\'selector\'].unselectAll(); });\nmap.addPopup(feature.popup);}\nfunction destroyPopup(feature) {feature.popup.destroy();feature.popup=null;}\n\nmap.addControl(controls[\'selector\']);\ncontrols[\'selector\'].activate();\nmap.addLayer(vectorLayer);\n</script>\n</body>\n</html>");

	fputs(bottom_page, fp1);
}

fclose(fp1);

}

main (int argc, char* argv[]) {
	if (argc == 1) {
		thefunc(argc, argv);
	}
	else {
		printf("err\n");
		exit(1);
	}
}
