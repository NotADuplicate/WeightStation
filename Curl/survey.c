char *get_survey(const char* baseUrl, int teamIndex) {
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
  printf("%s\n",json_copy);

  return json_copy;
}

surveyQuestion *process_json(const char *json_string, int *numPlayers_pointer) {
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
    surveyQuestion questions;
    questions = malloc(json_array_size(root) * sizeof(surveyQuestion));
    printf("Total questions: %i", json_array_size(root));
    json_array_foreach(root, index, value) {
        create_question(value,player_count,players);
        question_count++;
    }

    json_decref(root);
    return(questions);
}
