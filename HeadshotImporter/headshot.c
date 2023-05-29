#include <stdio.h>
#include <curl/curl.h>
#include <gtk/gtk.h>

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    if (written < nmemb)
        fprintf(stderr, "Warning: not all data was written to file\n");
    return written;
}

void downscale_image(const char *filepath) {
	GdkPixbuf *pixbuf;
	GError *error = NULL;
    // Load and scale the image into a GdkPixbuf
    pixbuf = gdk_pixbuf_new_from_file_at_scale(filepath, -1,176, TRUE, &error);
    if (!pixbuf) {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return;
    }

    // Save the scaled image
    if (!gdk_pixbuf_save(pixbuf, filepath, "jpeg", &error, NULL)) {
        g_printerr("Error saving scaled image: %s\n", error->message);
        g_clear_error(&error);
        // Don't return; continue to draw the image even if saving failed
    }
    printf("Downscaling %s\n",filepath);
}

void download_image(const char *url, const char *output_path) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(output_path,"wb");
        if(!fp) {
            perror("Error opening output file. Check your output path.\n");
            return;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        // Enable redirect following
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    } else {
        fprintf(stderr, "Error initializing curl\n");
    }
    curl_global_cleanup();
    printf("Downloading %s \n",url);
    downscale_image(output_path);
}
