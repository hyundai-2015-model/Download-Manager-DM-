#include "config.h"
#include "downloader.h"
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "http.h"
#include "utils.h"
#include <sys/stat.h>
#include <errno.h>

void extract_filename(const char *url, char *filename)
{
    const char *qmark = strchr(url, '?');
    char clean_url[512];

    if (qmark)
    {
        size_t len = qmark - url;
        strncpy(clean_url, url, len);
        clean_url[len] = '\0';
    }
    else
    {
        strcpy(clean_url, url);
    }

    const char *last_slash = strrchr(clean_url, '/');

    if (last_slash && *(last_slash + 1) != '\0')
    {
        strcpy(filename, last_slash + 1);
    }
    else
    {
        strcpy(filename, "downloaded_file");
    }
}

void make_unique_filename(const char *original, char *unique, size_t size)
{
    if (mkdir(config.download_dir, 0755) != 0)
    {
        if (errno != EEXIST)
        {
            perror("mkdir Downloads");
            return;
        }
    }

    char name[256];
    char ext[64];
    char fullpath[512];

    const char *dot = strrchr(original, '.');

    if (dot)
    {
        size_t len = dot - original;
        strncpy(name, original, len);
        name[len] = '\0';
        strcpy(ext, dot);
    }
    else
    {
        strcpy(name, original);
        ext[0] = '\0';
    }

    strcpy(unique, original);

    int counter = 1;
    FILE *f;

    snprintf(fullpath, sizeof(fullpath), "%s/%s", config.download_dir, unique);

    while ((f = fopen(fullpath, "rb")) != NULL)
    {
        fclose(f);

        snprintf(unique, size, "%s(%d)%s", name, counter++, ext);
        snprintf(fullpath, sizeof(fullpath),"%s/%s", config.download_dir, unique);
    }
}

void start_download(const char *url)
{
    FILE *fp = NULL;

    char filename[256];
    char unique_filename[256];
    char final_name[256];
    char path[512];
    char temp_path[512];
    char json_file[512];

    extract_filename(url, filename);

    make_unique_filename(filename, unique_filename, sizeof(unique_filename));

    snprintf(final_name, sizeof(final_name),"%s.part",unique_filename);
    snprintf(path, sizeof(path), "%s/%s", config.download_dir, unique_filename);
    snprintf(json_file, sizeof(json_file), "%s/%s.json", config.download_dir, unique_filename);
    snprintf(temp_path, sizeof(temp_path), "%s/%s.part", config.download_dir, unique_filename); 

    FILE *placeholder = fopen(path, "wb");
    FILE *jsonholder = fopen(json_file, "wb");

    if (placeholder && jsonholder)
    {
        fclose(placeholder);
        fclose(jsonholder);
    }

    fp = fopen(temp_path, "wb");

    if (!fp)
    {
        remove(path);
        log_msg("Failed to open file");
        return;
    }

    int on_success = http_download(url, fp, json_file, final_name, 0, 0);

    fclose(fp);

    if (on_success)
    {
        remove(path);
        remove(json_file);

        if (rename(temp_path, path) != 0)
        {
            perror("rename");
        }
    }

}