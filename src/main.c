#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "downloader.h"
#include "resume.h"
#include "utils.h"
#include "config.h"

#define MAX_JSON_FILES 100

int main()
{
    char ans[3];
    char json_files[MAX_JSON_FILES][PATH_MAX];

    int count = get_json_files(config.download_dir, json_files, MAX_JSON_FILES);

    if (count < 0)
    {
        printf("Failed to scan directory.\n");
    }

    if (count > 0)
    {
        printf(":: Partially %d downloaded files found, do want to continue with these files? [Y/n] ", count);
        scanf("%2s", ans);
    }

    if (count == 0 || strcmp(ans, "n") == 0)
    {
        char url[256];

        printf("Enter download URL: ");
        scanf("%255s", url);

        start_download(url);
    }
    else if (count > 0 && strcmp(ans, "y") == 0)
    {
        printf("Found %d resume file(s):\n", count);

        for (int i = 0; i < count; i++)
        {
            char *buffer = text_extraction_from_json(json_files[i]);
            char *filename = extract_value(buffer, "filename");

            if (filename)
            {
                char *dot = strrchr(filename, '.');
                if (dot) {*dot = '\0';};
                

                printf("%s ", filename);
                free(filename);
            }

            free(buffer);
        }

        char resume_filename[256];
        char resume_filepath[256];
        char json_resume_filepath[256];
        char simple_filepath[256];

        FILE *file_exist = NULL;

        do
        {
            printf("\n:: Enter filename from above: ");
            scanf("%255s", resume_filename);

            snprintf(resume_filepath, sizeof(resume_filepath), "%s/%s.part", config.download_dir, resume_filename);

            snprintf(json_resume_filepath,sizeof(json_resume_filepath),"%s/%s.json", config.download_dir, resume_filename);

            snprintf(simple_filepath,sizeof(simple_filepath),"%s/%s", config.download_dir, resume_filename);

            file_exist = fopen(simple_filepath, "r");

            if (!file_exist)
            {
                printf("Please enter the correct filename.\n");
            }

        } while (!file_exist);

        fclose(file_exist);

        resume_download(resume_filepath, json_resume_filepath, simple_filepath);
    }
}