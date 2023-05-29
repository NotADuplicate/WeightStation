#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include "jsonReader.h"

#include <jansson.h>

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

Settings* load_settings(const char* filename, int teamIndex) {
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

    json_t *settingsData = json_array_get(settingsArray, teamIndex); // Get the first object in the array
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

    /*json_t *NumPlayers = json_object_get(settingsData, "NumPlayers");
    if (!json_is_integer(NumPlayers)) {
        fprintf(stderr, "error: NumPlayers is not an integer\n");
        json_decref(root);
        return NULL;
    }*/
    
    json_t *playerPerPage = json_object_get(settingsData, "PlayersPerScreen");
    if (!json_is_string(playerPerPage)) {
        printf("%s\n",json_string_value(playerPerPage));
        fprintf(stderr, "error: PlayersPerScreen is not an integer\n");
        json_decref(root);
        return NULL;
    }
    settings->playerPerPage = atoi(json_string_value(playerPerPage));
    
    json_t *numberOfScreens = json_object_get(settingsData, "NumberOfPages");
    if (!json_is_string(numberOfScreens)) {
        fprintf(stderr, "error: NumberOfPages is not an integer\n");
        json_decref(root);
        return NULL;
    }
    settings->numPages = atoi(json_string_value(playerPerPage));

    json_decref(root);

    return settings;
}
