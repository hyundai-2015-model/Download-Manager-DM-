#ifndef HTTP_H
#define HTTP_H
#include <curl/curl.h>

#include <stdio.h>

int http_download(const char *url, FILE *fp, const char *json_file, const char *filename, curl_off_t initial_downloaded, int resume);

#endif