#ifndef TLHEADER_H
#define TLHEADER_H

/* ansi colors used in main */

#define ANSI_RED        "\x1b[31m"
#define ANSI_YELLOW     "\x1b[33m"
#define ANSI_GREEN      "\x1b[32m"
#define ANSI_BLUE       "\x1b[34m"
#define ANSI_RESET      "\x1b[0m"

/* prototypes for each additional function/file */

int curl_func(void);
int imgmaker(int);

/* global vars from main to be used in imggen.c */

extern int threatlevel;
extern char* imgsavedir;
extern char* savefile;
extern char* urltopass;
extern char* theitems[5000];
extern char* theitems2[5000];
extern char* theitems3[5000];

#endif 
