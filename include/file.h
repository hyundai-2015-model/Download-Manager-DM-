#ifndef FILE_H
#define FILE_H

#include <stdio.h>

int create_file(const char *filename);

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);

#endif