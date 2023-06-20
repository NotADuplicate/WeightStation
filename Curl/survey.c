#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "roster.h"
#include "survey.h"

void create_question(json_t *value, int question_count, surveyQuestion *questions) {
        json_t *id_json, *name_json, *question_json, *response_json;
        printf("Getting survey questions: %i ",question_count);
        id_json = json_object_get(value, "Id");
        name_json = json_object_get(value, "Name");
        question_json = json_object_get(value, "Question");
        response_json = json_object_get(value, "SurveyResponseTypeId");

        if (!json_is_string(name_json) || !json_is_integer(id_json)) {
            fprintf(stderr, "error: Id or Name is not of correct type\n");
			return;
        }
        questions[question_count].Id = json_integer_value(id_json);
        strncpy(questions[question_count].Name, json_string_value(name_json), sizeof(questions[question_count].Name) - 1);
        printf(" %s\n",questions[question_count].Name);    
        
		if(!json_is_integer(response_json)) {
            fprintf(stderr, "error: Responsetype is not of correct type\n");
			return;
        }
        else
			questions[question_count].responseType = json_integer_value(response_json);
        
        if(!json_is_string(question_json)) 
            printf("%s's question is not a string\n",questions[question_count].Name);
        else
            strncpy(questions[question_count].Question, json_string_value(question_json), sizeof(questions[question_count].Question) - 1);
            
}

surveyQuestion *process_survey_json(const char *json_string, int *count_ptr) {
  //printf("Processing json in roster.c: \n %s\n",json_string);
    json_error_t error;
    json_t *root;
    int question_count = 0;

    root = json_loads(json_string, 0, &error);
    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL; // we might want to handle this error differently
    }

    if (!json_is_array(root)) {
        fprintf(stderr, "error: root is not an array\n");
        json_decref(root);
        return NULL; // we might want to handle this error differently
    }

    size_t index;
    json_t *value;
    json_t *name_json;
    surveyQuestion *questions;
    questions = malloc(json_array_size(root) * sizeof(surveyQuestion));
    printf("Total questions: %i", json_array_size(root));
    json_array_foreach(root, index, value) {
        create_question(value,question_count,questions);
        question_count++;
    }

    json_decref(root);
    //free(json_string);
    *count_ptr = question_count; 
    return(questions);
}

surveyQuestion *get_survey(const char* baseUrl, int teamIndex, int *count_ptr) {
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
	const char *relative = "/OasisService/api/OASIS/SurveyQuestions";
    curl_easy_setopt(curl, CURLOPT_URL, concat(baseUrl,relative));

    // Add headers
    printf("Getting names with token from team %i\n",teamIndex);
    headers = curl_slist_append(headers, concat("Authorization: Bearer ",get_existing_token(teamIndex)));
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
  //printf("%s\n",json_copy);

  return process_survey_json(json_copy, count_ptr);
}
