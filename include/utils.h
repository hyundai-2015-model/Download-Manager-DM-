#ifndef UTILS_H
#define UTILS_H
#include <limits.h>

char *text_extraction_from_json(const char *json);
void log_msg(const char *msg);
int get_json_files(const char *dir_path, char json_files[][PATH_MAX], int max_files);
char *extract_value(const char *json, const char *key);

#endif