#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include "jsonReader.h"

char** get_team_codes(const char *json_file_path, int *numTeams) {
    json_t *root;
    json_error_t error;
    json_t *settings, *team_code;
    size_t i;

    // Load the JSON file
    root = json_load_file(json_file_path, 0, &error);

    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    // Get the "Settings" array
    settings = json_object_get(root, "Settings");

    if (!json_is_array(settings)) {
        fprintf(stderr, "error: Settings is not an array\n");
        json_decref(root);
        return NULL;
    }
    
    printf("Got settings.json array\n");

    // Get the number of teams
    size_t num_teams = json_array_size(settings);
    printf("Size: %i\n",num_teams);

    // Allocate memory for the array of team codes
    char **team_codes = malloc((num_teams + 1) * sizeof(char*));

    // For each object in the array, get the "Team code"
    for (i = 0; i < num_teams; i++) {
        json_t *data = json_array_get(settings, i);
        team_code = json_object_get(data, "TeamCode");
        team_codes[i] = strdup(json_string_value(team_code));
    }
    *numTeams = num_teams;
    
    // NULL terminate the array
    team_codes[num_teams] = NULL;

    // Clean up
    json_decref(root);

    return team_codes;
}

int get_num_teams(const char *json_file_path) {
    json_t *root;
    json_error_t error;
    json_t *settings, *team_code;
    size_t i;

    // Load the JSON file
    root = json_load_file(json_file_path, 0, &error);

    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 0;
    }

    // Get the "Settings" array
    settings = json_object_get(root, "Settings");

    if (!json_is_array(settings)) {
        fprintf(stderr, "error: Settings is not an array\n");
        json_decref(root);
        return 0;
    }
    
    printf("Got settings.json array\n");

    // Get the number of teams
    size_t num_teams = json_array_size(settings);
    printf("Size: %i\n",num_teams);
    return(num_teams);
}

void load_RGB(Settings *settings, json_t *settingsData, json_t *root) {
    char *rgb_component, *end, *temp_str;

    // Parse CompletedRGB
    json_t *completeRGB = json_object_get(settingsData, "CompletedRGB");
    if (!json_is_string(completeRGB)) {
        fprintf(stderr, "error: CompletedRGB is not a string\n");
        json_decref(root);
        return;
    }

    temp_str = strdup(json_string_value(completeRGB));
    for (int i = 0; i < 3; i++) {
        rgb_component = strtok_r(i == 0 ? temp_str : NULL, ",", &end);
        if (!rgb_component) {
            fprintf(stderr, "error: not enough CompletedRGB values\n");
            free(temp_str);
            json_decref(root);
            return;
        }
        settings->completedRGB[i] = atoi(rgb_component);
    }
    free(temp_str);

    // Parse NotCompletedRGB
    json_t *notCompleteRGB = json_object_get(settingsData, "NotCompletedRGB");
    if (!json_is_string(notCompleteRGB)) {
        fprintf(stderr, "error: NotCompletedRGB is not a string\n");
        json_decref(root);
        return;
    }

    temp_str = strdup(json_string_value(notCompleteRGB));
    for (int i = 0; i < 3; i++) {
        rgb_component = strtok_r(i == 0 ? temp_str : NULL, ",", &end);
        if (!rgb_component) {
            fprintf(stderr, "error: not enough NotCompletedRGB values\n");
            free(temp_str);
            json_decref(root);
            return;
        }
        settings->incompletedRGB[i] = atoi(rgb_component);
    }
    free(temp_str);

    // Parse InvalidRGB
    json_t *invalidRGB = json_object_get(settingsData, "InvalidRGB");
    if (!json_is_string(invalidRGB)) {
        fprintf(stderr, "error: InvalidRGB is not a string\n");
        json_decref(root);
        return;
    }

    temp_str = strdup(json_string_value(invalidRGB));
    for (int i = 0; i < 3; i++) {
        rgb_component = strtok_r(i == 0 ? temp_str : NULL, ",", &end);
        if (!rgb_component) {
            fprintf(stderr, "error: not enough InvalidRGB values\n");
            free(temp_str);
            json_decref(root);
            return;
        }
        settings->invalidRGB[i] = atoi(rgb_component);
    }
    free(temp_str);
}

Settings* load_settings(const char* filename, int teamIndex) {
    printf("LOADING SETTINGS\n\n\nLOADING\n");
    json_t *root;
    json_error_t error;
    root = json_load_file(filename, 0, &error);

    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    json_t *settingsArray = json_object_get(root, "Settings");
    if (!json_is_array(settingsArray)) {
        fprintf(stderr, "error: settingsArray is not an array\n");
        json_decref(root);
        return NULL;
    }

    json_t *settingsData = json_array_get(settingsArray, teamIndex); // Get the specific team settings
    if (!json_is_object(settingsData)) {
        fprintf(stderr, "error: settingsData is not an object\n");
        json_decref(root);
        return NULL;
    }

    Settings *settings = malloc(sizeof(Settings));

    json_t *baseUrl = json_object_get(settingsData, "BaseUrl");
    if (!json_is_string(baseUrl)) {
        fprintf(stderr, "error: baseUrl is not a string\n");
        json_decref(root);
        return NULL;
    }
    settings->baseUrl = strdup(json_string_value(baseUrl));

    json_t *username = json_object_get(settingsData, "username");
    if (!json_is_string(username)) {
        fprintf(stderr, "error: username is not a string\n");
        json_decref(root);
        return NULL;
    }
    settings->username = strdup(json_string_value(username));

    json_t *password = json_object_get(settingsData, "password");
    if (!json_is_string(password)) {
        fprintf(stderr, "error: password is not a string\n");
        json_decref(root);
        return NULL;
    }
    settings->password = strdup(json_string_value(password));

    json_t *ip = json_object_get(root, "ServerIp");
    if (!json_is_string(ip)) {
        fprintf(stderr, "error: ip is not a string\n");
        json_decref(root);
        return NULL;
    }
    settings->ip = strdup(json_string_value(ip));
    
    json_t *port = json_object_get(root, "ServerPort");
    if (!json_is_string(port)) {
        fprintf(stderr, "error: port is not a string\n");
        json_decref(root);
        return NULL;
    }
    settings->port = strdup(json_string_value(port));
    
    json_t *playerPerPage = json_object_get(settingsData, "PlayersPerScreen");
    if (!json_is_string(playerPerPage)) {
        printf("%s\n",json_string_value(playerPerPage));
        fprintf(stderr, "error: PlayersPerScreen is not an integer\n");
        json_decref(root);
        return NULL;
    }
    settings->playerPerPage = atoi(json_string_value(playerPerPage));
    
    json_t *survey = json_object_get(settingsData, "Survey");
    printf("%s\n",json_string_value(survey));
    if (!json_is_string(survey)) {
        printf("%s\n",json_string_value(playerPerPage));
        fprintf(stderr, "error: Survey is not an integer\n");
        json_decref(root);
        return NULL;
    }
    if(json_string_value(survey)[0] == 'T') //jank way to check if true
        settings->survey = 1;
    else
        settings->survey = 0;
        
    json_t *server = json_object_get(root, "ServerMode");
    if (!json_is_string(server)) {
        printf("%s\n",json_string_value(server));
        fprintf(stderr, "error: Server is not a string\n");
        json_decref(root);
        return NULL;
    }
    if(json_string_value(server)[0] == 'S') //jank way to check if server
        settings->server = 1;
    else
        settings->server = 0;
    
    json_t *weighIn = json_object_get(settingsData, "WeighInOffset");
    if (!json_is_string(weighIn)) {
        printf("%s\n",json_string_value(weighIn));
        fprintf(stderr, "error: WeighInOffset is not an integer\n");
        json_decref(root);
        return NULL;
    }
    settings->weighInOffset = atoi(json_string_value(weighIn));
    
    json_t *weighOut = json_object_get(settingsData, "WeighOutOffset");
    if (!json_is_string(weighOut)) {
        printf("%s\n",json_string_value(weighOut));
        fprintf(stderr, "error: WeighOutOffset is not an integer\n");
        json_decref(root);
        return NULL;
    }
    settings->weighOutOffset = atoi(json_string_value(weighIn));
    
    json_t *numberOfScreens = json_object_get(settingsData, "NumberOfPages");
    if (!json_is_string(numberOfScreens)) {
        fprintf(stderr, "error: NumberOfPages is not an integer\n");
        json_decref(root);
        return NULL;
    }
    settings->numPages = atoi(json_string_value(numberOfScreens));
    
    load_RGB(settings,settingsData,root);

    json_decref(root);
    
    printf("Done with settings\n");

    return settings;
}
