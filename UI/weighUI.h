//WeighUI.h
#ifndef WEIGHUI_H
#define WEIGHUI_IN

typedef struct {
    int playerIndex;
    GtkWidget *entry;
} SubmitData;

void create_weigh_input(GdkPixbuf *pixbuf, const char *text, int playerIndex, void (*submit_func)(const char *));

#endif
