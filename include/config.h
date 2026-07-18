#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PATH 512

typedef struct
{
    char download_dir[MAX_PATH];
} Config;

extern Config config;

int load_config(const char *filename);

#endif
