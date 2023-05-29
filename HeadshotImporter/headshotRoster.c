#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "headshot.h"
#include "../Helpers/jsonReader.h"
#include "../Curl/roster.h"

Settings* headshotSettings;

char *getJson(char *token, char* baseURL) {
  CURL *curl;
  CURLcode res;
  char *json_copy;
  struct curl_slist *headers = NULL;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  
  chunk.size = 0;    

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
	const char *relative = "/OasisService/api/OASIS/Players";
    curl_easy_setopt(curl, CURLOPT_URL, concat(baseURL,relative));

    // Add headers
    headers = curl_slist_append(headers, concat("Authorization: Bearer ",token));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    else {
      printf("%lu bytes retrieved\n", (long)chunk.size);
      
      // JSON parsing begins here
      json_error_t error;
      json_t *root;
      root = json_loads(chunk.memory, 0, &error);
      json_copy = strdup(chunk.memory);
      
      if(!root){
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
      }

      // TODO: Add JSON parsing logic here.

      // cleanup JSON
      json_decref(root);
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers); // free the list of headers

    free(chunk.memory);
  }

  curl_global_cleanup();

  return json_copy;
}

void process_json_for_headshots(const char *json_string) {
    json_error_t error;
    json_t *root;
    int player_count = 0;

    root = json_loads(json_string, 0, &error);
    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return; // we might want to handle this error differently
    }

    if (!json_is_array(root)) {
        fprintf(stderr, "error: root is not an array\n");
        json_decref(root);
        return; // we might want to handle this error differently
    }

    size_t index;
    json_t *value;
    json_array_foreach(root, index, value) {
        json_t *id_json, *name_json, *headshot_json;

        id_json = json_object_get(value, "Id");
        name_json = json_object_get(value, "DisplayName");
        headshot_json = json_object_get(value, "Headshot");
        const char *headshotUrl = json_string_value(headshot_json);

        if (!json_is_string(name_json) || !json_is_integer(id_json)) {
            fprintf(stderr, "error: Id or Name is not of correct type\n");
            continue;
        }

        const char *headshotFilepath = concat(concat("../WeightStation/Headshots/",json_string_value(name_json)),".jpg");
        download_image(json_string_value(headshot_json),headshotFilepath);

        player_count++;
    }
    json_decref(root);
}

void getHeadshots(int teamIndex) {
  headshotSettings = load_settings("../WeightStation/settings.json",teamIndex);
  process_json_for_headshots(getJson(get_token(headshotSettings->baseUrl, headshotSettings->username, headshotSettings->password),headshotSettings->baseUrl));
  free(headshotSettings->baseUrl);
  free(headshotSettings->password);
  free(headshotSettings->username);
  free(headshotSettings);
}
