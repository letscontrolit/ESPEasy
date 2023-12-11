/*
 * Copyright 2015, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * File:   CronExprParser_test.cpp
 * Author: alex
 *
 * Created on February 24, 2015, 9:36 AM
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "ccronexpr.h"

#define MAX_SECONDS 62
#define CRON_MAX_MINUTES 60
#define CRON_MAX_HOURS 24
#define CRON_MAX_DAYS_OF_WEEK 8
#define CRON_MAX_DAYS_OF_MONTH 32
#define CRON_MAX_MONTHS 12

#define DATE_FORMAT "%Y-%m-%d_%H:%M:%S"

#ifndef ARRAY_LEN
#define ARRAY_LEN(x) sizeof(x)/sizeof(x[0])
#endif

/* declared in cronexpr.c */
time_t cron_mktime(struct tm* tm);

/**
 * uint8_t* replace char* for storing hit dates, set_bit and get_bit are used as handlers
 */
uint8_t cron_get_bit(const uint8_t* rbyte, int idx);
void cron_set_bit(uint8_t* rbyte, int idx);
void cron_del_bit(uint8_t* rbyte, int idx);

static int crons_equal(cron_expr* cr1, cron_expr* cr2) {
    unsigned int i;
    for (i = 0; i < ARRAY_LEN(cr1->seconds); i++) {
        if (cr1->seconds[i] != cr2->seconds[i]) {
            printf("seconds not equal @%d %02x != %02x", i, cr1->seconds[i], cr2->seconds[i]);
            return 0;
        }
    }
    for (i = 0; i < ARRAY_LEN(cr1->minutes); i++) {
        if (cr1->minutes[i] != cr2->minutes[i]) {
            printf("minutes not equal @%d %02x != %02x", i, cr1->minutes[i], cr2->minutes[i]);
            return 0;
        }
    }
    for (i = 0; i < ARRAY_LEN(cr1->hours); i++) {
        if (cr1->hours[i] != cr2->hours[i]) {
            printf("hours not equal @%d %02x != %02x", i, cr1->hours[i], cr2->hours[i]);
            return 0;
        }
    }
    for (i = 0; i < ARRAY_LEN(cr1->days_of_week); i++) {
        if (cr1->days_of_week[i] != cr2->days_of_week[i]) {
            printf("days_of_week not equal @%d %02x != %02x", i, cr1->days_of_week[i], cr2->days_of_week[i]);
            return 0;
        }
    }
    for (i = 0; i < ARRAY_LEN(cr1->days_of_month); i++) {
        if (cr1->days_of_month[i] != cr2->days_of_month[i]) {
            printf("days_of_month not equal @%d %02x != %02x", i, cr1->days_of_month[i], cr2->days_of_month[i]);
            return 0;
        }
    }
    for (i = 0; i < ARRAY_LEN(cr1->months); i++) {
        if (cr1->months[i] != cr2->months[i]) {
            printf("months not equal @%d %02x != %02x", i, cr1->months[i], cr2->months[i]);
            return 0;
        }
    }
    return 1;
}

int one_dec_num(const char ch) {
    switch (ch) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    default:
        return -1;
    }
}

int extract_digits(const char *str, int *digit_count) {
    int num = 0;
    if (!str || !digit_count) return 0;
    while (*str) {
        (*digit_count)++;
        if (*str >= '0' && *str <= '9') {
            num = num * 10 + (*str - '0');
        } else break;
        str++;
    }
    return num;
}

/* strptime is not available in msvc */
/* 2012-07-01_09:53:50 */
/* 0123456789012345678 */
void poors_mans_strptime(const char* str, struct tm* cal) {
    int count = 0;
    assert(cal != NULL);
    memset(cal, 0, sizeof(struct tm));
    cal->tm_year = extract_digits(str, &count) - 1900;
    cal->tm_mon = extract_digits(str + count, &count) - 1;
    cal->tm_mday = extract_digits(str + count, &count);
    cal->tm_wday = 0;
    cal->tm_yday = 0;
    cal->tm_hour = extract_digits(str + count, &count);
    cal->tm_min = extract_digits(str + count, &count);
    cal->tm_sec = extract_digits(str + count, &count);
    cal->tm_isdst = -1;
}

typedef time_t (*cron_find_fn)(cron_expr*, time_t);

int count_fields(const char* str, char del) {
    size_t count = 0;
    if (!str) return -1;
    while ((str = strchr(str, del)) != NULL) {
        count++;
        do str++; while (del == *str);
    }
    return (int)count + 1;
}

#define check_fn(fn_fn,pattern,initial,expected) check_fn_line(fn_fn, pattern, initial, expected, __LINE__)
void check_fn_line(cron_find_fn fn, const char* pattern, const char* initial, const char* expected, int line) {
    const char* err = NULL;
    const int len = count_fields(pattern, ' ');
    cron_expr parsed1, parsed2;
    /*printf("Pattern: %s\n", pattern);**/
    cron_parse_expr(pattern, &parsed1, &err);

    struct tm calinit;
    poors_mans_strptime(initial, &calinit);
    time_t dateinit = cron_mktime(&calinit);
    assert(-1 != dateinit);
    time_t datenext = fn(&parsed1, dateinit);
#ifdef CRON_USE_LOCAL_TIME
    struct tm* calnext = localtime(&datenext);
#else
    struct tm* calnext = gmtime(&datenext);
#endif
    assert(calnext);
    char* buffer = (char*) malloc(512);
    memset(buffer, 0, 512);
    strftime(buffer, 512, DATE_FORMAT, calnext);
    printf("parsed: %s\n", pattern);
    if (0 != strcmp(expected, buffer+(buffer[0] == '+' ? 1 : 0))) {
        printf("Line: %d\n", line);
        printf("Pattern: %s\n", pattern);
        printf("Initial: %s\n", initial);
        printf("Expected: %s\n", expected);
        printf("Actual: %s\n", buffer);
        assert(0);
    }
    assert(cron_generate_expr(&parsed1, buffer, 512, len, &err) > 0);
    if (0 != strcmp(pattern, buffer)) {
        printf("Line: %d\n", line);
        printf("Pattern: %s\n", pattern);
        printf("Actual:  %s\n", buffer);
    }
    cron_parse_expr(buffer, &parsed2, &err);
    assert(crons_equal(&parsed1, &parsed2));
    free(buffer);
}

void check_same(const char* expr1, const char* expr2) {
    cron_expr parsed1;
    cron_parse_expr(expr1, &parsed1, NULL);
    cron_expr parsed2;
    cron_parse_expr(expr2, &parsed2, NULL);
    printf("parsed1: %s\n", expr1);
    printf("parsed2: %s\n", expr2);
    assert(crons_equal(&parsed1, &parsed2));
}

void check_calc_invalid() {
    cron_expr parsed;
    cron_parse_expr("0 0 0 31 6 *", &parsed, NULL);
    struct tm calinit;
    poors_mans_strptime("2012-07-01_09:53:50", &calinit);
    time_t dateinit = cron_mktime(&calinit);
    time_t res = cron_next(&parsed, dateinit);
    assert(CRON_INVALID_INSTANT == res);
}

void check_expr_invalid(const char* pattern) {
    const char* err = NULL;
    const int len = count_fields(pattern, ' ');
    cron_expr parsed1, parsed2;
    cron_parse_expr(pattern, &parsed1, &err);
    printf("parsed1: %s\n", pattern);
    if (err) {
        printf("check_expr_invalid: %s\n", err);
    }
    assert(err);
}

#define check_expr_valid(pattern) check_expr_valid_line(pattern, __LINE__)
void check_expr_valid_line(const char* pattern, int line) {
    const char* err = NULL;
    const int len = count_fields(pattern, ' ');
    cron_expr parsed1, parsed2;
    cron_parse_expr(pattern, &parsed1, &err);
    printf("parsed1: %s\n", pattern);
    if (err) {
        printf("check_expr_invalid: %s\n", err);
    }
    char* buffer = (char*) malloc(512);
    memset(buffer, 0, 512);
    assert(cron_generate_expr(&parsed1, buffer, 512, len, &err) > 0);
    if (0 != strcmp(pattern, buffer)) {
        printf("Line: %d\n", line);
        printf("Pattern: %s\n", pattern);
        printf("Actual:  %s\n", buffer);
    }
    cron_parse_expr(buffer, &parsed2, &err);
    assert(crons_equal(&parsed1, &parsed2));

    assert(!err);
}

void test_expr() {
    char* tz = getenv("TZ");
    /*Test leap seconds - nejsou nastavené hodnoty, co se kontrolují */ 
    /*Test leap seconds
    check_fn(cron_next, "60 0 0 * * *", "2015-01-01_15:12:42", "2015-06-30_00:00:00");*/
    check_fn(cron_next, "* * * * * *", "2100-01-01_15:12:42", "2100-01-01_15:12:43");
    check_fn(cron_next, "* * * * * *", "2198-01-01_15:12:42", "2198-01-01_15:12:43");
    check_fn(cron_next, "* * * * * *", "2199-01-01_15:12:42", "2199-01-01_15:12:43");
    if (tz && !strcmp("right/UTC", tz)) {
        check_fn(cron_next, "L59 * * * * *", "2016-12-31_23:50:00", "2016-12-31_23:50:59");
        check_fn(cron_next, "L60 * * * * *", "2016-12-31_23:50:00", "2016-12-31_23:59:60");
        check_fn(cron_next, "L60 * * * * *", "2016-12-31_23:59:59", "2016-12-31_23:59:60");
    }

    check_fn(cron_next, "* * * 1 1 * *", "1970-01-01_15:12:42", "1970-01-01_15:12:43");

#ifndef CRON_DISABLE_YEARS
    check_fn(cron_next, "* * * 1 1 * 1970,2100,2193,2199", "1970-01-01_15:12:42", "1970-01-01_15:12:43");
    /*check_fn(cron_next, "* * * 1 1 * 1969,2100,2193,2199", "1969-01-01_15:12:42", "1969-01-01_15:12:43");*/
    check_fn(cron_next, "* * * 1 1 * 1970,2100,2193,2199", "1971-01-01_15:12:42", "2100-01-01_00:00:00");
    check_fn(cron_next, "* * * 1 1 * 1970,2100,2193,2199", "2195-01-01_15:12:42", "2199-01-01_00:00:00");
    /*check_fn(cron_next, "* * * 1 1 * 1970,2100,2193,2200", "2195-01-01_15:12:42", "2200-01-01_00:00:00");*/
    check_fn(cron_next, "* * * 1 1 * 2020", "2011-01-01_15:12:42", "2020-01-01_00:00:00");
#endif

    check_fn(cron_next, "* * * 1W * *", "2011-01-01_15:12:42", "2011-01-03_00:00:00");
    check_fn(cron_next, "* * * 31W * *", "2010-01-01_15:12:42", "2010-01-29_00:00:00");

    check_fn(cron_next, "* * * LW * *", "2010-01-01_15:12:42", "2010-01-29_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-02-03_15:12:42", "2010-02-26_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-03-06_15:12:42", "2010-03-31_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-04-09_15:12:42", "2010-04-30_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-05-12_15:12:42", "2010-05-31_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-06-15_15:12:42", "2010-06-30_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-07-18_15:12:42", "2010-07-30_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-08-21_15:12:42", "2010-08-31_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-09-24_15:12:42", "2010-09-30_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-10-27_15:12:42", "2010-10-29_00:00:00");
    check_fn(cron_next, "* * * LW * *", "2010-11-30_15:12:42", "2010-11-30_15:12:43");
    check_fn(cron_next, "* * * LW * *", "2010-12-31_15:12:42", "2010-12-31_15:12:43");

    check_fn(cron_next, "* * * 15W * *", "2010-01-01_15:12:42", "2010-01-15_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-02-03_15:12:42", "2010-02-15_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-03-06_15:12:42", "2010-03-15_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-04-09_15:12:42", "2010-04-15_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-05-12_15:12:42", "2010-05-14_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-06-15_15:12:42", "2010-06-15_15:12:43");
    check_fn(cron_next, "* * * 15W * *", "2010-07-18_15:12:42", "2010-08-16_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-08-21_15:12:42", "2010-09-15_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-09-24_15:12:42", "2010-10-15_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-10-27_15:12:42", "2010-11-15_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-11-30_15:12:42", "2010-12-15_00:00:00");
    check_fn(cron_next, "* * * 15W * *", "2010-12-31_15:12:42", "2011-01-14_00:00:00");

    check_fn(cron_next, "* * * L-1 * *", "2010-01-01_15:12:42", "2010-01-30_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-02-03_15:12:42", "2010-02-27_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-03-06_15:12:42", "2010-03-30_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-04-09_15:12:42", "2010-04-29_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-05-12_15:12:42", "2010-05-30_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-06-15_15:12:42", "2010-06-29_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-07-18_15:12:42", "2010-07-30_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-08-21_15:12:42", "2010-08-30_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-09-24_15:12:42", "2010-09-29_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-10-27_15:12:42", "2010-10-30_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-11-30_15:12:42", "2010-12-30_00:00:00");
    check_fn(cron_next, "* * * L-1 * *", "2010-12-31_15:12:42", "2011-01-30_00:00:00");

    check_fn(cron_next, "* * * L * *", "2010-01-01_15:12:42", "2010-01-31_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-02-03_15:12:42", "2010-02-28_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-03-06_15:12:42", "2010-03-31_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-04-09_15:12:42", "2010-04-30_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-05-12_15:12:42", "2010-05-31_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-06-15_15:12:42", "2010-06-30_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-07-18_15:12:42", "2010-07-31_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-08-21_15:12:42", "2010-08-31_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-09-24_15:12:42", "2010-09-30_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-10-27_15:12:42", "2010-10-31_00:00:00");
    check_fn(cron_next, "* * * L * *", "2010-11-30_15:12:42", "2010-11-30_15:12:43");
    check_fn(cron_next, "* * * L * *", "2010-12-31_15:12:42", "2010-12-31_15:12:43");

    check_fn(cron_next, "* * * * * 1#-5", "2010-09-03_15:12:42", "2010-11-01_00:00:00");
    check_fn(cron_next, "* * * * * 1#-4", "2010-09-03_15:12:42", "2010-09-06_00:00:00");
    check_fn(cron_next, "* * * * * 1#-3", "2010-09-03_15:12:42", "2010-09-13_00:00:00");
    check_fn(cron_next, "* * * * * 1#-2", "2010-09-03_15:12:42", "2010-09-20_00:00:00");
    check_fn(cron_next, "* * * * * 1#1", "2010-09-03_15:12:42", "2010-09-06_00:00:00");
    check_fn(cron_next, "* * * * * 1#2", "2010-09-03_15:12:42", "2010-09-13_00:00:00");
    check_fn(cron_next, "* * * * * 1#3", "2010-09-03_15:12:42", "2010-09-20_00:00:00");
    check_fn(cron_next, "* * * * * 1#4", "2010-09-03_15:12:42", "2010-09-27_00:00:00");
    check_fn(cron_next, "* * * * * 1#5", "2010-09-03_15:12:42", "2010-11-29_00:00:00");

    check_fn(cron_next, "* * * * * 2#-5", "2010-09-03_15:12:42", "2010-11-02_00:00:00");
    check_fn(cron_next, "* * * * * 2#-4", "2010-09-03_15:12:42", "2010-09-07_00:00:00");
    check_fn(cron_next, "* * * * * 2#-3", "2010-09-03_15:12:42", "2010-09-14_00:00:00");
    check_fn(cron_next, "* * * * * 2#-2", "2010-09-03_15:12:42", "2010-09-21_00:00:00");
    check_fn(cron_next, "* * * * * 2#1", "2010-09-03_15:12:42", "2010-09-07_00:00:00");
    check_fn(cron_next, "* * * * * 2#2", "2010-09-03_15:12:42", "2010-09-14_00:00:00");
    check_fn(cron_next, "* * * * * 2#3", "2010-09-03_15:12:42", "2010-09-21_00:00:00");
    check_fn(cron_next, "* * * * * 2#4", "2010-09-03_15:12:42", "2010-09-28_00:00:00");
    check_fn(cron_next, "* * * * * 2#5", "2010-09-03_15:12:42", "2010-11-30_00:00:00");

    check_fn(cron_next, "* * * * * 3#-5", "2010-09-03_15:12:42", "2010-12-01_00:00:00");
    check_fn(cron_next, "* * * * * 3#-4", "2010-09-03_15:12:42", "2010-09-08_00:00:00");
    check_fn(cron_next, "* * * * * 3#-3", "2010-09-03_15:12:42", "2010-09-15_00:00:00");
    check_fn(cron_next, "* * * * * 3#-2", "2010-09-03_15:12:42", "2010-09-22_00:00:00");
    check_fn(cron_next, "* * * * * 3#1", "2010-09-03_15:12:42", "2010-10-06_00:00:00");
    check_fn(cron_next, "* * * * * 3#2", "2010-09-03_15:12:42", "2010-09-08_00:00:00");
    check_fn(cron_next, "* * * * * 3#3", "2010-09-03_15:12:42", "2010-09-15_00:00:00");
    check_fn(cron_next, "* * * * * 3#4", "2010-09-03_15:12:42", "2010-09-22_00:00:00");
    check_fn(cron_next, "* * * * * 3#5", "2010-09-03_15:12:42", "2010-09-29_00:00:00");

    check_fn(cron_next, "* * * * * 4#-5", "2010-09-03_15:12:42", "2010-12-02_00:00:00");
    check_fn(cron_next, "* * * * * 4#-4", "2010-09-03_15:12:42", "2010-09-09_00:00:00");
    check_fn(cron_next, "* * * * * 4#-3", "2010-09-03_15:12:42", "2010-09-16_00:00:00");
    check_fn(cron_next, "* * * * * 4#-2", "2010-09-03_15:12:42", "2010-09-23_00:00:00");
    check_fn(cron_next, "* * * * * 4#1", "2010-09-03_15:12:42", "2010-10-07_00:00:00");
    check_fn(cron_next, "* * * * * 4#2", "2010-09-03_15:12:42", "2010-09-09_00:00:00");
    check_fn(cron_next, "* * * * * 4#3", "2010-09-03_15:12:42", "2010-09-16_00:00:00");
    check_fn(cron_next, "* * * * * 4#4", "2010-09-03_15:12:42", "2010-09-23_00:00:00");
    check_fn(cron_next, "* * * * * 4#5", "2010-09-03_15:12:42", "2010-09-30_00:00:00");

    check_fn(cron_next, "* * * * * 5#-5", "2010-09-03_15:12:42", "2010-10-01_00:00:00");
    check_fn(cron_next, "* * * * * 5#-4", "2010-09-03_15:12:42", "2010-09-03_15:12:43");
    check_fn(cron_next, "* * * * * 5#-3", "2010-09-03_15:12:42", "2010-09-10_00:00:00");
    check_fn(cron_next, "* * * * * 5#-2", "2010-09-03_15:12:42", "2010-09-17_00:00:00");
    check_fn(cron_next, "* * * * * 5#1", "2010-09-03_15:12:42", "2010-09-03_15:12:43");
    check_fn(cron_next, "* * * * * 5#2", "2010-09-03_15:12:42", "2010-09-10_00:00:00");
    check_fn(cron_next, "* * * * * 5#3", "2010-09-03_15:12:42", "2010-09-17_00:00:00");
    check_fn(cron_next, "* * * * * 5#4", "2010-09-03_15:12:42", "2010-09-24_00:00:00");
    check_fn(cron_next, "* * * * * 5#5", "2010-09-03_15:12:42", "2010-10-29_00:00:00");

    check_fn(cron_next, "* * * * * 6#-5", "2010-09-03_15:12:42", "2010-10-02_00:00:00");
    check_fn(cron_next, "* * * * * 6#-4", "2010-09-03_15:12:42", "2010-09-04_00:00:00");
    check_fn(cron_next, "* * * * * 6#-3", "2010-09-03_15:12:42", "2010-09-11_00:00:00");
    check_fn(cron_next, "* * * * * 6#-2", "2010-09-03_15:12:42", "2010-09-18_00:00:00");
    check_fn(cron_next, "* * * * * 6#1", "2010-09-03_15:12:42", "2010-09-04_00:00:00");
    check_fn(cron_next, "* * * * * 6#2", "2010-09-03_15:12:42", "2010-09-11_00:00:00");
    check_fn(cron_next, "* * * * * 6#3", "2010-09-03_15:12:42", "2010-09-18_00:00:00");
    check_fn(cron_next, "* * * * * 6#4", "2010-09-03_15:12:42", "2010-09-25_00:00:00");
    check_fn(cron_next, "* * * * * 6#5", "2010-09-03_15:12:42", "2010-10-30_00:00:00");

    check_fn(cron_next, "* * * * * 7#-5", "2010-09-03_15:12:42", "2010-10-03_00:00:00");
    check_fn(cron_next, "* * * * * 7#-4", "2010-09-03_15:12:42", "2010-09-05_00:00:00");
    check_fn(cron_next, "* * * * * 7#-3", "2010-09-03_15:12:42", "2010-09-12_00:00:00");
    check_fn(cron_next, "* * * * * 7#-2", "2010-09-03_15:12:42", "2010-09-19_00:00:00");
    check_fn(cron_next, "* * * * * 7#1", "2010-09-03_15:12:42", "2010-09-05_00:00:00");
    check_fn(cron_next, "* * * * * 7#2", "2010-09-03_15:12:42", "2010-09-12_00:00:00");
    check_fn(cron_next, "* * * * * 7#3", "2010-09-03_15:12:42", "2010-09-19_00:00:00");
    check_fn(cron_next, "* * * * * 7#4", "2010-09-03_15:12:42", "2010-09-26_00:00:00");
    check_fn(cron_next, "* * * * * 7#5", "2010-09-03_15:12:42", "2010-10-31_00:00:00");

    check_fn(cron_next, "* * * * * 1L", "2010-09-30_15:12:42", "2010-10-25_00:00:00");
    check_fn(cron_next, "* * * * * 2L", "2010-09-30_15:12:42", "2010-10-26_00:00:00");
    check_fn(cron_next, "* * * * * 3L", "2010-09-30_15:12:42", "2010-10-27_00:00:00");
    check_fn(cron_next, "* * * * * 4L", "2010-09-30_15:12:42", "2010-09-30_15:12:43");
    check_fn(cron_next, "* * * * * 5L", "2010-09-30_15:12:42", "2010-10-29_00:00:00");
    check_fn(cron_next, "* * * * * 6L", "2010-09-30_15:12:42", "2010-10-30_00:00:00");
    check_fn(cron_next, "* * * * * 7L", "2010-09-30_15:12:42", "2010-10-31_00:00:00");
    check_fn(cron_next, "* * * * * 1L", "2010-10-27_15:12:42", "2010-11-29_00:00:00");
    check_fn(cron_next, "* * * * * 2L", "2010-10-27_15:12:42", "2010-11-30_00:00:00");
    check_fn(cron_next, "* * * * * 3L", "2010-10-27_15:12:42", "2010-10-27_15:12:43");
    check_fn(cron_next, "* * * * * 4L", "2010-10-27_15:12:42", "2010-10-28_00:00:00");
    check_fn(cron_next, "* * * * * 5L", "2010-10-27_15:12:42", "2010-10-29_00:00:00");
    check_fn(cron_next, "* * * * * 6L", "2010-10-27_15:12:42", "2010-10-30_00:00:00");
    check_fn(cron_next, "* * * * * 7L", "2010-10-27_15:12:42", "2010-10-31_00:00:00");

    check_fn(cron_next, "* * * * * 1L", "2010-10-30_15:12:42", "2010-11-29_00:00:00");
    check_fn(cron_next, "* * * * * 2L", "2010-10-30_15:12:42", "2010-11-30_00:00:00");
    check_fn(cron_next, "* * * * * 3L", "2010-10-30_15:12:42", "2010-11-24_00:00:00");
    check_fn(cron_next, "* * * * * 4L", "2010-10-30_15:12:42", "2010-11-25_00:00:00");
    check_fn(cron_next, "* * * * * 5L", "2010-10-30_15:12:42", "2010-11-26_00:00:00");
    check_fn(cron_next, "* * * * * 6L", "2010-10-30_15:12:42", "2010-10-30_15:12:43");
    check_fn(cron_next, "* * * * * 7L", "2010-10-30_15:12:42", "2010-10-31_00:00:00");
    check_fn(cron_next, "* * * * * 1L", "2010-11-27_15:12:42", "2010-11-29_00:00:00");
    check_fn(cron_next, "* * * * * 2L", "2010-11-27_15:12:42", "2010-11-30_00:00:00");
    check_fn(cron_next, "* * * * * 3L", "2010-11-27_15:12:42", "2010-12-29_00:00:00");
    check_fn(cron_next, "* * * * * 4L", "2010-11-27_15:12:42", "2010-12-30_00:00:00");
    check_fn(cron_next, "* * * * * 5L", "2010-11-27_15:12:42", "2010-12-31_00:00:00");
    check_fn(cron_next, "* * * * * 6L", "2010-11-27_15:12:42", "2010-11-27_15:12:43");
    check_fn(cron_next, "* * * * * 7L", "2010-11-27_15:12:42", "2010-11-28_00:00:00");

    check_fn(cron_next, "0 0 7 W * *", "2009-09-26_00:42:55", "2009-09-28_07:00:00");
    check_fn(cron_next, "0 0 7 W * *", "2009-09-28_07:00:00", "2009-09-29_07:00:00");
    check_fn(cron_next, "* * * * * L", "2010-10-25_15:12:42", "2010-10-31_00:00:00");
    check_fn(cron_next, "* * * * * L", "2010-10-20_15:12:42", "2010-10-24_00:00:00");
    check_fn(cron_next, "* * * * * L", "2010-10-27_15:12:42", "2010-10-31_00:00:00");

    check_fn(cron_next, "* 15 11 * * *", "2019-03-09_11:43:00", "2019-03-10_11:15:00");
    check_fn(cron_next, "*/15 * 1-4 * * *", "2012-07-01_09:53:50", "2012-07-02_01:00:00");
    check_fn(cron_next, "*/15 * 1-4 * * *", "2012-07-01_09:53:00", "2012-07-02_01:00:00");
    check_fn(cron_next, "0,15,30,45 * 1,2,3,4 * * *", "2012-07-01_09:53:00", "2012-07-02_01:00:00");
    check_fn(cron_next, "0 */2 1-4 * * *", "2012-07-01_09:00:00", "2012-07-02_01:00:00");
    check_fn(cron_next, "0 */2 * * * *", "2012-07-01_09:00:00", "2012-07-01_09:02:00");
    check_fn(cron_next, "0 */2 * * * *", "2013-07-01_09:00:00", "2013-07-01_09:02:00");
    check_fn(cron_next, "0 */2 * * * *", "2018-09-14_14:24:00", "2018-09-14_14:26:00");
    check_fn(cron_next, "0 */2 * * * *", "2018-09-14_14:25:00", "2018-09-14_14:26:00");
    check_fn(cron_next, "0 */20 * * * *", "2018-09-14_14:24:00", "2018-09-14_14:40:00");
    check_fn(cron_next, "* * * * * *", "2012-07-01_09:00:00", "2012-07-01_09:00:01");
    check_fn(cron_next, "* * * * * *", "2012-12-01_09:00:58", "2012-12-01_09:00:59");
    check_fn(cron_next, "10 * * * * *", "2012-12-01_09:42:09", "2012-12-01_09:42:10");
    check_fn(cron_next, "11 * * * * *", "2012-12-01_09:42:10", "2012-12-01_09:42:11");
    check_fn(cron_next, "10 * * * * *", "2012-12-01_09:42:10", "2012-12-01_09:43:10");
    check_fn(cron_next, "10-15 * * * * *", "2012-12-01_09:42:09", "2012-12-01_09:42:10");
    check_fn(cron_next, "10-15 * * * * *", "2012-12-01_21:42:14", "2012-12-01_21:42:15");
    check_fn(cron_next, "0 * * * * *", "2012-12-01_21:10:42", "2012-12-01_21:11:00");
    check_fn(cron_next, "0 * * * * *", "2012-12-01_21:11:00", "2012-12-01_21:12:00");
    check_fn(cron_next, "0 11 * * * *", "2012-12-01_21:10:42", "2012-12-01_21:11:00");
    check_fn(cron_next, "0 10 * * * *", "2012-12-01_21:11:00", "2012-12-01_22:10:00");
    check_fn(cron_next, "0 0 * * * *", "2012-09-30_11:01:00", "2012-09-30_12:00:00");
    check_fn(cron_next, "0 0 * * * *", "2012-09-30_12:00:00", "2012-09-30_13:00:00");
    check_fn(cron_next, "0 0 * * * *", "2012-09-10_23:01:00", "2012-09-11_00:00:00");
    check_fn(cron_next, "0 0 * * * *", "2012-09-11_00:00:00", "2012-09-11_01:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2012-09-01_14:42:43", "2012-09-02_00:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2012-09-02_00:00:00", "2012-09-03_00:00:00");
    check_fn(cron_next, "* * * 10 * *", "2012-10-09_15:12:42", "2012-10-10_00:00:00");
    check_fn(cron_next, "* * * 10 * *", "2012-10-11_15:12:42", "2012-11-10_00:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2012-09-30_15:12:42", "2012-10-01_00:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2012-10-01_00:00:00", "2012-10-02_00:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2012-08-30_15:12:42", "2012-08-31_00:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2012-08-31_00:00:00", "2012-09-01_00:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2012-10-30_15:12:42", "2012-10-31_00:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2012-10-31_00:00:00", "2012-11-01_00:00:00");
    check_fn(cron_next, "0 0 0 1 * *", "2012-10-30_15:12:42", "2012-11-01_00:00:00");
    check_fn(cron_next, "0 0 0 1 * *", "2012-11-01_00:00:00", "2012-12-01_00:00:00");
    check_fn(cron_next, "0 0 0 1 * *", "2010-12-31_15:12:42", "2011-01-01_00:00:00");
    check_fn(cron_next, "0 0 0 1 * *", "2011-01-01_00:00:00", "2011-02-01_00:00:00");
    check_fn(cron_next, "0 0 0 31 * *", "2011-10-30_15:12:42", "2011-10-31_00:00:00");
    check_fn(cron_next, "0 0 0 1 * *", "2011-10-30_15:12:42", "2011-11-01_00:00:00");
    check_fn(cron_next, "* * * * * 2", "2010-10-25_15:12:42", "2010-10-26_00:00:00");
    check_fn(cron_next, "* * * * * 2", "2010-10-20_15:12:42", "2010-10-26_00:00:00");
    check_fn(cron_next, "* * * * * 2", "2010-10-27_15:12:42", "2010-11-02_00:00:00");
    check_fn(cron_next, "55 5 * * * *", "2010-10-27_15:04:54", "2010-10-27_15:05:55");
    check_fn(cron_next, "55 5 * * * *", "2010-10-27_15:05:55", "2010-10-27_16:05:55");
    check_fn(cron_next, "55 * 10 * * *", "2010-10-27_09:04:54", "2010-10-27_10:00:55");
    check_fn(cron_next, "55 * 10 * * *", "2010-10-27_10:00:55", "2010-10-27_10:01:55");
    check_fn(cron_next, "* 5 10 * * *", "2010-10-27_09:04:55", "2010-10-27_10:05:00");
    check_fn(cron_next, "* 5 10 * * *", "2010-10-27_10:05:00", "2010-10-27_10:05:01");
    check_fn(cron_next, "55 * * 3 * *", "2010-10-02_10:05:54", "2010-10-03_00:00:55");
    check_fn(cron_next, "55 * * 3 * *", "2010-10-03_00:00:55", "2010-10-03_00:01:55");
    check_fn(cron_next, "* * * 3 11 *", "2010-10-02_14:42:55", "2010-11-03_00:00:00");
    check_fn(cron_next, "* * * 3 11 *", "2010-11-03_00:00:00", "2010-11-03_00:00:01");
    check_fn(cron_next, "0 0 0 29 2 *", "2007-02-10_14:42:55", "2008-02-29_00:00:00");
    check_fn(cron_next, "0 0 0 29 2 *", "2008-02-29_00:00:00", "2012-02-29_00:00:00");
    check_fn(cron_next, "0 0 7 ? * MON-FRI", "2009-09-26_00:42:55", "2009-09-28_07:00:00");
    check_fn(cron_next, "0 0 7 ? * MON-FRI", "2009-09-28_07:00:00", "2009-09-29_07:00:00");
    check_fn(cron_next, "0 30 23 30 1/3 ?", "2010-12-30_00:00:00", "2011-01-30_23:30:00");
    check_fn(cron_next, "0 30 23 30 1/3 ?", "2011-01-30_23:30:00", "2011-04-30_23:30:00");
    check_fn(cron_next, "0 30 23 30 1/3 ?", "2011-04-30_23:30:00", "2011-07-30_23:30:00");
    check_fn(cron_next, "* * * * * *", "2020-12-31_23:59:59", "2021-01-01_00:00:00");
    check_fn(cron_next, "0 0 * * * *", "2020-02-28_23:00:00", "2020-02-29_00:00:00");
    check_fn(cron_next, "0 0 0 * * *", "2020-02-29_01:02:03", "2020-03-01_00:00:00");

    check_fn(cron_prev, "* 15 11 * * *", "2019-03-09_11:43:00", "2019-03-09_11:15:59");
    check_fn(cron_prev, "*/15 * 1-4 * * *", "2012-07-01_09:53:50", "2012-07-01_04:59:45");
    check_fn(cron_prev, "*/15 * 1-4 * * *", "2012-07-01_01:00:14", "2012-07-01_01:00:00");
    check_fn(cron_prev, "*/15 * 1-4 * * *", "2012-07-01_01:00:00", "2012-06-30_04:59:45");
    check_fn(cron_prev, "* * * * * *", "2012-07-01_09:00:00", "2012-07-01_08:59:59");
    check_fn(cron_prev, "* * * * * *", "2021-01-01_00:00:00", "2020-12-31_23:59:59");
    check_fn(cron_prev, "0 0 * * * *", "2020-02-29_00:00:00", "2020-02-28_23:00:00");
    check_fn(cron_prev, "0 0 0 * * *", "2020-03-01_00:00:00", "2020-02-29_00:00:00");
    check_fn(cron_prev, "0 0 * * * *", "2020-03-01_00:00:00", "2020-02-29_23:00:00");

    check_fn(cron_next, "0 0 0 ? 11-12 *",   "2022-05-31_00:00:00", "2022-11-01_00:00:00");
    check_fn(cron_next, "0 0 0 ? 11-12 *",   "2022-07-31_00:00:00", "2022-11-01_00:00:00");
    check_fn(cron_next, "0 0 0 ? 11-12 *",   "2022-08-31_00:00:00", "2022-11-01_00:00:00");
    check_fn(cron_next, "0 0 0 ? 11-12 *",   "2022-10-31_00:00:00", "2022-11-01_00:00:00");
    check_fn(cron_next, "0 0 0 ? 6-7 *",     "2022-05-31_00:00:00", "2022-06-01_00:00:00");
    check_fn(cron_next, "0 0 0 ? 8-9 *",     "2022-07-31_00:00:00", "2022-08-01_00:00:00");
    check_fn(cron_next, "0 0 0 ? 9-10 *",    "2022-08-31_00:00:00", "2022-09-01_00:00:00");
    check_fn(cron_next, "0 0 0 ? 2-3 *",     "2022-01-31_00:00:00", "2022-02-01_00:00:00");
    check_fn(cron_next, "0 0 0 ? 4-5 *",     "2022-03-31_00:00:00", "2022-04-01_00:00:00");

    check_fn(cron_next, "* * * 29 2 *",  	"2021-12-07_12:00:00", "2024-02-29_00:00:00");
    check_fn(cron_prev, "* * * 29 2 *",  	"2021-12-07_12:00:00", "2020-02-29_23:59:59");

    check_fn(cron_prev, "* * * 1 2 *", "2023-11-01_00:00:00", "2023-02-01_23:59:59");
    check_fn(cron_prev, "* * * 2 2 *", "2023-11-01_00:00:00", "2023-02-02_23:59:59");
    check_fn(cron_prev, "* * * 3 2 *", "2023-11-01_00:00:00", "2023-02-03_23:59:59");
    check_fn(cron_prev, "* * * 4 2 *", "2023-11-01_00:00:00", "2023-02-04_23:59:59");

    check_fn(cron_prev, "* * * 1 4 *", "2023-10-01_00:00:00", "2023-04-01_23:59:59");
    check_fn(cron_prev, "* * * 2 4 *", "2023-10-01_00:00:00", "2023-04-02_23:59:59");
    check_fn(cron_prev, "* * * 3 4 *", "2023-10-01_00:00:00", "2023-04-03_23:59:59");
    check_fn(cron_prev, "* * * 4 4 *", "2023-10-01_00:00:00", "2023-04-04_23:59:59");

    check_fn(cron_prev, "0 0 20 1 2 *", "2022-12-30_08:10:23", "2022-02-01_20:00:00");
    check_fn(cron_prev, "0 0 20 2 2 *", "2022-12-30_08:10:23", "2022-02-02_20:00:00");
    check_fn(cron_prev, "0 0 20 3 2 *", "2022-12-30_08:10:23", "2022-02-03_20:00:00");
    check_fn(cron_prev, "0 0 20 1 2 *", "2022-12-31_08:10:23", "2022-02-01_20:00:00");
    check_fn(cron_prev, "0 0 20 2 2 *", "2022-12-31_08:10:23", "2022-02-02_20:00:00");
    check_fn(cron_prev, "0 0 20 3 2 *", "2022-12-31_08:10:23", "2022-02-03_20:00:00");
    check_fn(cron_prev, "0 0 20 1 2 *", "2023-01-01_08:10:23", "2022-02-01_20:00:00");
    check_fn(cron_prev, "0 0 20 2 2 *", "2023-01-01_08:10:23", "2022-02-02_20:00:00");
    check_fn(cron_prev, "0 0 20 3 2 *", "2023-01-01_08:10:23", "2022-02-03_20:00:00");

    check_fn(cron_prev, "0 0 17 * 2 2-4", "2023-08-31_18:00:00", "2023-02-28_17:00:00");
    check_fn(cron_prev, "0 0 17 * 2 2-4", "2023-09-01_18:00:00", "2023-02-28_17:00:00");
    check_fn(cron_prev, "0 0 17 * 2 2-4", "2023-09-02_18:00:00", "2023-02-28_17:00:00");
    check_fn(cron_prev, "0 0 17 * 2 2-4", "2023-09-03_18:00:00", "2023-02-28_17:00:00");
    check_fn(cron_prev, "0 0 17 * 2 2-4", "2023-09-04_18:00:00", "2023-02-28_17:00:00");
    check_fn(cron_prev, "0 0 17 * 2 2-4", "2023-09-05_18:00:00", "2023-02-28_17:00:00");

    check_fn(cron_prev, "0 0 17 * 3 1-5", "2023-03-02_17:00:00", "2023-03-01_17:00:00");
    check_fn(cron_prev, "0 0 17 * 3 1-5", "2023-03-02_17:00:01", "2023-03-02_17:00:00");
    check_fn(cron_prev, "0 0 17 * 3 1-5", "2023-03-02_18:00:00", "2023-03-02_17:00:00");
    check_fn(cron_prev, "0 0 17 * 3 1-5", "2023-03-03_18:00:00", "2023-03-03_17:00:00");
    check_fn(cron_prev, "0 0 17 * 3 1-5", "2023-03-04_18:00:00", "2023-03-03_17:00:00");
    check_fn(cron_prev, "0 0 17 * 3 1-5", "2023-03-05_18:00:00", "2023-03-03_17:00:00");
    check_fn(cron_prev, "0 0 17 * 3 1-5", "2023-03-06_18:00:00", "2023-03-06_17:00:00");

    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-05_18:00:00", "2023-04-29_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-06_09:29:59", "2023-04-29_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-06_09:30:00", "2023-04-29_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-06_09:30:01", "2024-04-06_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-06_18:00:00", "2024-04-06_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-07_18:00:00", "2024-04-06_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-26_18:00:00", "2024-04-20_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-27_18:00:00", "2024-04-27_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-04-28_18:00:00", "2024-04-27_09:30:00");
    check_fn(cron_prev, "0 30 9 * 4 6", "2024-05-01_18:00:00", "2024-04-27_09:30:00");

    check_fn(cron_prev, "0 30 11 * * 6", "2020-02-27_10:00:00", "2020-02-22_11:30:00");
    check_fn(cron_prev, "0 30 11 * * 6", "2020-02-28_10:00:00", "2020-02-22_11:30:00");
    check_fn(cron_prev, "0 30 11 * * 6", "2020-02-29_10:00:00", "2020-02-22_11:30:00");
    check_fn(cron_prev, "0 30 11 * * 6", "2020-02-29_11:31:00", "2020-02-29_11:30:00");
    check_fn(cron_prev, "0 30 11 * * 6", "2020-03-01_10:00:00", "2020-02-29_11:30:00");
    check_fn(cron_prev, "0 30 11 * * 6", "2020-03-01_12:00:00", "2020-02-29_11:30:00");

    check_fn(cron_prev, "0 0 10 * * *", "2020-02-29_09:59:59", "2020-02-28_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2020-02-29_10:00:00", "2020-02-28_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2020-02-29_10:00:01", "2020-02-29_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2022-02-28_09:59:59", "2022-02-27_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2022-02-28_10:00:00", "2022-02-27_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2022-02-28_10:00:01", "2022-02-28_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2022-12-31_09:59:59", "2022-12-30_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2022-12-31_10:00:00", "2022-12-30_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2022-12-31_10:00:01", "2022-12-31_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2022-12-31_23:59:59", "2022-12-31_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2023-01-01_00:00:00", "2022-12-31_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2023-01-01_00:00:01", "2022-12-31_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2023-01-01_09:59:59", "2022-12-31_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2023-01-01_10:00:00", "2022-12-31_10:00:00");
    check_fn(cron_prev, "0 0 10 * * *", "2023-01-01_10:00:01", "2023-01-01_10:00:00");

    check_fn(cron_prev, "30 50 23 20,21,22 * *", "2023-07-01_12:34:56", "2023-06-22_23:50:30");
    check_fn(cron_prev, "30 50 23 20,21,22 * *", "2023-07-19_12:34:56", "2023-06-22_23:50:30");
    check_fn(cron_prev, "30 50 23 20,21,22 * *", "2023-07-20_12:34:56", "2023-06-22_23:50:30");
    check_fn(cron_prev, "30 50 23 20,21,22 * *", "2023-07-21_12:34:56", "2023-07-20_23:50:30");
    check_fn(cron_prev, "30 50 23 20,21,22 * *", "2023-07-22_12:34:56", "2023-07-21_23:50:30");
    check_fn(cron_prev, "30 50 23 20,21,22 * *", "2023-07-23_12:34:56", "2023-07-22_23:50:30");
}

void test_parse() {
    check_same("* * * W * *", "* * * * * 1-5");
    check_same("* * * * * L", "* * * * * 0");
    check_same("* * * * * 6#-1", "* * * * * 6L");
    check_same("0 0 0 * * *", "0 0 * * *");
    check_same("0 0 0 * * *", "0 0 0 * * * *");
    check_same("* * * * * *", "* * * * * * *");

    check_same("@annually", "0 0 0 1 1 * *");
    check_same("@yearly", "0 0 0 1 1 * *");
    check_same("@monthly", "0 0 0 1 * * *");
    check_same("@weekly", "0 0 0 * * 0 *");
    check_same("@daily", "0 0 0 * * * *");
    check_same("@midnight", "0 0 0 * * * *");
    check_same("@hourly", "0 0 * * * * *");
    check_same("@minutely", "0 * * * * * *");
    check_same("@secondly", "* * * * * * *");

    check_same("* * * 2 * *", "* * * 2 * ?");
    check_same("* * * 2 * *", "* * * 2 * ?");
    check_same("57,59 * * * * *", "57/2 * * * * *");
    check_same("L57,59,61 * * * * *", "L57/2 * * * * *");
    check_same("1,3,5 * * * * *", "1-6/2 * * * * *");
    check_same("* * 4,8,12,16,20 * * *", "* * 4/4 * * *");
    check_same("* * * * * 0-6", "* * * * * TUE,WED,THU,FRI,SAT,SUN,MON");
    check_same("* * * * * 0", "* * * * * SUN");
    check_same("* * * * * 0", "* * * * * 7");
    check_same("* * * * 1-12 *", "* * * * FEB,JAN,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC *");
    check_same("* * * * 2 *", "* * * * Feb *");
    check_same("*  *  * *  1 *", "* * * * 1 *");

    check_expr_invalid("Z * * * * *");
    check_expr_invalid("* * * * * 1#-6");
    check_expr_invalid("* * * * * 1#6");

    check_expr_invalid("77 * * * * *");
    check_expr_invalid("44-77 * * * * *");
    check_expr_invalid("* 77 * * * *");
    check_expr_invalid("* 44-77 * * * *");
    check_expr_invalid("* * 27 * * *");
    check_expr_invalid("* * 23-28 * * *");
    check_expr_invalid("* * * 45 * *");
    check_expr_invalid("* * * 28-45 * *");
    check_expr_invalid("0 0 0 25 13 ?");
    check_expr_invalid("0 0 0 25 0 ?");
    check_expr_invalid("0 0 0 32 12 ?");
    check_expr_invalid("* * * * 11-13 *");
    check_expr_invalid("-5 * * * * *");
    check_expr_invalid("3-2 */5 * * * *");
    check_expr_invalid("/5 * * * * *");
    check_expr_invalid("*/0 * * * * *");
    check_expr_invalid("*/-0 * * * * *");
    check_expr_invalid("* 1 1 0 * *");

    /* Source: https://www.freeformatter.com/cron-expression-generator-quartz.html */

    check_expr_valid("* * * ? * *");                /* Every second */
    check_expr_valid("0 * * ? * *");                /* Every minute */
    check_expr_valid("0 */2 * ? * *");              /* Every even minute */
    check_expr_valid("0 1/2 * ? * *");              /* Every uneven minute */
    check_expr_valid("0 */2 * ? * *");              /* Every 2 minutes */
    check_expr_valid("0 */3 * ? * *");              /* Every 3 minutes */
    check_expr_valid("0 */4 * ? * *");              /* Every 4 minutes */
    check_expr_valid("0 */5 * ? * *");              /* Every 5 minutes */
    check_expr_valid("0 */10 * ? * *");             /* Every 10 minutes */
    check_expr_valid("0 */15 * ? * *");             /* Every 15 minutes */
    check_expr_valid("0 */30 * ? * *");             /* Every 30 minutes */
    check_expr_valid("0 15,30,45 * ? * *");         /* Every hour at minutes 15, 30 and 45 */
    check_expr_valid("0 0 * ? * *");                /* Every hour */
    check_expr_valid("0 0 */2 ? * *");              /* Every hour */
    check_expr_valid("0 0 0/2 ? * *");              /* Every even hour */
    check_expr_valid("0 0 1/2 ? * *");              /* Every uneven hour */
    check_expr_valid("0 0 */3 ? * *");              /* Every three hours */
    check_expr_valid("0 0 */4 ? * *");              /* Every four hours */
    check_expr_valid("0 0 */6 ? * *");              /* Every six hours */
    check_expr_valid("0 0 */8 ? * *");              /* Every eight hours */
    check_expr_valid("0 0 */12 ? * *");             /* Every twelve hours */
    check_expr_valid("0 0 0 * * ?");                /* Every day at midnight - 12am */
    check_expr_valid("0 0 1 * * ?");                /* Every day at 1am */
    check_expr_valid("0 0 6 * * ?");                /* Every day at 6am */
    check_expr_valid("0 0 12 * * ?");               /* Every day at noon - 12pm */
    check_expr_valid("0 0 12 * * ?");               /* Every day at noon - 12pm */
    check_expr_valid("0 0 12 ? * SUN");             /* Every Sunday at noon */
    check_expr_valid("0 0 12 ? * MON");             /* Every Monday at noon */
    check_expr_valid("0 0 12 ? * TUE");             /* Every Tuesday at noon */
    check_expr_valid("0 0 12 ? * WED");             /* Every Wednesday at noon */
    check_expr_valid("0 0 12 ? * THU");             /* Every Thursday at noon */
    check_expr_valid("0 0 12 ? * FRI");             /* Every Friday at noon */
    check_expr_valid("0 0 12 ? * SAT");             /* Every Saturday at noon */
    check_expr_valid("0 0 12 ? * MON-FRI");         /* Every Weekday at noon */
    check_expr_valid("0 0 12 ? * SUN,SAT");         /* Every Saturday and Sunday at noon */
    check_expr_valid("0 0 12 */7 * ?");             /* Every 7 days at noon */
    check_expr_valid("0 0 12 1 * ?");               /* Every month on the 1st, at noon */
    check_expr_valid("0 0 12 2 * ?");               /* Every month on the 2nd, at noon */
    check_expr_valid("0 0 12 15 * ?");              /* Every month on the 15th, at noon */
    check_expr_valid("0 0 12 1/2 * ?");             /* Every 2 days starting on the 1st of the month, at noon */
    check_expr_valid("0 0 12 1/4 * ?");             /* Every 4 days staring on the 1st of the month, at noon */
    check_expr_valid("0 0 12 L * ?");               /* Every month on the last day of the month, at noon */
    check_expr_valid("0 0 12 L-2 * ?");             /* Every month on the second to last day of the month, at noon */
    check_expr_valid("0 0 12 LW * ?");              /* Every month on the last weekday, at noon */

    /*check_expr_valid("0 0 12 1L * ?"); */             /* Every month on the last Sunday, at noon */
    /*check_expr_valid("0 0 12 2L * ?"); */             /* Every month on the last Monday, at noon */
    /*check_expr_valid("0 0 12 6L * ?"); */             /* Every month on the last Friday, at noon */

    check_expr_valid("0 0 12 ? * 1L");              /* Every month on the last Sunday, at noon */
    check_expr_valid("0 0 12 ? * 2L");              /* Every month on the last Monday, at noon */
    check_expr_valid("0 0 12 ? * 6L");              /* Every month on the last Friday, at noon */

    check_expr_valid("0 0 12 1W * ?");              /* Every month on the nearest Weekday to the 1st of the month, at noon */
    check_expr_valid("0 0 12 15W * ?");             /* Every month on the nearest Weekday to the 15th of the month, at noon */
    check_expr_valid("0 0 12 ? * 2#1");             /* Every month on the first Monday of the Month, at noon */
    check_expr_valid("0 0 12 ? * 6#1");             /* Every month on the first Friday of the Month, at noon */
    check_expr_valid("0 0 12 ? * 2#2");             /* Every month on the second Monday of the Month, at noon */
    check_expr_valid("0 0 12 ? * 5#3");             /* Every month on the third Thursday of the Month, at noon - 12pm */
    check_expr_valid("0 0 12 ? JAN *");             /* Every day at noon in January only */
    check_expr_valid("0 0 12 ? JUN *");             /* Every day at noon in June only */
    check_expr_valid("0 0 12 ? JAN,JUN *");         /* Every day at noon in January and June */
    check_expr_valid("0 0 12 ? DEC *");             /* Every day at noon in December only */
    check_expr_valid("0 0 12 ? JAN,FEB,MAR,APR *"); /* Every day at noon in January, February, March and April */
    check_expr_valid("0 0 12 ? 9-12 *");            /* Every day at noon between September and December */

    /* ChatGPT generated inputs for further testing. */

    check_expr_valid("0 0 12 * * ?"); /* Every day at 12 PM (noon). */
    check_expr_valid("0 15 10 ? * *"); /* Every day at 10:15 AM. */
    check_expr_valid("0 15 10 * * ?"); /* Every day at 10:15 AM. */
    check_expr_valid("0 15 10 * * ? *"); /* Every day at 10:15 AM. */
    check_expr_valid("0 15 10 * * ? 2023"); /* Every day at 10:15 AM during the year 2023. */
    check_expr_valid("0 * 14 * * ?"); /* Every minute starting at 2 PM and ending at 2:59 PM, every day. */
    check_expr_valid("0 0/5 14 * * ?"); /* Every 5 minutes starting at 2 PM and ending at 2:55 PM, every day. */
    check_expr_valid("0 0/5 14,18 * * ?"); /* Every 5 minutes starting at 2 PM and ending at 2:55 PM, AND every 5 minutes starting at 6 PM and ending at 6:55 PM, every day. */
    check_expr_valid("0 0-5 14 * * ?"); /* Every minute starting at 2 PM and ending at 2:05 PM, every day. */
    check_expr_valid("0 10,44 14 ? 3 WED"); /* Every Wednesday in March at 2:10 PM and 2:44 PM. */
    check_expr_valid("0 15 10 ? * MON-FRI"); /* Every weekday at 10:15 AM. */
    check_expr_valid("0 15 10 15 * ?"); /* Every 15th day of the month at 10:15 AM. */
    check_expr_valid("0 15 10 L * ?"); /* Last day of every month at 10:15 AM. */
    check_expr_valid("0 15 10 ? * 6L"); /* Last Friday of every month at 10:15 AM. */
    check_expr_valid("0 15 10 ? * 6L 2022-2025"); /* Last Friday of every month during the years 2022 through 2025 at 10:15 AM. */
    check_expr_valid("0 15 10 ? * 6#3"); /* Third Friday of every month at 10:15 AM. */
    check_expr_valid("0 15 10 ? * 2-6"); /* Every weekday (Monday to Friday) at 10:15 AM. */
    check_expr_valid("0 0/5 14-18 * * ?"); /* Every 5 minutes from 2 PM to 6:55 PM every day. */
    check_expr_valid("0 0 12 1/5 * ?"); /* Every 5 days at 12 PM. */
    check_expr_valid("0 11 11 11 11 ?"); /* Every November 11th at 11:11 AM. */
    check_expr_valid("0 0 12 ? * SUN"); /* Every Sunday at noon. */
    check_expr_valid("0 0 12 ? * MON"); /* Every Monday at noon. */
    check_expr_valid("0 0 12 ? * TUE"); /* Every Tuesday at noon. */
    check_expr_valid("0 0 12 ? * WED"); /* Every Wednesday at noon. */
    check_expr_valid("0 0 12 ? * THU"); /* Every Thursday at noon. */
    check_expr_valid("0 0 12 ? * FRI"); /* Every Friday at noon. */
    check_expr_valid("0 0 12 ? * SAT"); /* Every Saturday at noon. */
    check_expr_valid("0 0/30 8-9 1 * ?"); /* Every 30 minutes between 8-9 AM on the 1st of the month. */
    check_expr_valid("0 0 0 1 1 ?"); /* Every New Year's Day at midnight. */
    check_expr_valid("0 0 0 25 12 ?"); /* Every Christmas at midnight. */
    check_expr_valid("0 0 6,18 * * ?"); /* Every day at 6 AM and 6 PM. */
    check_expr_valid("0 0 8-10 ? * 2-6"); /* Every weekday between 8-10 AM. */
    check_expr_valid("0 0/15 * ? * *"); /* Every 15 minutes. */
    check_expr_valid("0 0 12 LW * ?"); /* Last weekday of the month at noon. */
    check_expr_valid("0 0 12 1W * ?"); /* Nearest weekday to the 1st of the month at noon. */
    check_expr_valid("0 0 12 W * ?"); /* Every weekday at noon. */
    check_expr_valid("0 0 12 ? JAN,DEC *"); /* Every day at noon in January and December. */
    check_expr_valid("0 0 12 ? * 1#2"); /* Second Sunday of every month at noon. */
    check_expr_valid("0 0 12 ? * 2#2"); /* Second Monday of every month at noon. */
    check_expr_valid("0 0 12 ? * 6#1"); /* First Friday of every month at noon. */
    check_expr_valid("0 0 8,20 ? * *"); /* Every day at 8 AM and 8 PM. */
    check_expr_valid("0 30 10-13 ? * WED,FRI"); /* Every Wednesday and Friday between 10:30 AM and 1:30 PM. */
    check_expr_valid("0 0/10 * * * ?"); /* Every 10 minutes. */
    check_expr_valid("0 0 12 L-2 * ?"); /* Two days before the end of the month at noon. */
    check_expr_valid("0 0 12 15W * ?"); /* Nearest weekday to the 15th of every month at noon. */
    check_expr_valid("0 0 0 * * ?"); /* Every day at midnight (beginning of the day). */
    check_expr_valid("0 0 23 * * ?"); /* Every day at 11 PM (end of the day). */
    check_expr_valid("0 30 10 ? * 2-6"); /* Every weekday at 10:30 AM. */
    check_expr_valid("0 0/10 8-17 ? * MON-FRI"); /* Every 10 minutes during working hours (8 AM - 5 PM) on weekdays. */
    /*check_expr_valid("0 0 12 1L * ?"); */ /* Last day of the month at noon. */
    check_expr_valid("0 0 12 ? * SUN,MON"); /* Every Sunday and Monday at noon. */
    check_expr_valid("0 0/5 9-16 * * ?"); /* Every 5 minutes from 9 AM to 4:55 PM every day. */
    check_expr_valid("0 0 12 10-15 * ?"); /* Every day from the 10th to the 15th at noon. */
    check_expr_valid("0 0 0 1 JAN-JUN ?"); /* First day of the month from January to June at midnight. */
    check_expr_valid("0 0 0 ? * 2L"); /* Last Monday of the month at midnight. */
    check_expr_valid("0 0 0 ? * 6#4"); /* Fourth Friday of the month at midnight. */
    check_expr_valid("0 0 12 1/2 * ?"); /* Every other day at noon. */
    check_expr_valid("0 0 12 ? * 2/2"); /* Every other Monday at noon. */
    check_expr_valid("0 0 0 29 FEB ?"); /* Every 29th of February at midnight (Leap Year). */
    check_expr_valid("0 0 12 1,15 * ?"); /* 1st and 15th of the month at noon. */
    check_expr_valid("0 0 0 1 * ? 2024"); /* 1st of every month in 2024 at midnight. */
    check_expr_valid("0 30 6 ? * 1-5"); /* Weekdays at 6:30 AM. */
    check_expr_valid("0 0 0/2 * * ?"); /* Every 2 hours. */
    check_expr_valid("0 0 12/3 ? * *"); /* Every 3 hours starting at noon. */
    check_expr_valid("0 15,45 * ? * *"); /* Every hour at 15 and 45 minutes past. */
    check_expr_valid("0 0 0 ? * SAT,SUN"); /* Every weekend at midnight. */
    check_expr_valid("0 0 8-10,14-16 * * ?"); /* Every day from 8-10 AM and 2-4 PM. */
    check_expr_valid("0 0 12 ? 1-6,9-12 *"); /* Every day at noon excluding July and August. */
    check_expr_valid("0 0 12/4 * * ?"); /* Every 4 hours starting at noon. */
    check_expr_valid("0 0 9-17 * * ?"); /* Every hour from 9 AM to 5 PM. */
    check_expr_valid("0 0/20 9-16 * * ?"); /* Every 20 minutes from 9 AM to 4:40 PM. */
    check_expr_valid("0 0 12 ? * 2-6"); /* Every weekday at noon. */
    check_expr_valid("0 0 8-11,13-16 * * ?"); /* Every day from 8-11 AM and 1-4 PM. */
    check_expr_valid("0 0/30 6-18 * * ?"); /* Every 30 minutes from 6 AM to 6:30 PM. */
    check_expr_valid("0 0 12 ? JAN,MAR,MAY,JUL,SEP,NOV *"); /* Every alternate month at noon. */
    check_expr_valid("0 15 9 ? * MON,TUE,WED,THU,FRI"); /* Every weekday at 9:15 AM. */
    check_expr_valid("0 0/40 * ? * *"); /* Every 40 minutes. */
    check_expr_valid("0 30 10-14 ? * ?"); /* Every day from 10:30 AM to 2:30 PM. */
    check_expr_valid("0 0 12 * JAN-DEC ?"); /* Every day of the year at noon. */
    check_expr_valid("0 0 0 25 12 ? *"); /* Every Christmas at midnight. */
    check_expr_valid("0 0 0/3 * * ?"); /* Every 3 hours. */
    check_expr_valid("0 0 12 * * ? 2025"); /* Every day at noon in the year 2025. */
    check_expr_valid("0 0 6 ? * 2-6"); /* Every weekday at 6 AM. */
    check_expr_valid("0 0 18 ? * 1-5"); /* Every weekday at 6 PM. */
    check_expr_valid("0 0 0 ? * * 2026"); /* Every day at midnight in 2026. */
    check_expr_valid("0 0/45 10-14 * * ?"); /* Every 45 minutes between 10 AM and 2:45 PM. */
    check_expr_valid("0 0 12 1/3 * ?"); /* Every 3 days at noon. */
    check_expr_valid("0 0 0 1 JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC ?"); /* Start of every month at midnight. */
    check_expr_valid("0 0 12 10,20,30 * ?"); /* Every 10th, 20th, and 30th of the month at noon. */
    check_expr_valid("0 0/50 8-16 * * ?"); /* Every 50 minutes from 8 AM to 4:50 PM. */
    check_expr_invalid("0 0 12 ? * 1#1,3#3,5#5"); /* First Sunday, third Wednesday, and fifth Friday of every month at noon. */
    check_expr_valid("0 0 0/4 * * ?"); /* Every 4 hours. */
    check_expr_valid("0 15,30,45 * * * ?"); /* Every hour at 15, 30, and 45 minutes past. */
    check_expr_valid("0 0 12 ? * 2L"); /* Last Monday of every month at noon. */
    check_expr_valid("0 0 12 ? * 5L"); /* Last Thursday of every month at noon. */
    check_expr_valid("0 0 0/6 * * ?"); /* Every 6 hours. */
    check_expr_valid("0 0 12/2 ? * *"); /* Every 2 hours starting at noon. */
    check_expr_valid("0 0 12 ? 1,3,5,7,9,11 *"); /* Every odd month at noon. */
    check_expr_valid("0 0 0 ? 2,4,6,8,10,12 *"); /* Every even month at midnight. */
    check_expr_valid("0 0 12 * * ? 2027"); /* Every day at noon in the year 2027. */
}

void test_bits() {

    uint8_t testbyte[8];
    memset(testbyte, 0, 8);
    int err = 0;
    int i;

    for (i = 0; i <= 63; i++) {
        cron_set_bit(testbyte, i);
        if (!cron_get_bit(testbyte, i)) {
            printf("Bit set error! Bit: %d!\n", i);
            err = 1;
        }
        cron_del_bit(testbyte, i);
        if (cron_get_bit(testbyte, i)) {
            printf("Bit clear error! Bit: %d!\n", i);
            err = 1;
        }
        assert(!err);
    }

    for (i = 0; i < 12; i++) {
        cron_set_bit(testbyte, i);
    }
    if (testbyte[0] != 0xff) {
        err = 1;
    }
    if (testbyte[1] != 0x0f) {
        err = 1;
    }

    assert(!err);
}

int main() {
    test_bits();

    test_expr();
    test_parse();
    check_calc_invalid();
    printf("\nAll OK!\n");
    return 0;
}

