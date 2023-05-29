// jsonReader.h
#ifndef JSONREADER_H
#define JSONREADER_H

typedef struct Settings {
    char* baseUrl;
    char* username;
    char* password;
    int NumPlayers;
    int playerPerPage;
    int numPages;
} Settings;

Settings* load_settings(const char* filename, int teamIndex);
char** get_team_codes(const char *json_file_path, int *numTeams);

#endif 
