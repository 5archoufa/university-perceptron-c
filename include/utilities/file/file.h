#ifndef UTILITIES_FILE_H
#define UTILITIES_FILE_H

#include <stdio.h>
#include <stdlib.h>

char *File_LoadStr(const char *filename);
unsigned char *File_LoadBinary(const char *filename);

#endif