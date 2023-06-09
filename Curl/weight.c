#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};


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

char *send_weight(const char *token, const char* baseUrl) {
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
	const char *relative = "/OasisService/api/OASIS/Weights";
	printf("%s%s\n",baseUrl,relative);
    curl_easy_setopt(curl, CURLOPT_URL, concat(baseUrl,relative));

    // Add headers
    headers = curl_slist_append(headers, concat("Authorization: Bearer ",token));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    //Add body
	char *playerid = curl_easy_escape(curl, "2208", 0);
	char *weightTypeId = curl_easy_escape(curl, "1", 0);
	char *value = curl_easy_escape(curl, "1", 0);

	char body[1024];
	snprintf(body, sizeof(body), "PlayerId=%s&WeightTypeId=%s&Value=%s", playerid,weightTypeId,value);
	printf("%s\n",body);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    
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
        return json_copy;
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

int main(int argc, char *argv[]) {
	const char *token = get_token("http://152.3.99.52","charlie","12112002a!");
	printf("%s\n",token);
	printf(send_weight(token,"http://152.3.99.52"));
	return 0;
}
