#pragma once

#include <time.h>

typedef enum {
	SCHEDULE_SUN = 0,
	SCHEDULE_MON,
	SCHEDULE_TUE,
	SCHEDULE_WED,
	SCHEDULE_THU,
	SCHEDULE_FRI,
	SCHEDULE_SAT,
} schedule_weekday_t;

typedef enum {
	SCHEDULE_POS_ANY  = 0,
	SCHEDULE_POS_1    = 1,
	SCHEDULE_POS_2    = 2,
	SCHEDULE_POS_3    = 3,
	SCHEDULE_POS_4    = 4,
	SCHEDULE_POS_LAST = 5,
} schedule_pos_t;

typedef struct {
	uint8_t hour;
	uint8_t minute;
} schedule_clock_t;

typedef struct {
	schedule_clock_t start;
	schedule_clock_t end;
	uint32_t period;
} schedule_clock_span_t;

typedef struct {
	schedule_weekday_t weekday;
	schedule_pos_t pos;
} schedule_weekday_pos_t;

typedef struct {
	schedule_weekday_pos_t start;
	schedule_weekday_pos_t end;
} schedule_week_span_t;

typedef struct {
	uint32_t year_matrix[12]; // 12 months, 1 bit per day
	uint8_t week_span_count;
	uint8_t clock_span_count;
	schedule_week_span_t  week_spans[8];
	schedule_clock_span_t clock_spans[8];
} schedule_set_t; 

typedef struct {
	schedule_set_t sets[8];
	uint8_t set_count;
	uint32_t order;
} schedule_t;

bool schedule_parse(schedule_t *sched, const char *text);
bool schedule_next(schedule_t *sched, time_t last, time_t now, time_t *next);
