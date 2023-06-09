#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "roster.h"
#include "schedule.h"
#include "../Helpers/playerHelper.h"
#include "../Helpers/jsonReader.h"

#define THREE_DAYS 259200
#define SECONDS_IN_MINUTE 60
#define MINUTES_IN_HOUR   60
#define HOURS_IN_DAY      24

practiceStruct *practiceArray;
int numPractices;

char* seconds_to_readable_time(double input_seconds, char *result) {
    int seconds = (int)input_seconds;

    int days = seconds / (HOURS_IN_DAY * MINUTES_IN_HOUR * SECONDS_IN_MINUTE);
    seconds %= (HOURS_IN_DAY * MINUTES_IN_HOUR * SECONDS_IN_MINUTE);

    int hours = seconds / (MINUTES_IN_HOUR * SECONDS_IN_MINUTE);
    seconds %= (MINUTES_IN_HOUR * SECONDS_IN_MINUTE);

    int minutes = seconds / SECONDS_IN_MINUTE;
    seconds %= SECONDS_IN_MINUTE;

    result[0] = '\0'; // Initialize to an empty string

    if (days > 0)
        sprintf(result, "%d days ", days);
    if (hours > 0)
        sprintf(result + strlen(result), "%d hours ", hours);
    if (minutes > 0)
        sprintf(result + strlen(result), "%d minutes ", minutes);
    if (seconds > 0 && (days == 0 && hours == 0 && minutes == 0)) // Show seconds if they are the only non-zero component
        sprintf(result + strlen(result), "%d seconds", seconds);

    return result;
}

void *json_to_practices(json_t* json_array, size_t* out_size) {
    size_t size = json_array_size(json_array);
    practiceArray = malloc(size * sizeof(practiceStruct));
    
    if (practiceArray == NULL) {
        /* Allocation failed, return NULL */
        *out_size = 0;
        printf("Allocation failed in json_to_practices\n");
        return NULL;
    }

    for (size_t i = 0; i < size; ++i) {
        json_t* item = json_array_get(json_array, i);

        /* Extract the values and put them into the struct */
        strncpy(practiceArray[i].name,json_string_value(json_object_get(item, "Name")),sizeof(practiceArray[i].name));
        practiceArray[i].practiceType = json_integer_value(json_object_get(item, "PracticeTypeId"));
        //practices[i].name = strdup(json_string_value(json_object_get(item, "Name")));
        practiceArray[i].utcStartTime = json_real_value(json_object_get(item, "ScheduledStartUtc"));
        practiceArray[i].utcEndTime = json_real_value(json_object_get(item, "ScheduledEndUtc"));
        practiceArray[i].id = json_integer_value(json_object_get(item, "Id"));
        printf("Practice: %s\n",practiceArray[i].name);
    }

    *out_size = size;
}

double get_utc(const char *baseUrl) {
  CURL *curl;
  CURLcode res;
  double time;
  char *endPointer;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  
  chunk.size = 0;    

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    const char *relative = "/OasisService/api/OASIS/Utc";
      curl_easy_setopt(curl, CURLOPT_URL, concat(baseUrl,relative));
      
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

      res = curl_easy_perform(curl);
      if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
      else {
        printf("%lu bytes retrieved\n", (long)chunk.size);
        
        // JSON parsing begins here
        time = strtod(chunk.memory,&endPointer);
        free(chunk.memory);
    }
  }
        curl_global_cleanup();
        return time;
}

int compare_start_utc(const void *a, const void *b) {
    json_t *practice1 = *(json_t **)a;
    json_t *practice2 = *(json_t **)b;
    double utc1 = json_real_value(json_object_get(practice1, "ScheduledStartUtc"));
    double utc2 = json_real_value(json_object_get(practice2, "ScheduledStartUtc"));

    return (utc1 > utc2) - (utc1 < utc2);
}

json_t *sort_practice_array(json_t *practices_json) {
    // Convert JSON array to C array
    size_t index;
    json_t *practice, **array;
    size_t array_size = json_array_size(practices_json);

    array = malloc(array_size * sizeof(json_t *));
    json_array_foreach(practices_json, index, practice) {
        array[index] = practice;
    }

    // Sort array using qsort
    qsort(array, array_size, sizeof(json_t *), compare_start_utc);

    // Create a new JSON array and add the sorted practices to it
    json_t *sorted_practices = json_array();
    for (index = 0; index < array_size; index++) {
        json_array_append(sorted_practices, array[index]);
    }

    free(array);
    
    return sorted_practices;
}

json_t *filter_practices(json_t *practices_json, double utc_threshold) {
    size_t index;
    json_t *value, *scheduledStartUtc;

    json_t *filtered_practices = json_array();

    json_array_foreach(practices_json, index, value) {
        scheduledStartUtc = json_object_get(value, "ScheduledStartUtc");

        if(json_is_real(scheduledStartUtc)) {
            double utc_time = json_real_value(scheduledStartUtc);

            if(utc_time >= utc_threshold && utc_time <= utc_threshold + THREE_DAYS) {
                json_array_append(filtered_practices, value);
            }
        }
    }
    //create_practice_window(filtered_practices);

    return filtered_practices;
}

practiceStruct *get_next_practices(const char* baseUrl, int teamIndex) {
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
	const char *relative = "/OasisService/api/OASIS/Practices";
    curl_easy_setopt(curl, CURLOPT_URL, concat(baseUrl,relative));
    char *token = get_existing_token(teamIndex);
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
      
      if(!root){
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
      }
      //Actually handle the practices and put them into an array
      json_t *filtered = filter_practices(root,get_utc(baseUrl));
      json_t *sorted_filtered = sort_practice_array(filtered);
      json_to_practices(sorted_filtered,&numPractices);
      
      json_decref(root);
      json_decref(sorted_filtered);
      json_decref(filtered);
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers); // free the list of headers
    json_copy = strdup(chunk.memory);

    free(chunk.memory);
  }
  

  curl_global_cleanup();
  
  printf("Got %d practices\n",numPractices);
  
  return practiceArray;
}
