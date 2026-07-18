#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "http.h"
#include "utils.h"

void resume_download(char *file_path, char *json_path, char *simple_filepath)
{
    char *buffer = text_extraction_from_json(json_path);
    if (!buffer)
    {
        printf("\nFailed to extract metadeta.\n");
        return;
    }

    char *url = extract_value(buffer, "url");
    char *filename = extract_value(buffer, "filename");

    FILE *fp = fopen(file_path, "ab");
    if (!fp)
    {
        printf("Failed to open file.\n");

        free(buffer);
        free(url);
        free(filename);

        return;
    }

    fseek(fp, 0, SEEK_END);
    curl_off_t initial_downloaded = ftell(fp);

    int success = http_download(url, fp, json_path, filename, initial_downloaded, 1);

    if (success)
    {
        remove(json_path);
        remove(simple_filepath);

        if (rename(file_path, simple_filepath) != 0)
        {
            perror("rename");
        }
    }

    fclose(fp);

    free(buffer);
    free(url);
    free(filename);
}