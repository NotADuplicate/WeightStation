//Window.h
#ifndef WINDOW_H
#define WINDOW_H

enum Mode {
    WAITING,
    WEIGH_IN,
    WEIGH_OUT
};

enum View {
    MAIN_WINDOW,
    DMX_WINDOW
};

typedef struct {
    double x;
    double y;
    double radius;
} circleStruct;

typedef struct {
    Player *teamPlayers;
    circleStruct *teamCircles;
    Settings *teamSettings;
    GdkPixbuf **teamHeadshots;
    int *teamSorted;
    char *token;
    practiceStruct *teamPractices; 
} teamStruct;

void create_main_window();

void cleanup();

void load_globals(int teamNum);

void set_mode(enum Mode setmode, int manualOverride);

#endif
