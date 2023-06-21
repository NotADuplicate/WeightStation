//WeighUI.h
#ifndef WEIGHUI_H
#define WEIGHUI_IN

#include "../Curl/roster.h"

typedef struct {
    int playerIndex;
    GtkWidget *window;
    GString *typed_text;
} SubmitData;

typedef struct {
    Player *player;
    GdkPixbuf *image;
} DrawData;

void create_weigh_input(GdkPixbuf *pixbuf, Player *player, int playerIndex, void (*submit_func)(SubmitData*));

#endif
