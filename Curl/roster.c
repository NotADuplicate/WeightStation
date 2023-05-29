#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "roster.h"
#include "../Helpers/playerHelper.h"

/*struct MemoryStruct {
  char *memory;
  size_t size;
};*/

char* concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) { //should be static
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

char* get_token(const char* baseUrl, const char *username, const char *password) {
  CURL *curl;
  CURLcode res;
  char *token_copy;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  
  chunk.size = 0;    

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, concat(baseUrl,"/OasisService/api/Accounts/Login?ApplicationName=Oasis&ApplicationVersion=1"));
    printf("Trying to login  username: %s    password %s\n",username,password);
    char *grant = concat(concat(concat("grant_type=password&username=",username),"&password="),password);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, grant);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    else {
      //printf("%lu bytes retrieved\n", (long)chunk.size);
      
      // JSON parsing begins here
      json_error_t error;
      json_t *root;
      root = json_loads(chunk.memory, 0, &error);
      
      if(!root){
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
      }

      json_t *token;
      const char *token_str;
      token = json_object_get(root, "access_token");
      if(!json_is_string(token)){
        fprintf(stderr, "error: token is not a string\n");
        json_decref(root);
        return NULL;
      }

      token_str = json_string_value(token);
      token_copy = strdup(token_str);
      
      // end of JSON parsing
      
      // cleanup JSON
      json_decref(root);
      curl_global_cleanup();
    }

    curl_easy_cleanup(curl);

    free(chunk.memory);
  }

  curl_global_cleanup();
  return(token_copy);
}

char *getNames(char *token, const char* baseUrl) {
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
    curl_easy_setopt(curl, CURLOPT_URL, concat(baseUrl,relative));

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

Player* process_json(const char *json_string, int *numPlayers_pointer) {
  printf("Processing json in roster.c\n");
    json_error_t error;
    json_t *root;
    int player_count = 0;

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
    Player *players;
    players = malloc(json_array_size(root) * sizeof(Player));
    printf("About to reference numPlayers_pointer\n");
    numPlayers_pointer[0] = json_array_size(root);
    printf("Total players: %i", json_array_size(root));
    json_array_foreach(root, index, value) {
      name_json = json_object_get(value, "DisplayName");
        //printf("Making player: %i\n",player_count);
        create_player(value,player_count,players);
        //players[player_count].HeadshotFilepath = concat(concat("Resources/",players[player_count].Name),".jpg");
        //download_image(headshotUrl,players[player_count].HeadshotFilepath);
        player_count++;
    }

    json_decref(root);
    return(players);
}


Player* getPlayers(const char *baseUrl, const char *username, const char *password, int *numPlayers_pointer) {
  //printf("Trying to get %i players\n",num);
  return(process_json(getNames(get_token(baseUrl,username,password),baseUrl),numPlayers_pointer));
}
