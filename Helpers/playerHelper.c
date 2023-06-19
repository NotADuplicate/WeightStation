#include <stdlib.h>
#include <jansson.h>
#include <string.h>
#include "../Curl/roster.h"
#include "playerHelper.h"

int compare(const void* a, const void* b) {
    indexed_player* playerA = (indexed_player*)a;
    indexed_player* playerB = (indexed_player*)b;
    return (atoi(playerA->p->UniformNumber) - atoi(playerB->p->UniformNumber));
}

int* sort_players(Player* players, int num_players) {
    indexed_player* indexed_players = (indexed_player*)malloc(num_players * sizeof(indexed_player));
    for (int i = 0; i < num_players; i++) {
        indexed_players[i].index = i;
        indexed_players[i].p = &players[i];
    }

    qsort(indexed_players, num_players, sizeof(indexed_player), compare);

    int* sorted_indexes = (int*)malloc(num_players * sizeof(int));
    for (int i = 0; i < num_players; i++) {
        sorted_indexes[i] = indexed_players[i].index;
    }

    free(indexed_players);
    return sorted_indexes;
}

void create_player(json_t *value, int player_count, Player *players) {
        json_t *id_json, *name_json, *headshot_json, *uniform_json;
        printf("Getting players json: %i ",player_count);
        id_json = json_object_get(value, "Id");
        name_json = json_object_get(value, "DisplayName");
        headshot_json = json_object_get(value, "Headshot");
        uniform_json = json_object_get(value, "UniformNumber");

        if (!json_is_string(name_json) || !json_is_integer(id_json)) {
            fprintf(stderr, "error: Id or Name is not of correct type\n");
			return;
        }
        players[player_count].Id = json_integer_value(id_json);
        strncpy(players[player_count].Name, json_string_value(name_json), sizeof(players[player_count].Name) - 1);
        printf(" %s\n",players[player_count].Name);
        
        if (!json_is_string(headshot_json)) {
            printf("%s's headshot Url is not a string\n",players[player_count].Name);
        }
        else {
            strncpy(players[player_count].HeadshotUrl, json_string_value(headshot_json), sizeof(players[player_count].HeadshotUrl) - 1);
            strncpy(players[player_count].HeadshotFilepath, concat(concat("Headshots/",players[player_count].Name),".jpg"), sizeof(players[player_count].HeadshotFilepath) - 1);
        }
            
        
        if(!json_is_string(uniform_json)) 
            printf("%s's uniform is not a string\n",players[player_count].Name);
        else
            strncpy(players[player_count].UniformNumber, json_string_value(uniform_json), sizeof(players[player_count].UniformNumber) - 1);
            
        players[player_count].weight = 0;
        players[player_count].weighed = 0;
}
