// Roster.h
#ifndef ROSTER_H
#define ROSTER_H

typedef struct {
    int Id;
    char Name[16];  // or whatever the maximum name length could be
    char HeadshotUrl[100];
    char HeadshotFilepath[50];
    char UniformNumber[4];
    float weight;
    int weighed; //0 is false, 1 is true
} Player;

struct MemoryStruct {
  char *memory;
  size_t size;
};

Player* getPlayers(const char *baseUrl, const char *username, const char *password, int *numPlayers_pointer, int teamIndex, int *connected_ptr);

char* concat(const char *s1, const char *s2);

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

char* get_token(const char* baseUrl, const char *username, const char *password, int teamIndex);

char *send_weight(const char* baseUrl, const char* weight, int playerId, int weightTypeId, double time, int teamIndex);

char* get_existing_token(int teamIndex);

void resend_weights(const char *url);
#endif /* CURL_H */
