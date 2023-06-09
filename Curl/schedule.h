// Roster.h
#ifndef SCHEDULE_H
#define SCHEDULE_H

typedef struct {
    double utcStartTime;
    double utcEndTime;
    char name[16];
    int practiceType;
    int id;
} practiceStruct;

practiceStruct *get_next_practices(const char* baseUrl, int teamIndex);

double get_utc(const char *baseUrl);

char* seconds_to_readable_time(double input_seconds, char *result);

#endif
