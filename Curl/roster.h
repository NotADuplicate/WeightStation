// Roster.h
#ifndef ROSTER_H
#define ROSTER_H

typedef struct {
    int Id;
    char Name[16];  // or whatever the maximum name length could be
    char HeadshotUrl[100];
    char HeadshotFilepath[50];
    char UniformNumber[4];
} Player;

struct MemoryStruct {
  char *memory;
  size_t size;
};

Player* getPlayers(const char *baseUrl, const char *username, const char *password, int *numPlayers_pointer);

char* concat(const char *s1, const char *s2);

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

char* get_token(const char* baseUrl, const char *username, const char *password);

#endif /* CURL_H */
