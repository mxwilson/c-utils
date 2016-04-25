/*
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
No warranty. Software provided as is.
Copyright Matthew Wilson, 2016.
*/

// curl.c: downloader component of threatlevel

#include<stdio.h>
#include<getopt.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<curl/curl.h>
#include"tlheader.h"

// function for saving a downloaded page

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* stream) {
        size_t written;
        written = fwrite(ptr, size, nmemb, stream);
        return written;
}

// main curl function

int curl_func(void) {
FILE* f;
CURL* thehandle;
CURLcode res; // result of retrieval / error codes - 0 = CURLE_OK

// first initialize

curl_global_init(CURL_GLOBAL_ALL);

// and use the handle to capture upcoming transfer(s)

thehandle = curl_easy_init();

// now tell curl what to do

curl_easy_setopt(thehandle, CURLOPT_URL, urltopass);

// download and save xml file

f=fopen(savefile, "wb");

if (f == NULL) {
        printf("xml savefile error :(\n");
        exit(1);
}

curl_easy_setopt(thehandle, CURLOPT_WRITEFUNCTION, &write_data);
curl_easy_setopt(thehandle, CURLOPT_WRITEDATA, f);
res = curl_easy_perform(thehandle);
fclose(f);

// now check for error

if (res != CURLE_OK) {
	remove(savefile);
	printf("curl error\n");
	exit(1);
}

// end the curl section

curl_global_cleanup();

// call the xmlreader func
//parseDoc();

// delete xml file after use 
//remove(savefile);

char embuff[50];
// remove all <br> tags and replace with newlines
snprintf(embuff, sizeof embuff, "sed -i \'s/&lt;br&gt;/\\\\n/g' %s", savefile);
system(embuff);
printf("xml download success\n");
}
