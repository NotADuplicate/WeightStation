#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "roster.h"
#include "survey.h"

void create_question_options(json_t *value, int question_count, surveyQuestion *question) {
        json_t *default_json, *order_json, *label_json;
        printf("Getting survey question options: %i ",question_count);
        default_json = json_object_get(value, "Default");
        order_json = json_object_get(value, "Order");
        label_json = json_object_get(value, "Label");

        if (!json_is_integer(order_json)) {
            printf("Survey option order is not an int\n");
			return;
        }
        int order = json_integer_value(order_json);
        
		if(!json_is_boolean(default_json)) {
            fprintf(stderr, "error: Default option is not of correct type: %i\n", json_integer_value(default_json));
			return;
        }
        else if(json_boolean_value(default_json) || order == 2) {
			question->defaultAnswer = order; //set to default answer
            printf("setting default\n\n\n\n\n");
        }
        
        if(!json_is_string(label_json)) {
            //printf("%s's the question option label is not a strinbg\n");
            strncpy(question->answerLabels[order], "", sizeof(question->answerLabels[order]) - 1);
        }
        else
            strncpy(question->answerLabels[order], json_string_value(label_json), sizeof(question->answerLabels[order]) - 1);
}

void process_survey_options_json(const char *json_string, surveyQuestion *question) {
  //printf("Processing json in roster.c: \n %s\n",json_string);
    json_error_t error;
    json_t *root;
    int question_count = 0;

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
    json_t *name_json;
    json_array_foreach(root, index, value) {
        create_question_options(value,question_count,question);
        question_count++;
    }

    json_decref(root);
    //free(json_string);
    //return(questions);
}

void get_survey_options(const char* baseUrl, int teamIndex, surveyQuestion *question) {
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
    char finalUrl[200];  // Allocate enough space for the final URL. You may need to adjust the size.

    // Use sprintf to format the final URL and convert Id to a string.
    const char *relative = "/OasisService/api/OASIS/SurveyQuestionOptions?SurveyQuestionId=";
    sprintf(finalUrl, "%s%s%d", baseUrl, relative, question->Id);
	
    curl_easy_setopt(curl, CURLOPT_URL, finalUrl);

    // Add headers
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
        return;
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

  process_survey_options_json(json_copy, question);
}

void create_question(json_t *value, int question_count, surveyQuestion *questions, const char *baseUrl, int teamIndex) {
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
        
        get_survey_options(baseUrl, teamIndex, &questions[question_count]);
}

surveyQuestion *process_survey_json(const char *json_string, int *count_ptr, const char *baseUrl, int teamIndex) {
  //printf("Processing json in survey.c: \n %s\n",json_string);
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
        create_question(value,question_count,questions, baseUrl, teamIndex);
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

  return process_survey_json(json_copy, count_ptr, baseUrl, teamIndex);
}
