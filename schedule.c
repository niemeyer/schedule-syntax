#include <string.h>
#include <stdio.h>
#include <time.h>

#include "esp_log.h"
#include "esp_timer.h"

#include "schedule.h"
#include "util.h"

#define TAG "SCHEDULE"

#define SCHEDULE_CLOCK_SPAN_IMPLIED_MINS 15

#define MIN_SPAN 2
#define MAX_SPAN 5

// For 2100+ (y%4>0||(y%100==0&&y%400>0))
#define MONTH_LENGTH(y, m) (30+(((m)&1)^((m)>>3))-((m)!=2?0:1+(y%4>0)))
//#define YEAR_DAY(y, m, d) ((d) + year_days[(m)-1] - ((m) < 3 ? 0 : ((y)%4>0)))
//static const int year_days[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

static const char *schedule_weekdays[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat", NULL};
static const char *schedule_months[] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec", NULL};

typedef struct {
	int16_t year; 
	int16_t month;
	int16_t day;
	int16_t weekday;
	int16_t hour;
	int16_t minute;
	int16_t second;
} schedule_time_t;

static void schedule_time_from_localtime(schedule_time_t *t, time_t time)
{
	struct tm tm;
	localtime_r(&time, &tm);
	t->year    = tm.tm_year+1900;
	t->month   = tm.tm_mon+1;
	t->day     = tm.tm_mday;
	t->weekday = tm.tm_wday;
	t->hour    = tm.tm_hour;
	t->minute  = tm.tm_min;
	t->second  = tm.tm_sec;
	ESP_LOGV(TAG, "Parsed time: hour=%d minute=%d weekday=%d day=%d month=%d year=%d", t->hour, t->minute, t->weekday, t->day, t->month, t->year);
}

static void schedule_time_to_localtime(schedule_time_t *t, time_t *time)
{
	struct tm tm = {
		.tm_year  = t->year-1900,
		.tm_mon   = t->month-1,
		.tm_mday  = t->day,
		.tm_hour  = t->hour,
		.tm_min   = t->minute,
		.tm_sec   = t->second,
		.tm_isdst = -1,
	};
	*time = mktime(&tm);
}

// Consistency rules for these parse functions to be easier to read/write:
//
// - Always start by defining (b = *buf), and use b not buf
// - Only reassign (*buf = b) right before return true
// - End of function is always return false
// - Only one return true
// - On success all fields of target value must be set
//

static bool parse_clock(const char **buf, schedule_clock_t *clock)
{
	const char *b = *buf;
	if (*b >= '0' && *b <= '9') {
		clock->hour = *b - '0';
		b++;
		if (*b >= '0' && *b <= '9') {
			clock->hour *= 10;
			clock->hour += *b - '0';
			b++;
			if (clock->hour == 24) {
				clock->hour = 0;
			}
		}
		if (b[0] == ':' && b[1] >= '0' && b[1] <= '9' && b[2] >= '0' && b[2] <= '9') {
			clock->minute = (b[1] - '0')*10 + (b[2] - '0');
			b += 3;
			if (clock->hour < 24 && clock->minute < 60) {
				*buf = b;
				return true;
			}
		}
	}
	return false;
}

static bool parse_split(const char **buf, uint32_t *split, uint32_t *split_unit)
{
	const char *b = *buf;
	*split_unit = 0;
	if (*b != '/') {
		*split = 1;
	} else {
		*split = 0;
		b++;
		if (*b >= '1' && *b <= '9') {
			for (int i = 0; *b >= '0' && *b <= '9' && i < 5; b++, i++) {
				*split *= 10;
				*split += *b - '0';
			}
			if (*b == 's') {
				*split_unit = 1;
				b++;
			} else if (*b == 'm') {
				*split_unit = 60;
				b++;
			} else if (*b == 'h') {
				*split_unit = 60*60;
				b++;
			}
		}
	}
	if (*split > 0) {
		*buf = b;
		return true;
	}
	return false;
}

static int schedule_clock_span_mins(schedule_clock_span_t *cspan)
{
	int mins = 0;
	if (cspan->end.hour < cspan->start.hour || (cspan->end.hour == cspan->start.hour && cspan->end.minute <= cspan->start.minute)) {
		mins += 24*60;
	}
	mins += cspan->end.hour*60 + cspan->end.minute;
	mins -= cspan->start.hour*60 + cspan->start.minute;
	return mins;
}

static bool parse_clock_span(const char **buf, schedule_clock_span_t *cspan)
{
	const char *b = *buf;
	if (parse_clock(&b, &cspan->start)) {
		if (*b == '-') {
			b++;
			if (!parse_clock(&b, &cspan->end)) {
				return false;
			}
			uint32_t unit;
			if (!parse_split(&b, &cspan->period, &unit)) {
				return false;
			}
			if (unit > 0) {
				cspan->period = cspan->period * unit;
			} else {
				cspan->period = (schedule_clock_span_mins(cspan) * 60) / cspan->period;
				if (cspan->period < 1) {
					cspan->period = 1;
				}
			}
		} else {
			cspan->end.hour = cspan->start.hour;
			cspan->end.minute = cspan->start.minute + SCHEDULE_CLOCK_SPAN_IMPLIED_MINS;
			if (cspan->end.minute > 59) {
				cspan->end.hour = (cspan->end.hour + 1)%24;
				cspan->end.minute %= 60;
			}
			cspan->period = schedule_clock_span_mins(cspan) * 60;
		}
		*buf = b;
		return true;
	}
	return false;
}

static bool parse_weekday(const char **buf, schedule_weekday_pos_t *wpos)
{
	const char *b = *buf;
	int index;
	if (skip_option(&b, schedule_weekdays, &index, 0)) {
		wpos->weekday = index;
		if (*b >= '1' && *b <= '5') {
			wpos->pos = *b - '0';
			b++;
		} else {
			wpos->pos = SCHEDULE_POS_ANY;
		}
		*buf = b;
		return true;
	}
	return false;
}

static bool parse_week_span(const char **buf, schedule_week_span_t *wspan)
{
	const char *b = *buf;
	if (parse_weekday(&b, &wspan->start)) {
		if (*b == '-') {
			b++;
			if (!parse_weekday(&b, &wspan->end)) {
				return false;
			}
		} else {
			memcpy(&wspan->end, &wspan->start, sizeof(schedule_weekday_pos_t));
		}
		*buf = b;
		return true;
	}
	return false;
}

static bool parse_day(const char **buf, uint32_t *day)
{
	const char *b = *buf;
	if (*b >= '1' && *b <= '9') {
		*day = *b - '0';
		b++;
		if (*b >= '0' && *b <= '9') {
			*day *= 10;
			*day += *b - '0';
			b++;
		}
		if (*day <= 31 && *b != ':' && (*b < '0' || *b > '9')) {
			*buf = b;
			return true;
		}
	}
	return false;
}

static bool parse_year_matrix(const char **buf, uint32_t year_matrix[12])
{
	const char *b = *buf;
	uint32_t months = 0;
	uint32_t days = 0;
	uint32_t mstart, mend;
	uint32_t dstart, dend;
	bool comma;
	for (;;) {
		comma = (*b == ',');
		if (comma) {
			b++;
		}
		if ((comma || months == 0) && skip_option(&b, schedule_months, (int *)&mstart, 0)) {
			if (days > 0) {
				for (int m = 0; m < 12; m++) {
					if (((months >> m)&1) > 0) {
						year_matrix[m] |= days;
					}
				}
				months = 0;
				days = 0;
			}
			if (*b == '-') {
				b++;
				if (!skip_option(&b, schedule_months, (int *)&mend, 0)) {
					break;
				}
			} else {
				mend = mstart;
			}
			if (mstart <= mend) {
				months |= ((1 << (mend-mstart+1))-1) << mstart;
			} else {
				months |= ((1 << (mend+1))-1) | (((1 << (12-mstart+1))-1) << mstart);
			}
			continue;
		} else if (months == 0) {
			break;
		}

		if (!comma || !parse_day(&b, &dstart)) {
			if (comma) {
				b--;
			}
			if (days == 0) {
				days = ((1U << 31)-1) << 1;
			}
			for (int m = 0; m < 12; m++) {
				if (((months >> m)&1) > 0) {
					ESP_LOGV(TAG, "... Setting month %d with days=0x%04X", m+1, days);
					year_matrix[m] |= days;
				}
			}
			*buf = b;
			return true;
		}
		if (*b == '-') {
			b++;
			if (!parse_day(&b, &dend)) {
				break;
			}
		} else {
			dend = dstart;
		}
		if (mstart <= mend) {
			days |= ((1 << (dend-dstart+1))-1) << dstart;
		} else {
			days |= (((1 << dend)-1) << 1) | (((1 << (32-dstart))-1) << dstart);
		}
		ESP_LOGV(TAG, "... Parsed days=0x%04X with months=0x%04X", days, months);
	}
	return false;
}

static bool parse_schedule_set(const char **buf, schedule_set_t *set)
{
	const char *b = *buf;
	int step = 0;
	while (step < 3) {
		switch (step) {
		case 0:
			step++;
			if (!parse_year_matrix(&b, set->year_matrix)) {
				memset(&set->year_matrix[0], 0xFF, sizeof(set->year_matrix));
				continue;
			}
			break;
		case 1:
			if (set->week_span_count == sizeof(set->week_spans)/sizeof(set->week_spans[0])) {
				return false;
			}
			if (!parse_week_span(&b, &set->week_spans[set->week_span_count])) {
				step++;
				continue;
			}
			set->week_span_count++;
			break;
		case 2:
			if (set->clock_span_count == sizeof(set->clock_spans)/sizeof(set->clock_spans[0])) {
				return false;
			}
			if (!parse_clock_span(&b, &set->clock_spans[set->clock_span_count])) {
				step++;
				continue;
			}
			set->clock_span_count++;
			break;
		}
		if (*b != ',') {
			break;
		}
		b++;
	}

	if (b != *buf) {
		if (set->week_span_count == 0) {
			// Imply sun-sat
			set->week_span_count = 1;
			set->week_spans[0].start.weekday = SCHEDULE_SUN;
			set->week_spans[0].start.pos = SCHEDULE_POS_ANY;
			set->week_spans[0].end.weekday = SCHEDULE_SAT;
			set->week_spans[0].end.pos = SCHEDULE_POS_ANY;
		}
		if (set->clock_span_count == 0) {
			// Imply 0:00-0:00
			set->clock_span_count = 1;
			set->clock_spans[0].period = 24*3600;
		}
		*buf = b;
		return true;
	}
	return false;
}

static void schedule_set_order(schedule_t *sched)
{
	// Order has 24 bits: <year[24:16]><weeks[15:11]><clocks[10:0]>

	// Year sum, max 366 days per year, 9 bits.
	uint32_t sum = 0;
	for (int m = 0; m < 12; m++) {
		uint32_t days = 0;
		for (int s = 0; s < sched->set_count; s++) {
			days |= sched->sets[s].year_matrix[m];
		}
		days &= ((1 << MONTH_LENGTH(2020, m+1))-1) << 1;
		sum += __builtin_popcount(days);
	}
	sched->order |= sum << 16;
	if (sum > 366) {
		ESP_LOGE(TAG, "Wrong %s schedule order: %d", "year", sum);
	}

	// Week sum, max 28 days, 5 bits.
	uint32_t days = 0;
	for (int s = 0; s < sched->set_count; s++) {
		__auto_type set = &sched->sets[s];
		for (int w = 0; w < set->week_span_count; w++) {
			__auto_type wspan = &set->week_spans[w];

			int start_pos = wspan->start.pos;
			int end_pos = wspan->end.pos;
			int wrap = 28;
			if (start_pos == 5) {
				start_pos = 4;
			}
			if (end_pos == 5) {
				end_pos = 4;
			}
			if (start_pos == 0 && end_pos == 0) {
				start_pos = 0;
				end_pos = 0;
				wrap = 7;
			} else if (start_pos == 0 && end_pos > 0) {
				end_pos--;
				if (end_pos == 0) {
					start_pos = 3;
				} else if (wspan->start.weekday <= wspan->end.weekday) {
					start_pos = end_pos;
				} else {
					start_pos = end_pos-1;
				}
			} else if (end_pos == 0 && start_pos > 0) {
				start_pos--;
				if (start_pos == 0) {
					end_pos = 3;
				} else if (wspan->start.weekday <= wspan->end.weekday) {
					end_pos = start_pos;
				} else {
					end_pos = start_pos+1;
				}
			} else {
				start_pos--;
				end_pos--;
			}
			int start = wspan->start.weekday + (7*start_pos);
			int end = wspan->end.weekday + (7*end_pos);
			uint32_t span;
			if (start <= end) {
				span = ((1 << (end - start + 1)) - 1) << start;
			} else {
				span = ((1 << (end + 1)) - 1) | (((1 << (wrap - start)) - 1) << start);
			}
			days |= span;
			if (wspan->start.pos == 0 && wspan->end.pos == 0) {
				days |= span << 7;
				days |= span << 14;
				days |= span << 21;
			}
		}
	}
	sum = __builtin_popcount(days);
	sched->order |= sum << 11;
	if (sum > 28) {
		ESP_LOGE(TAG, "Wrong %s schedule order: %d", "week", sum);
	}

	// Clock sum, max 1440 mins (24*0-59), 11 bits.
	sum = 0;
	for (int h = 0; h < 24 && sum < 1440; h++) {
		uint64_t mins = 0;
		int hstart = h*60;
		for (int s = 0; s < sched->set_count && sum < 1440; s++) {
			__auto_type set = &sched->sets[s];
			for (int c = 0; c < set->clock_span_count && sum < 1440; c++) {
				__auto_type cspan = &set->clock_spans[c];
				int start = cspan->start.hour*60 + cspan->start.minute;
				int end = cspan->end.hour*60 + cspan->end.minute;

				if (start == end) {
					// Fast path for 24h clock span.
					sched->order |= 1440;
					return;
				}
				if (end == 0) {
					end = 1440;
				}

				start -= hstart;
				end -= hstart;
				if (start <= end) {
					if (start >= 60 || end <= 0) {
						continue;
					}
					if (start < 0) {
						start = 0;
					}
					if (end > 60) {
						end = 60;
					}
					mins |= ((1ULL << (end - start)) - 1) << start;
				} else {
					if (start >= 60 && end <= 0) {
						continue;
					}
					if (start < 0) {
						start = 0;
					} else if (start > 60) {
						start = 60;
					}
					if (end < 0) {
						end = 0;
					} else if (end > 60) {
						end = 60;
					}
					mins |= ((1ULL << end) - 1) | (((1ULL << (60 - start)) - 1) << start);
				}
			}
		}
		sum += __builtin_popcountll(mins);
	}
	sched->order |= sum;
	if (sum > 1440) {
		ESP_LOGE(TAG, "Wrong %s schedule order: %d", "clock", sum);
	}
}

bool schedule_parse(schedule_t *sched, const char *text)
{
	memset(sched, 0, sizeof(schedule_t));
	const char *b = text;
	if (*b == 0) {
		return false;
	}
	for (int i = 0; i < sizeof(sched->sets)/sizeof(sched->sets[0]); i++) {
		if (!parse_schedule_set(&b, &sched->sets[i])) {
			break;
		}
		sched->set_count++;
		if (*b != ',') {
			break;
		}
		b++;
	}
	if (*b == 0) {
		schedule_set_order(sched);
		return true;
	}
	return false;
}

static int schedule_apply_weekday(schedule_time_t *d, int weekday, uint8_t pos, int dir)
{
	ESP_LOGV(TAG, "... Looking for weekday=%d pos=%d dir=%d with %04d-%02d-%02d %s", weekday, pos, dir, d->year, d->month, d->day, schedule_weekdays[d->weekday]);

	// Allowed for convenience when doing math.
	if (weekday < 0 || weekday > 6) {
		weekday = (weekday+7)%7;
	}

	// Applying this delta to the current day we get to the
	// nearest correct weekday towards the wanted direction.
	int delta = dir * ((dir*((int)weekday - d->weekday) + 7)%7);
	d->day += delta;
	d->weekday = weekday;
	ESP_LOGV(TAG, "... Applied delta was %d, day is %d", delta, d->day);

	// Check for fast case: any position is fine and the
	// the day is definitely inside whatever month.
	if (pos == SCHEDULE_POS_ANY && d->day > 0 && d->day < 29) {
		ESP_LOGV(TAG, "... Fast path with pos=ANY: %04d-%02d-%02d %s", d->year, d->month, d->day, schedule_weekdays[d->weekday]);
		return delta;
	}

	int month_days = MONTH_LENGTH(d->year, d->month);
	ESP_LOGV(TAG, "... Source month length is %d", month_days);

	// Loops at most twice to fix the year/month after changes
	// to day. Fixing can only happen once, but day
	// may be wrong for two different reasons.
	for (;;) {
		// If it crossed the month boundary, fix it.
		if (d->day <= 0 || d->day > month_days) {
			ESP_LOGV(TAG, "... Broken: %04d-%02d-(%02d) days=%d", d->year, d->month, d->day, month_days);
			d->month = (d->month-1+dir+12)%12 + 1;
			if (dir > 0) {
				if (d->month == 1) {
					d->year++;
				}
				d->day %= month_days;
			} else if (dir < 0) {
				if (d->month == 12) {
					d->year--;
				}
			}
			month_days = MONTH_LENGTH(d->year, d->month);
			if (dir < 0) {
				d->day += month_days;
			}
			ESP_LOGV(TAG, "... Fixed: %04d-%02d-%02d days=%d", d->year, d->month, d->day, month_days);
		}

		// If any position is okay, it's done.
		if (pos == SCHEDULE_POS_ANY) {
			ESP_LOGV(TAG, "... Return early with pos=ANY: %04d-%02d-%02d %s", d->year, d->month, d->day, schedule_weekdays[d->weekday]);
			break;
		}

		// Compute current position.
		int cur_pos = (d->day + 6)/7;
		int cur_is_last = (month_days - d->day) < 7;
		if (cur_pos == pos) {
			ESP_LOGV(TAG, "... Good pos=%d already: %04d-%02d-%02d %s", cur_pos, d->year, d->month, d->day, schedule_weekdays[d->weekday]);
			break;
		}
		ESP_LOGV(TAG, "... Current position is %d.", cur_pos);

		// The month may still be incorrect if the wanted
		// position is not ahead towards desired direction, but
		// this can only ever happen if the month wasn't crossed
		// before, as if that were the case the position would
		// be at the very start or very end of the other month
		// already and any position at all would be ahead still.
		int pos_delta;
		if (dir < 0) {
			if ((pos < 5 && cur_pos < pos) || (pos == 5 && !cur_is_last)) {
				pos_delta = cur_pos * 7;
				d->day -= pos_delta;
				delta -= pos_delta;
				ESP_LOGV(TAG, "... Changing month: delta=%d cur_pos=%d pos=%d pos_delta=%d day=%d", delta, cur_pos, pos, pos_delta, d->day);
				continue;
			}
		} else if (dir > 0) {
			if (cur_pos > pos) {
				pos_delta = (month_days - d->day + 7)/7*7;
				d->day += pos_delta;
				delta += pos_delta;
				ESP_LOGV(TAG, "... Changing month: delta=%d cur_pos=%d pos=%d pos_delta=%d day=%d", delta, cur_pos, pos, pos_delta, d->day);
				continue;
			}
		}

		// At this point the month is definitely correct and it's
		// just a matter of shifting the position towards dir.
		if (pos == 5) {
			pos_delta = (month_days - d->day)/7*7;
		} else {
			pos_delta = (pos-cur_pos)*7;
		}
		d->day += pos_delta;
		delta += pos_delta;
		ESP_LOGV(TAG, "... Return shifted pos=%d: %04d-%02d-%02d %s", pos, d->year, d->month, d->day, schedule_weekdays[d->weekday]);
		break;
	}

	return delta;
}

static void schedule_skip_ahead(schedule_time_t *d, uint32_t year_matrix[12])
{
	ESP_LOGV(TAG, "... Looking ahead on year matrix with %04d-%02d-%02d %s", d->year, d->month, d->day, schedule_weekdays[d->weekday]);

	int year = d->year;
	int month = d->month;
	int day = d->day;
	int delta = 0;
	int month_days = MONTH_LENGTH(year, month);

	delta++;
	day++;
	if (day > month_days) {
		day = 1;
		month++;
		if (month > 12) {
			month = 1;
			year++;
		}
		month_days = MONTH_LENGTH(year, month);
	}

	ESP_LOGV(TAG, "... before finding month: delta=%d", delta);

	// Find month with good day.
	while ((year_matrix[month-1] >> day) == 0) {
		ESP_LOGV(TAG, "... year_matrix %04d-%02d is zero after day=%d", year, month, day);
		if (day > 1) {
			delta -= day - 1;
			day = 1;
		}
		delta += month_days;
		month++;
		if (month > 12) {
			month = 1;
			year++;
		}
		month_days = MONTH_LENGTH(year, month);
	}
	ESP_LOGV(TAG, "... year_matrix %04d-%02d is not zero after day=%d", year, month, day);

	// Skip straight to next good day in month.
	int skip = __builtin_ctz(year_matrix[month-1] >> day);

	ESP_LOGV(TAG, "... year_matrix %04d-%02d starts on day=%d after day=%d", year, month, day+skip, day);

	ESP_LOGV(TAG, "... before skip: delta=%d", delta);

	delta += skip;
	day += skip;
	if (day > month_days) {
		delta -= day-month_days;
		day = month_days;
	}
	
	ESP_LOGV(TAG, "... after skip: delta=%d", delta);

	d->weekday = (d->weekday + delta)%7;
	d->day = day;
	d->month = month;
	d->year = year;

	ESP_LOGV(TAG, "... Return day: %04d-%02d-%02d %s", d->year, d->month, d->day, schedule_weekdays[d->weekday]);
}

#define ORDER(before, after) \
		((before)->year < (after)->year || ((before)->year == (after)->year && \
			((before)->month < (after)->month || ((before)->month == (after)->month && \
				((before)->day < (after)->day || ((before)->day == (after)->day && \
					((before)->hour < (after)->hour || ((before)->hour == (after)->hour && \
						((before)->minute < (after)->minute || ((before)->minute == (after)->minute && \
							((before)->second <= (after)->second)))))))))))

static void schedule_next_set(schedule_set_t *set, schedule_time_t *last, schedule_time_t *now, schedule_time_t *next)
{
	ESP_LOGV(TAG, "Checking set for any earlier next...");
	ESP_LOGV(TAG, "Current last: %04d-%02d-%02d %s %02d:%02d:%02d", last->year, last->month, last->day, schedule_weekdays[last->weekday], last->hour, last->minute, last->second);
	ESP_LOGV(TAG, "Current next: %04d-%02d-%02d %s %02d:%02d:%02d", next->year, next->month, next->day, schedule_weekdays[next->weekday], next->hour, next->minute, next->second);

	// Very first possible day to start the search from is the day before,
	// as it may have a span crossing midnight. From there, look ahead for
	// a good place to start from in the year matrix.
	schedule_time_t ystart;
	memcpy(&ystart, now, sizeof(schedule_time_t));
	ystart.hour = 0;
	ystart.minute = 0;
	ystart.second = 0;

	// Save time by starting all week spans from the earliest known good place.
	schedule_apply_weekday(&ystart, ystart.weekday-1, SCHEDULE_POS_ANY, -1);
	if (((set->year_matrix[ystart.month-1] >> ystart.day)&1) == 0) {
		schedule_skip_ahead(&ystart, set->year_matrix);
	}

	for (int wspan_i = 0; wspan_i < set->week_span_count; wspan_i++) {
		schedule_week_span_t *wspan = &set->week_spans[wspan_i];

		bool wspan_backwards = (wspan->start.pos == SCHEDULE_POS_ANY || wspan->end.pos != SCHEDULE_POS_ANY);

		ESP_LOGV(TAG, "Checking week span...");

		// While there's at least one span combination that is behind (its end
		// is past, or it already happened since last is inside its span),
		// there's potential for it to be the earliest time when it iterates
		// across the current time and into the future. So the logic below
		// iterates while it doesn't find the first future occurrence for
		// combinations of different span types. After that, _next_ will hold
		// the start of the earliest valid span observed, or will be untouched
		// if its prior value was earlier than all of them.
		bool clock_span_pending = true;

		schedule_time_t wstart, wend;
		memcpy(&wstart, &ystart, sizeof(schedule_time_t));
		bool skipped = true;

		for (int attempt = 100; clock_span_pending && attempt > 0; attempt--) {
			
			// In this logic wstart does not move backwards, so it's
			// easier to make it correct, and move forwards
			// alternating week and year, so long sparse spans are
			// traversed quickly.

			if (((set->year_matrix[wstart.month-1] >> wstart.day)&1) == 0) {
				schedule_skip_ahead(&wstart, set->year_matrix);
				skipped = true;
			}
			if (skipped) {
				// Go back and forth so ranges crossing the new wstart are observed.
				memcpy(&wend, &wstart, sizeof(schedule_time_t));
				schedule_apply_weekday(&wend, wspan->start.weekday, wspan->start.pos, -1);
			}
			int delta = schedule_apply_weekday(&wend, wspan->end.weekday, wspan->end.pos, +1);
			if (!ORDER(&wstart, &wend)) {
				schedule_apply_weekday(&wstart, wspan->start.weekday, wspan->start.pos, +1);
				memcpy(&wend, &wstart, sizeof(schedule_time_t));
				continue;
			}
			if (wspan_backwards && delta > 6) {
				memcpy(&wstart, &wend, sizeof(schedule_time_t));
				schedule_apply_weekday(&wstart, wspan->start.weekday, wspan->start.pos, -1);
				if (((set->year_matrix[wstart.month-1] >> wstart.day)&1) == 0) {
					continue;
				}
			}

			// Iterate over each day in the week span by moving wstart towards wend/mend.
			while (clock_span_pending && ORDER(&wstart, &wend)) {

				ESP_LOGV(TAG, "Attempt start: %04d-%02d-%02d %s %02d:%02d", wstart.year, wstart.month, wstart.day, schedule_weekdays[wstart.weekday], wstart.hour, wstart.minute);
				ESP_LOGV(TAG, "Attempt end:   %04d-%02d-%02d %s %02d:%02d", wend.year, wend.month, wend.day, schedule_weekdays[wend.weekday], wend.hour, wend.minute);

				clock_span_pending = false;

				// Iterate over each clock span.
				for (int cspan_i = 0; cspan_i < set->clock_span_count; cspan_i++) {
					schedule_clock_span_t *cspan = &set->clock_spans[cspan_i];

					schedule_time_t cstart, cend;
					memcpy(&cstart, &wstart, sizeof(schedule_time_t)); 
					memcpy(&cend, &wstart, sizeof(schedule_time_t)); 
					cstart.hour = cspan->start.hour;
					cstart.minute = cspan->start.minute;
					cend.hour = cspan->end.hour;
					cend.minute = cspan->end.minute;

					if (cend.hour < cstart.hour || (cend.hour == cstart.hour && cend.minute <= cstart.minute)) {
						// Ending is on the next day.
						schedule_apply_weekday(&cend, cend.weekday+1, SCHEDULE_POS_ANY, +1);
					}

					ESP_LOGV(TAG, "Clock span end: %04d-%02d-%02d %s %02d:%02d:%02d", cend.year, cend.month, cend.day, schedule_weekdays[cend.weekday], cend.hour, cend.minute, cend.second);

					if (ORDER(&cend, now) || ORDER(&cend, last)) {
						// Window entirely in the past.
						ESP_LOGV(TAG, "Span end before now or last. Skip span.");
						clock_span_pending = true;
						continue;
					} else {
						ESP_LOGV(TAG, "Span end after now and last. Good.");
					}

					int part = 1;
					if (ORDER(&cstart, last)) {
						ESP_LOGV(TAG, "Clock span has last inside it. Try to split it.");

						// As most last is on day right after cstart, otherwise cend would
						// be behind last and cspan must have been skipped earlier.
						int cstart_secs = cstart.hour*3600 + cstart.minute*60 + cstart.second;
						int last_secs = last->hour*3600 + last->minute*60 + last->second;
						if (last->day != cstart.day) {
							last_secs += 24*3600;
						}

						part = (last_secs - cstart_secs + cspan->period) / cspan->period;
						int part_secs = part * cspan->period;

						if (part_secs >= schedule_clock_span_mins(cspan) * 60) {
							ESP_LOGV(TAG, "Clock span has last inside its last part. Skip span.");
							clock_span_pending = true;
							continue;
						}

						cstart_secs += part_secs;
						cstart.hour   = cstart_secs/3600;
						cstart.minute = cstart_secs/60%60;
						cstart.second = cstart_secs%60;

						if (cstart.hour >= 24) {
							cstart.hour -= 24;
							schedule_apply_weekday(&cstart, cstart.weekday+1, SCHEDULE_POS_ANY, +1);
						}
					}

					ESP_LOGV(TAG, "Clock span start (part %d): %04d-%02d-%02d %s %02d:%02d:%02d", part, cstart.year, cstart.month, cstart.day, schedule_weekdays[cstart.weekday], cstart.hour, cstart.minute, cstart.second);

					if (ORDER(&cstart, next)) {
						memcpy(next, &cstart, sizeof(schedule_time_t));
						ESP_LOGV(TAG, "First valid clock span start (part %d) is before current next. Replace next!", part);
					} else {
						ESP_LOGV(TAG, "First valid clock span start (part %d) is past current next. Ignore span.", part);
					}
				}
				schedule_skip_ahead(&wstart, set->year_matrix);
			}
			skipped = true;
		}
	}
}

bool schedule_next(schedule_t *sched, time_t last, time_t now, time_t *next)
{
	schedule_time_t _last, _now;
	schedule_time_from_localtime(&_last, last);
	schedule_time_from_localtime(&_now,  now);

	//int64_t start = esp_timer_get_time();
	schedule_time_t _next = {.year = 9999};
	for (int i = 0; i < sched->set_count; i++) {
		schedule_next_set(&sched->sets[i], &_last, &_now, &_next);
	}
	//printf("Execution time: %lldus\n", esp_timer_get_time()-start);
	ESP_LOGV(TAG, "Final next: %04d-%02d-%02d %s %02d:%02d:%02d", _next.year, _next.month, _next.day, schedule_weekdays[_next.weekday], _next.hour, _next.minute, _next.second);
	if (_next.year == 9999) {
		return false;
	}

	schedule_time_to_localtime(&_next, next);
	return true;
}
