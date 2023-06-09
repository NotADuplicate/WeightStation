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
    double weighInOffset;
    double weighOutOffset;
    int completedRGB[3];
    int incompletedRGB[3];
    int invalidRGB[3];
} Settings;

Settings* load_settings(const char* filename, int teamIndex);
char** get_team_codes(const char *json_file_path, int *numTeams);
int get_num_teams(const char *json_file_path);

#endif 