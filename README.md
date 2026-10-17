# Schedule Syntax

This repository is an excerpt from a little ESP32 firmware I created years ago to drive remote automation inside the family motorhome for several years.

Some of this syntax was later ported over to control the refrehses
in snapd ([link](https://forum.snapcraft.io/t/refresh-scheduling-on-specific-days-of-the-month/1239/6)).

Here I'm putting both the original syntax and the code in public
domain. You can do whatever you want with it.


## Basics

A schedule string selects **when** events happen by combining three dimensions: days of the year, weekdays, and times of the day.

- [`schedule.h`](schedule.h) - public API
- [`schedule.c`](schedule.c) - parser and next-occurrence search
- [`schedule_test.c`](schedule_test.c) - tests, full of syntax examples

## API

```c
schedule_t sched;
schedule_parse(&sched, "mon-fri,8:00-17:00");

time_t next;
if (schedule_next(&sched, last_event_time, now, &next)) {
    // next holds the time of the next event at or after now
}
```

## Overview

A schedule is one or more **sets**, separated by a double comma:

```
<set>,,<set>,,<set>
```

Within a set, elements are separated by a single comma. Each element is
recognized by its shape, so they may appear in any order:

```
jan,10-15,mon-wed2,09:45,13:00-17:00/5
```

Anything omitted falls back to a sensible default:

| Dimension | Default | Meaning |
| --- | --- | --- |
| Months/days | all year | every day of every month |
| Weekdays | `sun-sat` | every weekday |
| Time | `0:00-0:00` | the whole day (24h) |

A double comma. e.g. `mon,,tue`, ends a set - anything after it belongs to a
new set. Up to 8 sets, 8 weekday spans and 8 clock spans per set are supported.

## Days of the year - months and days

Months are three-letter names, and may form ranges that wrap around the year:

- **`dec-mar`** - December through March
- **`jan,mar,may`** - January, March and May individually
- **`dec`** - Single month

Days of the month (1–31) may follow the months they apply to, either as single
days or ranges, again with wrap-around:

- **`dec-mar,5-10`** - Days 5–10 of each month from December through March
- **`jan,15-20`** - January 15th through 20th
- **`jan,1-31,feb`** - Whole January and February
- **`mar,29-31,apr,1-2`** - Wrapping range: March 29 through April 2

If no days are given for a month, **all** its days are selected.

## Weekdays

Weekdays are `sun`, `mon`, `tue`, `wed`, `thu`, `fri`, `sat`, and
may form ranges. Ranges wrap around the week, so `fri-mon` is Friday, Saturday,
Sunday and Monday:

- **`mon`** - Every Monday
- **`mon-fri`** - Monday through Friday
- **`fri-mon`** - Friday through Monday
- **`sat,sun`** - Weekends

A digit after a weekday restricts it to a given position within the month:

- **`mon3`** - Third Monday of the month
- **`sun1`** - First Sunday of the month
- **`mon3-sun`** - Third Monday through the following Sunday
- **`mon-sun4`** - Monday through the fourth Sunday

If no digit is given, any position within the month matches.

## Times of day

Clock values are `H:MM` (or `HH:MM`), with `24:00` accepted as midnight:

- **`13:00`** - One o'clock in the afternoon
- **`09:45`** - Quarter to ten in the morning
- **`24:00`** - Midnight (same as `0:00`)

A single time is shorthand for a 15-minute span, which may cross midnight:

- **`09:45`** - The span from 09:45 to 10:00
- **`23:55`** - The span from 23:55 to 00:10 on the next day

An explicit range is written with `-` and may cross midnight:

- **`8:00-17:00`** - From 08:00 through 17:00
- **`23:00-1:00`** - From 23:00 through 01:00 on the next day
- **`0:00-0:00`** - The whole day (24 hours)

### Splitting

A clock span may be subdivided into repeating triggers by appending `/N`,
where `N` is 1–99999, optionally followed by a unit:

- **`8:00-10:00/3`** - Three equal parts (40 minutes each)
- **`8:00-10:00/5`** - Five equal parts (24 minutes each)
- **`8:00-10:00/30s`** - Every 30 seconds
- **`8:00-10:00/15m`** - Every 15 minutes
- **`8:00-10:00/1h`** - Every hour

With no unit, `N` is the number of parts the span is divided into. With a
unit (`s`, `m`, `h`), `N` is the period between triggers in that unit.

## Examples

- **`mon-fri,8:00-17:00,,sat,sun,10:00,,mon1,9:00`** - Workdays 8–5, weekends at 10, and first Monday of each month at 9, as three separate sets
- **`dec-mar,1-14,sat`** - Days 1–14 of December through March, Saturdays only (all day)
- **`mon,8:00-10:00/30s`** - Every Monday, every 30 seconds between 8:00 and 10:00
- **`dec,31,23:55`** - New Year's Eve at 23:55 (the implied 15-minute span crosses midnight)
