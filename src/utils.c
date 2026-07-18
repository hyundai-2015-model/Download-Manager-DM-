#include "utils.h"
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>

void log_msg(const char *msg)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    printf(
        "[%02d:%02d:%02d] %s\n",
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        msg
    );
}

int get_json_files(
    const char *dir_path,
    char json_files[][PATH_MAX],
    int max_files
)
{
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    dir = opendir(dir_path);

    if (dir == NULL)
    {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        // Skip "." and ".."
        if (
            strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0
        )
            continue;

        char fullpath[PATH_MAX];

        snprintf(
            fullpath,
            sizeof(fullpath),
            "%s/%s",
            dir_path,
            entry->d_name
        );

        struct stat st;

        if (stat(fullpath, &st) != 0)
            continue;

        // Skip directories
        if (!S_ISREG(st.st_mode))
            continue;

        // Check if file ends with ".json"
        const char *ext = strrchr(entry->d_name, '.');

        if (ext && strcmp(ext, ".json") == 0)
        {
            strncpy(
                json_files[count],
                fullpath,
                PATH_MAX - 1
            );

            json_files[count][PATH_MAX - 1] = '\0';

            count++;

            if (count >= max_files)
                break;
        }
    }

    closedir(dir);

    return count;
}

char *text_extraction_from_json(const char *json)
{
    FILE *fp = fopen(json, "r");

    if (!fp)
    {
        printf("Can't open JSON file");
        return NULL;
    };

    fseek(fp, 0, SEEK_END);

    long size = ftell(fp);

    rewind(fp);

    char *buffer = malloc(size + 1);
    if (!buffer)
    {
        fclose(fp);
        return NULL;
    }

    fread(buffer, 1, size, fp);

    buffer[size] = '\0';

    fclose(fp);

    return buffer;
}

char *extract_value(const char *json, const char *key)
{
    char pattern[128];

    snprintf(
        pattern,
        sizeof(pattern),
        "\"%s\"",
        key
    );

    const char *p = strstr(json, pattern);

    if (!p)
        return NULL;

    p = strchr(p, ':');

    if (!p)
        return NULL;

    p++;

    while (*p == ' ' || *p == '"')
        p++;

    const char *end = strpbrk(p, "\",\n}");

    if (!end)
        return NULL;

    size_t len = end - p;

    char *value = malloc(len + 1);

    if (!value)
        return NULL;

    strncpy(value, p, len);

    value[len] = '\0';

    return value;
}