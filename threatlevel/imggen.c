/*:
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
No warranty. Software provided as is.
Copyright Matthew Wilson, 2016.
*/

// imggen.c: function to create the images for threatlevel

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<time.h>
#include"tlheader.h" // includes colors and prototypes

int imgmaker(int a) {
	char timebuf[26]; 
	time_t thetime;
	struct tm* tm_info;
	time(&thetime);
	tm_info = localtime(&thetime);
	strftime(timebuf, 26, "%Y%m%d%H%M", tm_info);

	char embuff[3000];
	char thecolor[10];

	// test for images directory first

	if ((access(imgsavedir, X_OK) == -1)) {
		if (mkdir(imgsavedir, 0700) != -1) {
			printf("creating images save dir\n");
		}
		else {
			printf("error: unable to create: %s\n", imgsavedir);
			exit(EXIT_FAILURE);
		}
	}

	// assign some nice colors for the image background

	if (threatlevel == 1) {
		strcpy(thecolor, "#FF0000");
	}
	else if (threatlevel == 2) {
		strcpy(thecolor, "#FBDB0C");
	}
	else if (threatlevel == 3) {
		strcpy(thecolor, "#00CC00");
	}
	else if (threatlevel == 4) {
		strcpy(thecolor, "#0026FF");
	}
	else {
		strcpy(thecolor, "#FF0000");
	}

	// now to prevent large summaries from running off edge of image
	
	char* appender = " [...]";
	int len;
	int maxlen = 570;
	int trimlen;

	if ((strlen(theitems2[a]) > maxlen)) {
		len = strlen(theitems2[a]);
		trimlen = len - maxlen;
		theitems2[a][len - trimlen] = 0;
		strcat(theitems2[a], appender);
	}

	char* header = "EMERGENCY ALERT";

	// call imagemagick

	snprintf(embuff, sizeof embuff, "convert -size 450X350 -background '%s' -quality 100 -fill white -gravity north -font FreeSans-Bold -pointsize 30 caption:\' \n %s\' -trim -font DejaVu-Sans-Mono -pointsize 14 caption:\"THREAT LEVEL: %d\n%s\n\n%s\" -append -border 2 \'%s%s-%d-%d.jpg\'", thecolor, header, threatlevel, theitems[a], theitems2[a], imgsavedir, timebuf, a, threatlevel);

	printf("\n%s\n", embuff);
	system(embuff);  
}
