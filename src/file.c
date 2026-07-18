#include "file.h"
#include <stdio.h>

int create_file(const char *filename)
{
    FILE *f = fopen(filename, "wb");

    if (!f)
    {
        printf("Failed to create file\n");
        return 0;
    }

    printf("File created: %s\n", filename);

    fclose(f);
    return 1;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}