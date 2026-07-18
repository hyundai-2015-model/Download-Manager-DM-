#include "http.h"
#include "utils.h"
#include "file.h"

#include <stdio.h>
#include <curl/curl.h>
#include <time.h>

typedef struct
{
    CURL *curl;
    curl_off_t initial_downloaded;
} ProgressData;

static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    ProgressData *pdata = (ProgressData *)clientp;

    curl_off_t downloaded = pdata->initial_downloaded + dlnow;
    curl_off_t total = pdata->initial_downloaded + dltotal;

    (void)ultotal;
    (void)ulnow;

    if (total <= 0)
        return 0;

    static clock_t last_update = 0;
    clock_t now = clock();

    if ((double)(now - last_update) / CLOCKS_PER_SEC < 0.25 &&
        downloaded < total)
    {
        return 0;
    }

    last_update = now;

    double percent = (double)downloaded / (double)total;

    double elapsed = 0.0;
    curl_easy_getinfo(pdata->curl, CURLINFO_TOTAL_TIME, &elapsed);

    double speed = 0.0;

    if (elapsed > 0.0)
        speed = dlnow / elapsed;

    static double display_speed = 0.0;

    if (display_speed == 0.0)
        display_speed = speed;
    else
        display_speed = display_speed * 0.8 + speed * 0.2;

    speed = display_speed;

    double eta = 0.0;

    if (speed > 0.0)
        eta = (total - downloaded) / speed;

    char speed_str[32];

    if (speed >= 1024.0 * 1024.0)
        snprintf(
            speed_str,
            sizeof(speed_str),
            "%.1f MB/s",
            speed / (1024.0 * 1024.0)
        );
    else if (speed >= 1024.0)
        snprintf(
            speed_str,
            sizeof(speed_str),
            "%.1f KB/s",
            speed / 1024.0
        );
    else
        snprintf(
            speed_str,
            sizeof(speed_str),
            "%.0f B/s",
            speed
        );

    char down_str[32];
    char total_str[32];

        if (downloaded >= 1024.0 * 1024.0 * 1024.0)
        snprintf(
            down_str,
            sizeof(down_str),
            "%.2f GB",
            downloaded / (1024.0 * 1024.0 * 1024.0)
        );
    else if (downloaded >= 1024.0 * 1024.0)
        snprintf(
            down_str,
            sizeof(down_str),
            "%.2f MB",
            downloaded / (1024.0 * 1024.0)
        );
    else if (downloaded >= 1024.0)
        snprintf(
            down_str,
            sizeof(down_str),
            "%.1f KB",
            downloaded / 1024.0
        );
    else
        snprintf(
            down_str,
            sizeof(down_str),
            "%.0f B",
            (double)downloaded
        );

    if (total >= 1024.0 * 1024.0 * 1024.0)
        snprintf(
            total_str,
            sizeof(total_str),
            "%.2f GB",
            total / (1024.0 * 1024.0 * 1024.0)
        );
    else if (total >= 1024.0 * 1024.0)
        snprintf(
            total_str,
            sizeof(total_str),
            "%.2f MB",
            total / (1024.0 * 1024.0)
        );
    else if (total >= 1024.0)
        snprintf(
            total_str,
            sizeof(total_str),
            "%.1f KB",
            total / 1024.0
        );
    else
        snprintf(
            total_str,
            sizeof(total_str),
            "%.0f B",
            (double)total
        );

    int eta_min = (int)eta / 60;
    int eta_sec = (int)eta % 60;

    const int width = 30;
    int pos = (int)(percent * width + 0.5);

    if (pos > width)
        pos = width;

    printf("\r");
    printf(" ");

    for (int i = 0; i < width; i++)
    {
        printf(
            "%s━",
            (i < pos)
                ? "\033[38;5;45m"
                : "\033[38;5;238m"
        );
    }

    printf("\033[0m");

    if (percent < 1.0)
        printf(
            " \033[1m%3.0f%%\033[22m ""\033[38;5;246m[%s %s/%s ETA %02d:%02d]\033[0m", percent * 100.0, speed_str, down_str, total_str, eta_min, eta_sec);
    else
        printf(
            " \033[1m100%%\033[22m ""\033[38;5;246m[%s %s/%s]\033[0m", speed_str, down_str, total_str);

    fflush(stdout);
    return 0;
}

int http_download(const char *url, FILE *fp, const char *json_file, const char *filename, curl_off_t initial_downloaded, int resume)
{
    CURL *curl = curl_easy_init();

    if (curl)
    {
        ProgressData pdata;

        pdata.curl = curl;
        pdata.initial_downloaded = initial_downloaded;

        FILE *json = fopen(json_file, "w");
        if (json)
        {
            fprintf(json, "{\n""    \"filename\": \"%s\",\n""    \"url\": \"%s\"\n""}\n",filename,url);
            fclose(json);
        };

        curl_easy_setopt(curl, CURLOPT_URL, url);
        if (resume)
        {
            char range[64];

            snprintf(range, sizeof(range), "%lld-", (long long)initial_downloaded);

            curl_easy_setopt(curl, CURLOPT_RANGE, range);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &pdata);

        log_msg("Starting...");

        CURLcode res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            return 0;
        }

        printf("\n");
        log_msg("Download completed");

        return 1;
    }
    else
    {
        return 0;
    }
}