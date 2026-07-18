#include "config.h"

#include <stdio.h>
#include <string.h>

Config config;

int load_config(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        return 0;
    }

    char line[1024];

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "download_dir=", 13) == 0)
        {
            strcpy(config.download_dir, line + 13);

            char *p = strchr(config.download_dir, '\n');

            if (p)
            {
                *p = '\0';
            }
        }

        fclose(fp);

        return 1;
    }
}