#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "cams.h"
#include "roster.h"

struct MemoryStruct {
  char *memory;
  size_t size;
};

const char *token_str;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int set_token(const char* baseUrl, const char *username, const char *password, const char *applicationName) { //get token from CAMS
  CURL *curl;
  CURLcode res;
  char *token_copy;
  struct curl_slist *headers = NULL;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  
  chunk.size = 0;    

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, baseUrl);
    printf("%s\n",baseUrl);
    char *grant = concat(concat(concat("grant_type=password&username=",username),"&password="),password);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, grant);
    headers = curl_slist_append(headers, concat("ApplicationName: ",applicationName));
    headers = curl_slist_append(headers,"ApplicationVersion: 3");

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
        return 0; // failure
    }
    else {
      //printf("%lu bytes retrieved\n", (long)chunk.size);
      
      // JSON parsing begins here
      json_error_t error;
      json_t *root;
      root = json_loads(chunk.memory, 0, &error);
      
      if(!root){
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 0; //failure
      }

      json_t *token;
      token = json_object_get(root, "access_token");
      if(!json_is_string(token)){
        fprintf(stderr, "error logging in, could not get access token\n");
        json_decref(root);
        return 0; //failure
      }
        printf("%s\n",json_string_value(token));
      token_str = strdup(json_string_value(token));
      printf("%s\n",token_str);
      
      // end of JSON parsing
      
      // cleanup JSON
      json_decref(root);
      curl_global_cleanup();
    }

    curl_easy_cleanup(curl);

    free(chunk.memory);
  }

  curl_global_cleanup();
  printf("Successfully got token");
  return 1; //success
}

char *get_settings(const char* baseUrl) {
  CURL *curl;
  CURLcode res;
  char *json_copy;
  struct curl_slist *headers = NULL;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  
  chunk.size = 0;    

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  printf("Trying to get settings\n");
  if(curl) {
    printf("Got settings\n");
    curl_easy_setopt(curl, CURLOPT_URL, baseUrl);
    printf("Made it here1\n");

    // Add headers
    headers = curl_slist_append(headers, concat("Authorization: Bearer ",token_str));
    printf("Made it here1\n");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    printf("Made it here1\n");
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    printf("Made it here2\n");
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    else {
      printf("%lu bytes retrieved\n", (long)chunk.size);
      printf("Made it here3\n");
      // JSON parsing begins here
      json_error_t error;
      json_t *root;
      root = json_loads(chunk.memory, 0, &error);
      json_copy = strdup(chunk.memory);
      printf("Made it here4\n");
      
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
  
  printf("Successfully got settings\n");

  return json_copy;
}

void append_to_settings(json_t* new_setting) {
    // Parse the new setting from a JSON string
    json_error_t error;
    //json_t *new_setting = json_loads(new_setting_str, 0, &error);
    if (!new_setting) {
        printf("Error: could not parse new setting JSON\n");
        return;
    }

    // Load the existing JSON data from the file
    json_t *root = json_load_file("settings.json", 0, &error);
    if (!root) {
        printf("Error: could not load JSON from settings.json\n");
        json_decref(new_setting);
        return;
    }

    // Get the Settings array
    json_t *settings_array = json_object_get(root, "Settings");
    if (!json_is_array(settings_array)) {
        printf("Error: Settings is not an array\n");
        json_decref(root);
        json_decref(new_setting);
        return;
    }

    // Append the new setting to the Settings array
    json_array_append(settings_array, new_setting);
    size_t length = json_array_size(settings_array);
    printf("The array has %zu elements\n", length);

    // Write the entire JSON object back to the file
    FILE *file = fopen("settings.json", "w");
    if (file != NULL) {
        char *new_json_str = json_dumps(root, JSON_INDENT(4));
        fputs(new_json_str, file);
        fclose(file);
        free(new_json_str);
    }

    // Free the JSON objects
    json_decref(root);
    json_decref(new_setting);
}

void set_settings(const char *json_str, const char *username, const char *password, int append) {
    // Parse JSON string
    json_error_t error;
    json_t* root = json_loads(json_str, 0, &error);

    // Extract Settings array
    json_t* settings_array = json_object_get(root, "Settings");
    if(!json_is_array(settings_array)) {
        printf("Error: Settings is not an array\n");
    }

    // Create a new JSON object for the settings
    json_t* new_json = json_object();

    // Iterate over Settings array
    size_t i;
    json_t* value;
    json_array_foreach(settings_array, i, value) {
        // Extract Name and Value
        const char* name = json_string_value(json_object_get(value, "Name"));
        const char* val = json_string_value(json_object_get(value, "Value"));

        // Add Name:Value pair to the new JSON object
        json_object_set_new(new_json, name, json_string(val));
    }

    json_object_set_new(new_json, "username", json_string(username));
    json_object_set_new(new_json, "password", json_string(password));

    // Create the Settings array and add the new JSON object to it
    json_t* new_settings_array = json_array();
    json_array_append_new(new_settings_array, new_json);
    
    //Append if appending
    if(append == 1) {
      printf("adding new team\n");
        append_to_settings(new_json);
        return;
    }

    // Create the new root object and add the Settings array to it
    json_t* new_root = json_object();
    json_object_set_new(new_root, "Settings", new_settings_array);

    // Convert the new root object to a string
    char* new_json_str = json_dumps(new_root, JSON_INDENT(4));
    printf("%s\n", new_json_str);
    
    // Write to settings.json
    FILE* file = fopen("settings.json", "w");
    if (file != NULL) {
        fputs(new_json_str, file);
        fclose(file);
    }

    // Free memory
    json_decref(root);
    json_decref(new_root);
    free(new_json_str);
}

void set_json(const char *username, const char *password, int appending) {
  const char *settings = get_settings("https://cams.smt.com/CAMS/api/WeightStationConfiguration");
  set_settings(settings,username,password,appending);
  printf("Successfully set settings\n");
}

void add_team(const char *username, const char *password) {
  const char *settings = get_settings("https://cams.smt.com/CAMS/api/WeightStationConfiguration");
  set_settings(settings,username,password,1);
  printf("Successfully set new team settings\n");
}

int cams_login(const char *camsUsername, const char *camsPassword) {
  const char *baseUrl = "https://cams.smt.com/CAMS/api/accounts/Login";
  return(set_token(baseUrl,camsUsername,camsPassword,"OasisWeightStation"));
}
