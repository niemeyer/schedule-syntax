#include <stdio.h>

#include "unity.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "schedule.c"

#define TEST_TAG "TEST"


TEST_CASE("Schedule parsing of week spans", "[schedule]")
{
	schedule_t sched;
	bool res = schedule_parse(&sched, "mon-fri,tue3,sat,,wed5-sun");

	TEST_ASSERT_EQUAL(2, sched.set_count);
	TEST_ASSERT_EQUAL(3, sched.sets[0].week_span_count);
	TEST_ASSERT_EQUAL(1, sched.sets[0].clock_span_count);
	TEST_ASSERT_EQUAL(1, sched.sets[1].week_span_count);
	TEST_ASSERT_EQUAL(1, sched.sets[1].clock_span_count);
	TEST_ASSERT_EQUAL(0, sched.sets[2].week_span_count);
	TEST_ASSERT_EQUAL(0, sched.sets[2].clock_span_count);

	TEST_ASSERT_EQUAL(1, sched.sets[0].week_spans[0].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[0].start.pos);
	TEST_ASSERT_EQUAL(5, sched.sets[0].week_spans[0].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[0].end.pos);

	TEST_ASSERT_EQUAL(2, sched.sets[0].week_spans[1].start.weekday);
	TEST_ASSERT_EQUAL(3, sched.sets[0].week_spans[1].start.pos);
	TEST_ASSERT_EQUAL(2, sched.sets[0].week_spans[1].end.weekday);
	TEST_ASSERT_EQUAL(3, sched.sets[0].week_spans[1].end.pos);

	TEST_ASSERT_EQUAL(6, sched.sets[0].week_spans[2].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[2].start.pos);
	TEST_ASSERT_EQUAL(6, sched.sets[0].week_spans[2].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[2].end.pos);

	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[3].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[3].start.pos);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[3].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[3].end.pos);

	TEST_ASSERT_EQUAL(3, sched.sets[1].week_spans[0].start.weekday);
	TEST_ASSERT_EQUAL(5, sched.sets[1].week_spans[0].start.pos);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[0].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[0].end.pos);

	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[1].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[1].start.pos);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[1].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[1].end.pos);

	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[0].start.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[0].start.minute);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[0].end.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[0].end.minute);
	TEST_ASSERT_EQUAL(86400, sched.sets[0].clock_spans[0].period);

	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[0].start.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[0].start.minute);
	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[0].end.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[0].end.minute);
	TEST_ASSERT_EQUAL(86400, sched.sets[1].clock_spans[0].period);

	TEST_ASSERT(res == true);
}

TEST_CASE("Schedule parsing of clock spans", "[schedule]")
{
	schedule_t sched;
	bool res = schedule_parse(&sched, "10:05-20:35/126,0:00-0:00,24:00,,12:34,1:00-2:00/30s,3:00-4:00/10m,5:00-7:00/1h");

	TEST_ASSERT_EQUAL(2, sched.set_count);
	TEST_ASSERT_EQUAL(1, sched.sets[0].week_span_count);
	TEST_ASSERT_EQUAL(3, sched.sets[0].clock_span_count);
	TEST_ASSERT_EQUAL(1, sched.sets[1].week_span_count);
	TEST_ASSERT_EQUAL(4, sched.sets[1].clock_span_count);
	TEST_ASSERT_EQUAL(0, sched.sets[2].week_span_count);
	TEST_ASSERT_EQUAL(0, sched.sets[2].clock_span_count);

	TEST_ASSERT_EQUAL(10, sched.sets[0].clock_spans[0].start.hour);
	TEST_ASSERT_EQUAL(5,  sched.sets[0].clock_spans[0].start.minute);
	TEST_ASSERT_EQUAL(20, sched.sets[0].clock_spans[0].end.hour);
	TEST_ASSERT_EQUAL(35, sched.sets[0].clock_spans[0].end.minute);
	TEST_ASSERT_EQUAL(300,  sched.sets[0].clock_spans[0].period);

	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[1].start.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[1].start.minute);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[1].end.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[1].end.minute);
	TEST_ASSERT_EQUAL(24*60*60, sched.sets[0].clock_spans[1].period);

	TEST_ASSERT_EQUAL(0,  sched.sets[0].clock_spans[2].start.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[0].clock_spans[2].start.minute);
	TEST_ASSERT_EQUAL(0,  sched.sets[0].clock_spans[2].end.hour);
	TEST_ASSERT_EQUAL(15, sched.sets[0].clock_spans[2].end.minute);
	TEST_ASSERT_EQUAL(15*60, sched.sets[0].clock_spans[2].period);

	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[3].start.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[3].start.minute);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[3].end.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[3].end.minute);
	TEST_ASSERT_EQUAL(0, sched.sets[0].clock_spans[3].period);

	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[0].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[0].start.pos);
	TEST_ASSERT_EQUAL(6, sched.sets[0].week_spans[0].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[0].end.pos);

	TEST_ASSERT_EQUAL(12, sched.sets[1].clock_spans[0].start.hour);
	TEST_ASSERT_EQUAL(34, sched.sets[1].clock_spans[0].start.minute);
	TEST_ASSERT_EQUAL(12, sched.sets[1].clock_spans[0].end.hour);
	TEST_ASSERT_EQUAL(34+15, sched.sets[1].clock_spans[0].end.minute);
	TEST_ASSERT_EQUAL(15*60, sched.sets[1].clock_spans[0].period);

	TEST_ASSERT_EQUAL(1,  sched.sets[1].clock_spans[1].start.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[1].clock_spans[1].start.minute);
	TEST_ASSERT_EQUAL(2,  sched.sets[1].clock_spans[1].end.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[1].clock_spans[1].end.minute);
	TEST_ASSERT_EQUAL(30, sched.sets[1].clock_spans[1].period);

	TEST_ASSERT_EQUAL(3,  sched.sets[1].clock_spans[2].start.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[1].clock_spans[2].start.minute);
	TEST_ASSERT_EQUAL(4,  sched.sets[1].clock_spans[2].end.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[1].clock_spans[2].end.minute);
	TEST_ASSERT_EQUAL(600, sched.sets[1].clock_spans[2].period);

	TEST_ASSERT_EQUAL(5,  sched.sets[1].clock_spans[3].start.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[1].clock_spans[3].start.minute);
	TEST_ASSERT_EQUAL(7,  sched.sets[1].clock_spans[3].end.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[1].clock_spans[3].end.minute);
	TEST_ASSERT_EQUAL(3600, sched.sets[1].clock_spans[3].period);

	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[4].start.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[4].start.minute);
	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[4].end.hour);
	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[4].end.minute);
	TEST_ASSERT_EQUAL(0, sched.sets[1].clock_spans[4].period);

	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[0].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[0].start.pos);
	TEST_ASSERT_EQUAL(6, sched.sets[1].week_spans[0].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[0].end.pos);

	TEST_ASSERT(res == true);
}

TEST_CASE("Schedule parsing of month spans", "[schedule]")
{
	schedule_t sched;
	bool res = schedule_parse(&sched, "dec-mar,may,5-10,jan,15-20,,jan,1-31,feb");
	TEST_ASSERT_EQUAL(true, res);
	TEST_ASSERT_EQUAL(2, sched.set_count);

	char msg[128];
	for (int s = 0; s < 3; s++) {
		for (int m = 0; m < 12; m++) {
			uint32_t obtained = sched.sets[s].year_matrix[m];
			uint32_t expected = 0;
			int month = m+1;
			if (s == 0) {
				if (month == 1) {
					expected = 0x1F87E0; // 5-10,15-20
				} else if (month == 2 || month == 3 || month == 5 || month == 12) {
					expected = 0x07E0;   // 5-10
				}
			} else if (s == 1) {
				if (month == 1 || month == 2) {
					expected = 0xFFFFFFFE; // 1-31, explicit and implicit
				}
			}

			sprintf(msg, "set=%d month=%d", s, m+1);
			TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, obtained, msg);
		}
	}
}

TEST_CASE("Schedule parsing of mixed sets", "[schedule]")
{
	schedule_t sched;
	bool res = schedule_parse(&sched, "jan,10-15,5,mon-wed2,fri,09:45,13:00-17:00/5,,feb,sat,12:34");

	uint32_t zero[12] = {0};

	TEST_ASSERT_EQUAL(2, sched.set_count);
	TEST_ASSERT_EQUAL(2, sched.sets[0].week_span_count);
	TEST_ASSERT_EQUAL(2, sched.sets[0].clock_span_count);
	TEST_ASSERT_EQUAL(1, sched.sets[1].week_span_count);
	TEST_ASSERT_EQUAL(1, sched.sets[1].clock_span_count);
	TEST_ASSERT_EQUAL(0, sched.sets[2].week_span_count);
	TEST_ASSERT_EQUAL(0, sched.sets[2].clock_span_count);

	TEST_ASSERT_EQUAL_UINT32(0xFC20 /* 5,10-15 */, sched.sets[0].year_matrix[0]);
	TEST_ASSERT_EQUAL(0, memcmp(zero, &sched.sets[0].year_matrix[1], sizeof(uint32_t)*11));

	TEST_ASSERT_EQUAL(1, sched.sets[0].week_spans[0].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[0].start.pos);
	TEST_ASSERT_EQUAL(3, sched.sets[0].week_spans[0].end.weekday);
	TEST_ASSERT_EQUAL(2, sched.sets[0].week_spans[0].end.pos);

	TEST_ASSERT_EQUAL(5, sched.sets[0].week_spans[1].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[1].start.pos);
	TEST_ASSERT_EQUAL(5, sched.sets[0].week_spans[1].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[0].week_spans[1].end.pos);

	TEST_ASSERT_EQUAL(9,  sched.sets[0].clock_spans[0].start.hour);
	TEST_ASSERT_EQUAL(45, sched.sets[0].clock_spans[0].start.minute);
	TEST_ASSERT_EQUAL(10, sched.sets[0].clock_spans[0].end.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[0].clock_spans[0].end.minute);
	TEST_ASSERT_EQUAL(15*60, sched.sets[0].clock_spans[0].period);

	TEST_ASSERT_EQUAL(13, sched.sets[0].clock_spans[1].start.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[0].clock_spans[1].start.minute);
	TEST_ASSERT_EQUAL(17, sched.sets[0].clock_spans[1].end.hour);
	TEST_ASSERT_EQUAL(0,  sched.sets[0].clock_spans[1].end.minute);
	TEST_ASSERT_EQUAL(((17-13)*60*60)/5, sched.sets[0].clock_spans[1].period);

	TEST_ASSERT_EQUAL_UINT32(0, sched.sets[1].year_matrix[0]);
	TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFE /* 1-31 */, sched.sets[1].year_matrix[1]);
	TEST_ASSERT_EQUAL(0, memcmp(zero, &sched.sets[1].year_matrix[2], sizeof(uint32_t)*10));

	TEST_ASSERT_EQUAL(6, sched.sets[1].week_spans[0].start.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[0].start.pos);
	TEST_ASSERT_EQUAL(6, sched.sets[1].week_spans[0].end.weekday);
	TEST_ASSERT_EQUAL(0, sched.sets[1].week_spans[0].end.pos);

	TEST_ASSERT_EQUAL(12, sched.sets[1].clock_spans[0].start.hour);
	TEST_ASSERT_EQUAL(34, sched.sets[1].clock_spans[0].start.minute);
	TEST_ASSERT_EQUAL(12, sched.sets[1].clock_spans[0].end.hour);
	TEST_ASSERT_EQUAL(34+15, sched.sets[1].clock_spans[0].end.minute);
	TEST_ASSERT_EQUAL(15*60, sched.sets[1].clock_spans[0].period);

	TEST_ASSERT(res == true);
}

TEST_CASE("Schedule applying of weekday", "[schedule]")
{
	struct test_case {
		int src_year;
		int src_month;
		int src_day;
		int src_weekday;

		int apply_weekday;
		int apply_pos;
		int apply_dir;

		int dst_year;
		int dst_month;
		int dst_day;
	};

	struct test_case tests[] = {{
		// Trivial and fast case.
		.src_year=2020,
		.src_month=1,
		.src_day=15,
		.src_weekday=SCHEDULE_WED,

		.apply_weekday=SCHEDULE_MON,
		.apply_pos=SCHEDULE_POS_ANY,
		.apply_dir=-1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=13,
	}, {
		// Relatively fast case.
		.src_year=2020,
		.src_month=1,
		.src_day=15,
		.src_weekday=SCHEDULE_WED,

		.apply_weekday=SCHEDULE_MON,
		.apply_pos=SCHEDULE_POS_2,
		.apply_dir=-1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=13,
	}, {
		// Shift forward without month change.
		.src_weekday=SCHEDULE_WED,
		.src_year=2020,
		.src_month=1,
		.src_day=8,

		.apply_weekday=SCHEDULE_THU,
		.apply_pos=SCHEDULE_POS_4,
		.apply_dir=1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=23,
	}, {
		// Shift backward without month change.
		.src_weekday=SCHEDULE_THU,
		.src_year=2020,
		.src_month=1,
		.src_day=23,

		.apply_weekday=SCHEDULE_WED,
		.apply_pos=SCHEDULE_POS_2,
		.apply_dir=-1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=8,
	}, {
		// Allow weekday+1 for convenience.
		.src_weekday=SCHEDULE_WED,
		.src_year=2020,
		.src_month=1,
		.src_day=15,

		.apply_weekday=SCHEDULE_SAT + 1,
		.apply_pos=SCHEDULE_POS_ANY,
		.apply_dir=1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=19,
	}, {
		// Allow weekday-1 for convenience.
		.src_weekday=SCHEDULE_WED,
		.src_year=2020,
		.src_month=1,
		.src_day=15,

		.apply_weekday=SCHEDULE_SUN - 1,
		.apply_pos=SCHEDULE_POS_ANY,
		.apply_dir=1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=18,
	}, {
		// Tests crossing the year backward.
		.src_weekday=SCHEDULE_WED,
		.src_year=2020,
		.src_month=1,
		.src_day=1,

		.apply_weekday=SCHEDULE_MON,
		.apply_pos=SCHEDULE_POS_ANY,
		.apply_dir=-1,

		.dst_year=2019,
		.dst_month=12,
		.dst_day=30,
	}, {
		.src_weekday=SCHEDULE_WED,
		.src_year=2020,
		.src_month=1,
		.src_day=8,

		.apply_weekday=SCHEDULE_MON,
		.apply_pos=SCHEDULE_POS_LAST,
		.apply_dir=-1,

		.dst_year=2019,
		.dst_month=12,
		.dst_day=30,
	}, {
		.src_weekday=SCHEDULE_WED,
		.src_year=2020,
		.src_month=1,
		.src_day=15,

		.apply_weekday=SCHEDULE_MON,
		.apply_pos=SCHEDULE_POS_3,
		.apply_dir=-1,

		.dst_year=2019,
		.dst_month=12,
		.dst_day=16,
	}, {
		.src_weekday=SCHEDULE_MON,
		.src_year=2020,
		.src_month=1,
		.src_day=6,

		.apply_weekday=SCHEDULE_TUE,
		.apply_pos=SCHEDULE_POS_1,
		.apply_dir=-1,

		.dst_year=2019,
		.dst_month=12,
		.dst_day=3,
	}, {
		// Tests crossing the year forward.
		.src_weekday=SCHEDULE_TUE,
		.src_year=2019,
		.src_month=12,
		.src_day=31,

		.apply_weekday=SCHEDULE_WED,
		.apply_pos=SCHEDULE_POS_ANY,
		.apply_dir=1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=1,
	}, {
		.src_weekday=SCHEDULE_TUE,
		.src_year=2019,
		.src_month=12,
		.src_day=31,

		.apply_weekday=SCHEDULE_MON,
		.apply_pos=SCHEDULE_POS_ANY,
		.apply_dir=1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=6,
	}, {
		.src_weekday=SCHEDULE_TUE,
		.src_year=2019,
		.src_month=12,
		.src_day=31,

		.apply_weekday=SCHEDULE_FRI,
		.apply_pos=SCHEDULE_POS_LAST,
		.apply_dir=1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=31,
	}, {
		.src_weekday=SCHEDULE_THU,
		.src_year=2020,
		.src_month=1,
		.src_day=9,

		.apply_weekday=SCHEDULE_MON,
		.apply_pos=SCHEDULE_POS_ANY,
		.apply_dir=-1,

		.dst_year=2020,
		.dst_month=1,
		.dst_day=6,
	}};

	for (int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		__auto_type test = &tests[i];

		ESP_LOGV(TEST_TAG, "======");

		schedule_time_t d = {
			.year    = test->src_year,
			.month   = test->src_month,
			.day     = test->src_day,
			.weekday = test->src_weekday,
		};

		schedule_apply_weekday(&d, test->apply_weekday, test->apply_pos, test->apply_dir);

		int dst_weekday = (test->apply_weekday + 7)%7;

		TEST_ASSERT_EQUAL(test->dst_year,  d.year);
		TEST_ASSERT_EQUAL(test->dst_month, d.month);
		TEST_ASSERT_EQUAL(test->dst_day,   d.day);
		TEST_ASSERT_EQUAL(dst_weekday,     d.weekday);
	}
}

TEST_CASE("Schedule next", "[schedule]")
{
	struct test_case {
		const char *schedule;
		schedule_time_t last;
		schedule_time_t curr;
		schedule_time_t next;
	};

	struct test_case tests[] = {


	// === Weeks ===
	{
		// Simple single-day next week matching.
		.schedule="mon",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 13, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Two sets, first one comes first.
		.schedule="mon,tue-wed",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 13, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Two sets, second one comes first.
		.schedule="tue,mon-wed",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 13, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Match following week as last and now overlap.
		.schedule="thu",
		.last={2020, 1,  9, SCHEDULE_THU, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 16, SCHEDULE_THU, 0,  0, 0},
	}, {
		// Now is inside week span, but match next day as last and now overlap.
		.schedule="sun-fri",
		.last={2020, 1,  9, SCHEDULE_THU, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 0,  0, 0},
	}, {
		// Similar case but on last day of span, so skips to following week.
		.schedule="sun-fri",
		.last={2020, 1, 10, SCHEDULE_FRI, 0,  0, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 12, SCHEDULE_SUN, 0,  0, 0},
	}, {
		// Positioned weekday.
		.schedule="mon3",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 20, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Same, but with range.
		.schedule="mon3-sun",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 20, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Same, but with reverse range positioning.
		.schedule="mon-sun4",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 20, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Same, but with range crossing year.
		.schedule="mon-fri1",
		.last={2019, 12, 27, SCHEDULE_FRI, 0,  0, 0},
		.curr={2019, 12, 27, SCHEDULE_FRI, 8, 45, 0},
		.next={2019, 12, 30, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Similar, but skip to next span across month.
		.schedule="mon-fri1",
		.last={2020, 1, 3, SCHEDULE_FRI, 0,  0, 0},
		.curr={2020, 1, 3, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 2, 3, SCHEDULE_MON, 0,  0, 0},
	},

	// === Clocks ===
	{
		// Simple single clock case moving forward on same day.
		.schedule="10:00",
		.last={2020, 1, 10, SCHEDULE_FRI, 0,  0, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 10, 0, 0},
	}, {
		// Simple single clock case moving forward to next day.
		.schedule="8:00",
		.last={2020, 1, 10, SCHEDULE_FRI, 8, 10, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 11, SCHEDULE_SAT, 8,  0, 0},
	}, {
		// Two clocks, first one comes first.
		.schedule="9:00,10:00",
		.last={2020, 1, 10, SCHEDULE_FRI, 0,  0, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 9,  0, 0},
	}, {
		// Two clocks, second one comes first.
		.schedule="10:00,9:00",
		.last={2020, 1, 10, SCHEDULE_FRI, 0,  0, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 9,  0, 0},
	}, {
		// Current time inside clock span.
		.schedule="8:00-9:00",
		.last={2020, 1, 10, SCHEDULE_FRI, 0,  0, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 8,  0, 0},
	}, {
		// Same, but last also inside it, so skip span.
		.schedule="8:00-9:00",
		.last={2020, 1, 10, SCHEDULE_FRI, 8, 15, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 11, SCHEDULE_SAT, 8,  0, 0},
	}, {
		// Same, but split in three so it isn't skipped.
		.schedule="8:00-9:00/3",
		.last={2020, 1, 10, SCHEDULE_FRI, 8, 15, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 8, 20, 0},
	}, {
		// Same, but cross-day, event on first day.
		.schedule="23:30-0:30/3",
		.last={2020, 1, 10, SCHEDULE_FRI, 23, 45, 0},
		.curr={2020, 1, 11, SCHEDULE_SAT,  0, 25, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 23, 50, 0},
	}, {
		// Same, but cross-day, event on second day.
		.schedule="23:30-0:30/3",
		.last={2020, 1, 10, SCHEDULE_FRI, 23, 55, 0},
		.curr={2020, 1, 11, SCHEDULE_SAT,  0, 25, 0},
		.next={2020, 1, 11, SCHEDULE_SAT,  0, 10, 0},
	}, {
		// Split unit with hours.
		.schedule="8:00-10:00/1h",
		.last={2020, 1, 10, SCHEDULE_FRI, 8, 15, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 9,  0, 0},
	}, {
		// Split unit with minutes.
		.schedule="8:00-10:00/30m",
		.last={2020, 1, 10, SCHEDULE_FRI, 8, 15, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 8, 30, 0},
	}, {
		// Split unit with minutes, last further in.
		.schedule="8:00-10:00/25m",
		.last={2020, 1, 10, SCHEDULE_FRI, 8, 55, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 55, 0},
		.next={2020, 1, 10, SCHEDULE_FRI, 9, 15, 0},
	}, {
		// Split unit with seconds.
		.schedule="8:00-10:00/30s",
		.last={2020, 1, 10, SCHEDULE_FRI, 8, 15,  0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45,  0},
		.next={2020, 1, 10, SCHEDULE_FRI, 8, 15, 30},
	}, {
		// Same, but last has seconds too.
		.schedule="8:00-10:00/25s",
		.last={2020, 1, 10, SCHEDULE_FRI, 8, 15, 55},
		.curr={2020, 1, 10, SCHEDULE_FRI, 8, 45,  0},
		.next={2020, 1, 10, SCHEDULE_FRI, 8, 16, 15},
	}, {
		// Second split across the year.
		.schedule="23:59-1:00/25s",
		.last={2019, 12, 31, SCHEDULE_TUE, 23, 59, 55},
		.curr={2019, 12, 31, SCHEDULE_TUE, 23, 59, 57},
		.next={2020,  1,  1, SCHEDULE_WED,  0,  0, 15},
	},

	// === Clocks and weeks ===
	{
		// Simple single-day next week matching.
		.schedule="mon,8:00",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 20, 0},
		.next={2020, 1, 13, SCHEDULE_MON, 8,  0, 0},
	}, {
		// Same day, but past the implied span end.
		.schedule="thu,8:00",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 20, 0},
		.next={2020, 1, 16, SCHEDULE_THU, 8,  0, 0},
	}, {
		// Same day, inside the implied span still.
		.schedule="thu,8:00",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 10, 0},
		.next={2020, 1,  9, SCHEDULE_THU, 8,  0, 0},
	}, {
		// Same day, past implied cross-day end.
		.schedule="thu,23:55",
		.last={2020, 1,  1, SCHEDULE_WED, 0,   0, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 0,  15, 0},
		.next={2020, 1, 16, SCHEDULE_THU, 23, 55, 0},
	}, {
		// Same day, inside implied cross-day end.
		.schedule="thu,23:55",
		.last={2020, 1,  1, SCHEDULE_WED, 0,   0, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 0,   5, 0},
		.next={2020, 1,  9, SCHEDULE_THU, 23, 55, 0},
	}, {
		// Same day, inside implied cross-day end, but
		// event already happened on prior day.
		.schedule="thu,23:55",
		.last={2020, 1,  9, SCHEDULE_THU, 23, 58, 0},
		.curr={2020, 1, 10, SCHEDULE_FRI, 0,   5, 0},
		.next={2020, 1, 16, SCHEDULE_THU, 23, 55, 0},
	}, {
		// Same day, inside explicit span.
		.schedule="thu,8:00-9:00",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1,  9, SCHEDULE_THU, 8,  0, 0},
	}, {
		// Interesting case: current time is past the specified day,
		// but because the span crosses midnight, the next event is
		// emitted for the prior day.
		.schedule="wed,23:00-1:00",
		.last={2020, 1,  8, SCHEDULE_WED, 8, 30, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 0, 45, 0},
		.next={2020, 1,  8, SCHEDULE_WED, 23, 0, 0},
	}, {
		// Similar to prior case, but the previous day was the end
		// of a span.
		.schedule="tue-wed,23:00-1:00",
		.last={2020, 1,  8, SCHEDULE_WED, 8, 30, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 0, 45, 0},
		.next={2020, 1,  8, SCHEDULE_WED, 23, 0, 0},
	}, {
		// Across the year to first monday.
		.schedule="mon1,8:00",
		.last={2019, 12, 27, SCHEDULE_FRI, 0,  0, 0},
		.curr={2019, 12, 27, SCHEDULE_FRI, 8, 45, 0},
		.next={2020,  1,  6, SCHEDULE_MON, 8,  0, 0},
	}, {
		// Multi-week cross-year span, now inside week span.
		.schedule="fri3-mon1,8:00",
		.last={2019, 12, 27, SCHEDULE_FRI, 0,  0, 0},
		.curr={2019, 12, 27, SCHEDULE_FRI, 8, 45, 0},
		.next={2019, 12, 28, SCHEDULE_SAT, 8,  0, 0},
	}, {
		// Multi-week span, now at edge of week span.
		.schedule="mon2-fri5,8:00",
		.last={2019, 12, 27, SCHEDULE_FRI, 0,  0, 0},
		.curr={2019, 12, 27, SCHEDULE_FRI, 8, 45, 0},
		.next={2020,  1, 13, SCHEDULE_MON, 8,  0, 0},
	},

	// === Months ===
	{
		// Whole current month, today.
		.schedule="jan",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1,  9, SCHEDULE_THU, 0,  0, 0},
	}, {
		// Whole other month, pick first day.
		.schedule="feb",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 2,  1, SCHEDULE_SAT, 0,  0, 0},
	}, {
		// Current month, skip to right day.
		.schedule="jan,13",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 1, 13, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Same with two days, last on first.
		.schedule="jan,13,15",
		.last={2020, 1, 13, SCHEDULE_MON, 0,  0, 0},
		.curr={2020, 1, 13, SCHEDULE_MON, 8, 45, 0},
		.next={2020, 1, 15, SCHEDULE_WED, 0,  0, 0},
	}, {
		// Different month, shift month and day.
		.schedule="mar,17",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 3, 17, SCHEDULE_TUE, 0,  0, 0},
	}, {
		// Friday, inside specific range.
		.schedule="feb,11-15,fri",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 2, 14, SCHEDULE_FRI, 0,  0, 0},
	}, {
		// Friday on the 12th needs to skip year.
		.schedule="feb,11-13,fri",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2021, 2, 12, SCHEDULE_FRI, 0,  0, 0},
	}, {
		// Weekday position inside range.
		.schedule="feb,4-29,mar,1-15,mon1",
		.last={2020, 1,  1, SCHEDULE_WED, 0,  0, 0},
		.curr={2020, 1,  9, SCHEDULE_THU, 8, 45, 0},
		.next={2020, 3,  2, SCHEDULE_MON, 0,  0, 0},
	}, {
		// Weekday position inside range, last inside first span.
		.schedule="mar,1-15,mon1,wed1,9:00-10:00/2",
		.last={2020, 3,  2, SCHEDULE_MON, 9,  0, 0},
		.curr={2020, 3,  2, SCHEDULE_MON, 9, 20, 0},
		.next={2020, 3,  2, SCHEDULE_MON, 9, 30, 0},
	}, {
		// Same, but last in second day skips year.
		.schedule="mar,1-15,mon1,wed1,9:00-10:00",
		.last={2020, 3,  4, SCHEDULE_WED, 9,  0, 0},
		.curr={2020, 3,  4, SCHEDULE_WED, 9, 20, 0},
		.next={2021, 3,  1, SCHEDULE_MON, 9,  0, 0},
	}, {
		// Across the year, with match in prior year.
		.schedule="dec,31,23:55",
		.last={2019, 12, 31, SCHEDULE_TUE,  0,  0, 0},
		.curr={2020,  1,  1, SCHEDULE_WED,  0,  5, 0},
		.next={2019, 12, 31, SCHEDULE_TUE, 23, 55, 0},
	}, {
		// Same, with range.
		.schedule="jan-dec,1-31,23:55",
		.last={2019, 12, 31, SCHEDULE_TUE,  0,  0, 0},
		.curr={2020,  1,  1, SCHEDULE_WED,  0,  5, 0},
		.next={2019, 12, 31, SCHEDULE_TUE, 23, 55, 0},
	}, {
		// Same, with weekday.
		.schedule="dec,tue5,23:55",
		.last={2019, 12, 31, SCHEDULE_TUE,  0,  0, 0},
		.curr={2020,  1,  1, SCHEDULE_WED,  0,  5, 0},
		.next={2019, 12, 31, SCHEDULE_TUE, 23, 55, 0},
	}, {
		// Inverted week span, skipping across month.
		.schedule="jan,feb,5,mon-fri1,8:00",
		.last={2020, 1, 3, SCHEDULE_FRI, 0,  0, 0},
		.curr={2020, 1, 3, SCHEDULE_FRI, 8, 45, 0},
		.next={2020, 2, 5, SCHEDULE_WED, 8,  0, 0},
	},
	// === End ===

	};

	for (int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		__auto_type test = &tests[i];

		ESP_LOGV(TEST_TAG, "====== %s ======", test->schedule);

		struct tm last = {
			.tm_year  = test->last.year-1900,
			.tm_mon   = test->last.month-1,
			.tm_mday  = test->last.day,
			.tm_wday  = test->last.weekday,
			.tm_hour  = test->last.hour,
			.tm_min   = test->last.minute,
			.tm_sec   = test->last.second,
			.tm_isdst = -1,
		};
		struct tm curr = {
			.tm_year  = test->curr.year-1900,
			.tm_mon   = test->curr.month-1,
			.tm_mday  = test->curr.day,
			.tm_wday  = test->curr.weekday,
			.tm_hour  = test->curr.hour,
			.tm_min   = test->curr.minute,
			.tm_sec   = test->curr.second,
			.tm_isdst = -1,
		};

		time_t last_time, curr_time, next_time; 
		last_time = mktime(&last);
		curr_time = mktime(&curr);

		TEST_ASSERT_GREATER_THAN(0, last_time);
		TEST_ASSERT_GREATER_THAN(0, curr_time);

		schedule_t sched;
		bool res = schedule_parse(&sched, test->schedule);
		TEST_ASSERT_TRUE(res);

		schedule_next(&sched, last_time, curr_time, &next_time);

		struct tm next;
		localtime_r(&next_time, &next);

		char msg[256];
		sprintf(msg, "Schedule: %s", test->schedule);

		TEST_ASSERT_EQUAL_MESSAGE(test->next.year,    next.tm_year+1900, msg);
		TEST_ASSERT_EQUAL_MESSAGE(test->next.month,   next.tm_mon+1, msg);
		TEST_ASSERT_EQUAL_MESSAGE(test->next.day,     next.tm_mday, msg);
		TEST_ASSERT_EQUAL_MESSAGE(test->next.weekday, next.tm_wday, msg);
		TEST_ASSERT_EQUAL_MESSAGE(test->next.hour,    next.tm_hour, msg);
		TEST_ASSERT_EQUAL_MESSAGE(test->next.minute,  next.tm_min, msg);
		TEST_ASSERT_EQUAL_MESSAGE(test->next.second,  next.tm_sec, msg);
	}
}

TEST_CASE("Schedule ordering", "[schedule]")
{
	struct test_case {
		const char *schedule;
		uint32_t   order;
		int        ydays;
		int        wdays;
		int        mins;
	};

	struct test_case tests[] = {{
		.schedule="jan-dec",
		.order=0x16EE5A0,
		.ydays=366,
		.wdays=28,
		.mins=1440,
	}, {
		.schedule="oct,jan-aug,sep,nov-dec",
		.order=0x16EE5A0,
		.ydays=366,
		.wdays=28,
		.mins=1440,
	}, {
		.schedule="jan,feb-mar,,may-dec",
		.order=0x150E5A0,
		.ydays=336,
		.wdays=28,
		.mins=1440,
	}, {
		.schedule="may-mar",
		.order=0x150E5A0,
		.ydays=336,
		.wdays=28,
		.mins=1440,
	}, {
		.schedule="sun1-sat5",
		.order=0x16EE5A0,
		.ydays=366,
		.wdays=28,
		.mins=1440,
	}, {
		.schedule="sun-sat",
		.order=0x16EE5A0,
		.ydays=366,
		.wdays=28,
		.mins=1440,
	}, {
		.schedule="mon",
		.order=0x16E25A0,
		.ydays=366,
		.wdays=4,
		.mins=1440,
	}, {
		.schedule="mon-fri",
		.order=0x16EA5A0,
		.ydays=366,
		.wdays=20,
		.mins=1440,
	}, {
		.schedule="fri-mon",
		.order=0x16E85A0,
		.ydays=366,
		.wdays=16,
		.mins=1440,
	}, {
		.schedule="mon2-fri",
		.order=0x16E2DA0,
		.ydays=366,
		.wdays=5,
		.mins=1440,
	}, {
		.schedule="mon-fri2",
		.order=0x16E2DA0,
		.ydays=366,
		.wdays=5,
		.mins=1440,
	}, {
		.schedule="fri2-mon",
		.order=0x16E25A0,
		.ydays=366,
		.wdays=4,
		.mins=1440,
	}, {
		.schedule="fri-mon2",
		.order=0x16E25A0,
		.ydays=366,
		.wdays=4,
		.mins=1440,
	}, {
		.schedule="sun3-sat2",
		.order=0x16EE5A0,
		.ydays=366,
		.wdays=28,
		.mins=1440,
	}, {
		.schedule="mon3-fri2",
		.order=0x16ED5A0,
		.ydays=366,
		.wdays=26,
		.mins=1440,
	}, {
		.schedule="mon,tue,,thu-fri",
		.order=0x16E85A0,
		.ydays=366,
		.wdays=16,
		.mins=1440,
	}, {
		.schedule="13:00",
		.order=0x16EE00F,
		.ydays=366,
		.wdays=28,
		.mins=15,
	}, {
		.schedule="13:00-15:00",
		.order=0x16EE078,
		.ydays=366,
		.wdays=28,
		.mins=120,
	}, {
		.schedule="23:00-01:00",
		.order=0x16EE078,
		.ydays=366,
		.wdays=28,
		.mins=120,
	}, {
		.schedule="23:30-01:30",
		.order=0x16EE078,
		.ydays=366,
		.wdays=28,
		.mins=120,
	}, {
		.schedule="13:00-15:00,14:00-16:00",
		.order=0x16EE0B4,
		.ydays=366,
		.wdays=28,
		.mins=180,
	}, {
		.schedule="13:00-14:00,15:00-16:00,,17:30-18:30",
		.order=0x16EE0B4,
		.ydays=366,
		.wdays=28,
		.mins=180,
	}};

	schedule_t sched;
	char msg[256];
	for (int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		__auto_type test = &tests[i];

		// https://play.golang.org/p/5JV_qo9ge46

		sprintf(msg, "schedule=%s\n", test->schedule);

		TEST_ASSERT_TRUE_MESSAGE(schedule_parse(&sched, test->schedule), msg);

		TEST_ASSERT_EQUAL_MESSAGE(test->ydays, (sched.order >> 16) & ((1 <<  9)-1), msg);
		TEST_ASSERT_EQUAL_MESSAGE(test->wdays, (sched.order >> 11) & ((1 <<  5)-1), msg);
		TEST_ASSERT_EQUAL_MESSAGE(test->mins, (sched.order >>  0) & ((1 << 11)-1), msg);

		TEST_ASSERT_EQUAL_MESSAGE(test->order, sched.order, msg);
	}
}
