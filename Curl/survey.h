// Survey.h
#ifndef SURVEY_H
#define SURVEY_H

typedef struct {
    int Id;
    char Name[16];  // or whatever the maximum name length could be
    char Question[100];
    int responseType; //1 is true or false, 2 is yes/no, 3 is 1-3, 4 is 1-5
} surveyQuestion;

surveyQuestion *get_survey(const char* baseUrl, int teamIndex, int *count_ptr);

#endif
