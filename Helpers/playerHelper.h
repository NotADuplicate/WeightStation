// Player.h
#ifndef PLAYER_H
#define PLAYER_H
#include <jansson.h>

typedef struct {
    int index;
    Player* p;
} indexed_player;

void create_player(json_t *value, int player_count, Player *players);
int* sort_players(Player* players, int num_players);

#endif /* PLAYER_H */
