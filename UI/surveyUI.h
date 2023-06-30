//SurveyUI.h
#ifndef SURVEYUI_H
#define SURVEYUI_H

#include "../Curl/roster.h"

typedef struct {
    int numQuestions;
    surveyQuestion *survey;
} SurveyDrawData;

void create_survey_window(surveyQuestion *survey, int numQuestions, int timer, GdkPixbuf *headshot, Player *player, char *weighMode);

#endif
