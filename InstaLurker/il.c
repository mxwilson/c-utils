/*
INSTALURKER v0.1 
InstaLurker is an Instagram scraper that downloads images. 
License GPLv3+: GNU GPL version 3 or later: http://gnu.org/licenses/gpl.html
No warranty. Software provided as is.
Copyright Matthew Wilson, 2015.

Program uses the Jansson library at http://www.digip.org/jansson/
To compile: gcc il.c -o il -ljansson 
To run: ./il <username>
*/

#include<jansson.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>

// prototype
int grepper(char username[50]);
int jsonparser(char jsonfilename[160], char username[50]);

// ops:
// use argv[1] as the username
// download the index file with wget and place in ./username/index.html
// then grep the index file and transform it just into a json file
// then parse the json, put all img urls in array
// then wget each url and download each image

int downloader(int argc, char* argv[]) {
char sitename[50]="http://instagram.com/";
char username[50];
char embuff[50];
char filemover[60];

strcpy(username, argv[1]);
strcat(sitename, username);
snprintf(embuff, sizeof embuff, "wget -q %s/", sitename);
system(embuff);

// wget the index.html file for the username
// if no 404 will download the index. if no index, no user 

if (access("./index.html", F_OK) == -1) {
	printf("no user found\n");
	exit(1);
}

// then construct a string ./username/index.html

strcpy(filemover, "./");
strcat(filemover, username);
strcat(filemover, "/index.html");
printf("%s\n", filemover);

// then create a directory for the user and move the index file there

if (access(username, F_OK) == -1) {
	if (mkdir(username, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) == -1) {
		printf("unable to create user folder\n");
		exit(1);
	}
}

rename("index.html", filemover); 

// now call the grepper

grepper(username);

}

int grepper(char username[50]) {
FILE* fp;
FILE* fp2;
char line[17000];
char line2[17000];
char embuff2[350];
char filename[160];
char outputfilename[160];
char jsonfilename[160];

// create two strings: /username/index.html and /username/gf (grepfile)

strcpy(filename, username);
strcat(filename, "/index.html");

strcpy(outputfilename, username);
strcat(outputfilename, "/gf"); 

// grep index.html > grepped file
// start grepping after word "nodes" and stop at first appearance of ]

snprintf(embuff2, sizeof embuff2, "grep -o 'nodes.*' ./%s | cut -f2- -d ':' | cut -d ']' -f1 > ./%s", filename, outputfilename);

//printf("%s\n", embuff2);
//printf("%s\n", outputfilename);

system(embuff2);

// read grepped file

if (!(fp=fopen(outputfilename, "r"))) {
	printf("Error: No file to grep\n");
	exit(1);
}

// get the line

fgets(line, sizeof line, fp);

strcpy(line2, line);

// remove newline

size_t ln = strlen(line2) - 1;
if (line2[ln] == '\n') {
        line2[ln] = '\0';
}

// concat the missing bracket to end of grepped file

strcat(line2, "]");

printf("%s", line2);

fclose(fp);

// now that line is stripped of js, put that into a json file and delete grepped file

// first create ./username/username.json string

strcpy(jsonfilename, "./");
strcat(jsonfilename, username);
strcat(jsonfilename, "/");
strcat(jsonfilename, username);
strcat(jsonfilename, ".json");

//printf("%s\n", jsonfilename);

// now write the line into ./username/username.json

if ((fp2=fopen(jsonfilename, "w")) == NULL) {
	printf("Error creating json file\n");
	exit(1);
}

else {
	fputs(line2, fp2);	
	fclose(fp2);
}

// then delete the original grepped file

if (remove(outputfilename) == -1) {
	printf("Error cleaning up grepped file\n");
	exit(1);
}

// now to parse the json file

jsonparser(jsonfilename, username);

}

int jsonparser(char jsonfilename[160], char username[50]) {

FILE* fp;
char line[15000];
size_t i;
json_t *root;
json_error_t error;

// open the saved json file: ./username/username.json

fp=fopen(jsonfilename, "r");

if (fp == NULL) {
        printf("Error reading json file\n");
        exit(1);
}

fgets(line, sizeof line, fp);

// remove newline

size_t ln = strlen(line) - 1;
if (line[ln] == '\n') {
        line[ln] = '\0';
}

// load the root of the json file

root=json_loads(line, 0, &error);
//free(line);

if (!root) {
        printf("Json error on line:%d. text:%s\n", error.line, error.text);
        exit(1);
}

// requires object to start and end with [ & ]  ?

if (!json_is_array(root)) {
        printf("Error: root not array\n");
        json_decref(root);
        exit(1);
}

// now loop thru it

json_t *data;
json_t *title;
json_t *type;
char* msg;
char* urlarr[1000];

//char* codearr[10000];
// other potential arrs of items from the json file

for (i=0; i < json_array_size(root); i++) {

        data = json_array_get(root, i);

        if(!json_is_object(data)) {
                printf("Error: %d data is not object\n", i+1);
                json_decref(root);
                return 1;
        }

	title = json_object_get(data, "code");
	type = json_object_get(data, "display_src");

	// so as of here, get the url for each image and put in urlarr[i]

	urlarr[i]=malloc(strlen(json_string_value(type))+1);
	strcpy(urlarr[i], json_string_value(type));
	printf("%d: %s\n", i, urlarr[i]);
}

fclose(fp);

// Download the images. First create subdir to save the images: ./username/img

char imagedir[60];

strcpy(imagedir, "./");
strcat(imagedir, username);
strcat(imagedir, "/img/");

// create if it doesn't exist

if (access(imagedir, F_OK) == -1) {
	if (mkdir(imagedir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) == -1) {
		printf("Unable to create image dir\n");
		exit(1);
	}
}

//printf("%d - %s\n", i, imagedir);

// now download the images to ./username/img/imgname.

int x;
char snbuff[300];

for (x=0; x < i; x++) {
	snprintf(snbuff, sizeof snbuff, "wget -P %s -nc %s", imagedir, urlarr[x]);
	system(snbuff);
}

exit(0);

}

int main(int argc, char* argv[]) {

if (argc != 2) {
	printf("Must provide username\n");
	exit(0);
}

if (argc == 2) {
	// remove potential slash at end of line

	size_t ln = strlen(argv[1]) - 1;

	if (argv[1][ln] == '/') {
        	argv[1][ln] = '\0';
	}
	
	downloader(argc, argv);
}

}
