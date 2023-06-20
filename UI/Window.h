//Window.h
#ifndef WINDOW_H
#define WINDOW_H

enum Mode {
    WAITING,
    WEIGH_IN,
    WEIGH_OUT
};

enum View {
    MAIN_WINDOW = 0,
    DMX_WINDOW = 1, 
    WEIGH_WINDOW = 2,
    SURVEY_WINDOW = 3
};

typedef struct {
    double x;
    double y;
    double radius;
} circleStruct;

typedef struct {
    int playerIndex;
    GtkWidget *entry;
    GtkWidget *window;
} SubmitData;


typedef struct {
    Player *teamPlayers;
    circleStruct *teamCircles;
    Settings *teamSettings;
    GdkPixbuf **teamHeadshots;
    int *teamSorted;
    char *token;
    practiceStruct *teamPractices; 
    surveyQuestion *survey;
    int surveyNumQuestions;
} teamStruct;

void create_main_window();

void cleanup();

void load_globals(int teamNum);

void set_mode(enum Mode setmode, int manualOverride);

void submit_weight(SubmitData *data);

#endif
