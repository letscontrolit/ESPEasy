
supertinycron
=============

A very small replacement for cron. Particularly useful within containers and for distributing cron tasks alongside a project as a standalone file.

Installing
----------

```bash
make
sudo mv supertinycron /usr/local/bin/
```

Usage
-----

```
supertinycron [expression] [command...]
```

Supertinycron can be conveniently used in your scripts interpreter line:
```bash
#!/usr/local/bin/supertinycron */5 * * * * * * /bin/sh
echo "Current time: $(date)"
```

Or invoked directly via commandline:
```bash
$ supertinycron '*/5 * * * * * *' /bin/echo hello
```

Implementation
--------------

This reference is based on:
- [1] [gorhill's cronexpr on GitHub](https://github.com/gorhill/cronexpr/blob/master/README.md)
- [2] [Quartz Scheduler's CronExpression](https://www.javadoc.io/doc/org.quartz-scheduler/quartz/latest/org/quartz/CronExpression.html)
- [3] [ccronexpr by staticlibs on GitHub](https://github.com/staticlibs/ccronexpr/blob/master/README.md)
- [4] [ccronexpr by mdvorak on GitHub](https://github.com/mdvorak/ccronexpr/blob/main/README.md)
- [5] [Wikipedia on CRON Expression](https://en.wikipedia.org/wiki/Cron#CRON_expression)

```
Field name     Mandatory?   Allowed values          Allowed special characters
----------     ----------   --------------          -------------------------
Second         No           0-59                    * / , - L
Minute         Yes          0-59                    * / , -
Hour           Yes          0-23                    * / , -
Day of month   Yes          1-31                    * / , - L W
Month          Yes          1-12 or JAN-DEC         * / , -
Day of week    Yes          0-6 or SUN-SAT          * / , - L #
Year           No           1970–2199               * / , -
```

**Note:** In the 'Day of week' field, both 0 and 7 represent SAT, as referenced by [crontab's man page](http://linux.die.net/man/5/crontab#). The 'Year' field spans to 2199 as per [2], which differs from [1] where it's up to 2099. When `*` is used for year it should function well above year 2199. Depending on libc, it may function well above year 10000.

### Special Characters

#### Asterisk `*`
The asterisk indicates that the cron expression matches all values of the field. For instance, an asterisk in the 'Month' field matches every month.

#### Hyphen `-`
Hyphens define ranges. For instance, `2000-2010` in the 'Year' field matches every year from 2000 to 2010, inclusive.

#### Slash `/`
Slashes specify increments within ranges. For example, `3-59/15` in the 'Minute' field matches the third minute of the hour and every 15 minutes thereafter. The form `*/...` is equivalent to `first-last/...`, representing an increment over the full range of the field.

#### Comma `,`
Commas separate items in a list. For instance, `MON,WED,FRI` in the 'Day of week' field matches Mondays, Wednesdays, and Fridays.

#### `L`
The character `L` stands for "last". In the 'Day of week' field, `5L` denotes the last Friday of a given month. In the 'Day of month' field, it represents the last day of the month.

- Using `L` alone in the 'Day of week' field is equivalent to `0` or `SAT`. Hence, expressions `* * * * * L *` and `* * * * * 0 *` are the same.
  
- When followed by another value in the 'Day of week' field, like `6L`, it signifies the last Friday of the month.
  
- If followed by a negative number in the 'Day of month' field, such as `L-3`, it indicates the third-to-last day of the month.

- If `L` is present in the beginning of 'Second' field, it turns on non standard leap second functionality. Unless timezone specifies leap seconds, it will cycle indefinitely, because it will not be able to find any leap second!

When using 'L', avoid specifying lists or ranges to prevent ambiguous results.

#### `W`
The `W` character is exclusive to the 'Day of month' field. It indicates the closest business day (Monday-Friday) to the given day. For example, `15W` means the nearest business day to the 15th of the month. If you set 1W for the day-of-month and the 1st falls on a Saturday, the trigger activates on Monday the 3rd, since it respects the month's day boundaries and won't skip over them. Similarly, at the end of the month, the behavior ensures it doesn't "jump" over the boundary to the following month.

The `W` character can also pair with `L` (as `LW`), signifying the last business day of the month. Alone, it's equivalent to the range `1-5`, making the expressions `* * * W * * *` and `* * * * * 1-5 *` identical. This interpretation differs from [1,2].

#### Hash `#`
The `#` character is only for the 'Day of week' field and should be followed by a number between one and five, or their negative values. It lets you specify constructs like "the second Friday" of a month.

For example, `6#3` means the third Friday of the month. Note that if you use `#5` and there isn't a fifth occurrence of that weekday in the month, no firing occurs for that month. Using the '#' character requires a single expression in the 'Day of week' field.

Negative nth values are also valid. For instance, `6#-1` is equivalent to `6L`.

#### Known limitation

1. Leap seconds can be prefixed with multiple `L` symbols: `LLLL60` without issuing errors.
2. Ordinals `JAN`...`DEC` and `SUN`...`SAT` are processed in all fields without issuing errors.
3. Errors from lexical analyzers are masked by parser errors.
4. Multiple `#` segments: `1#1,3#3,5#5` are not allowed.

Predefined cron expressions
---------------------------
(Copied from <https://en.wikipedia.org/wiki/Cron#Predefined_scheduling_definitions>, with text modified according to this implementation)

    Entry       Description                                                             Equivalent to
    @annually   Run once a year at midnight in the morning of January 1                 0 0 0 1 1 *
    @yearly     Run once a year at midnight in the morning of January 1                 0 0 0 1 1 *
    @monthly    Run once a month at midnight in the morning of the first of the month   0 0 0 1 * *
    @weekly     Run once a week at midnight in the morning of Sunday                    0 0 0 * * 0
    @daily      Run once a day at midnight                                              0 0 0 * * *
    @hourly     Run once an hour at the beginning of the hour                           0 0 * * * *
    @minutely   Run once a minute at the beginning of minute                            0 * * * * *
    @secondly   Run once every second                                                   * * * * * * *
    @reboot     Not supported

Note that `@minutely` and `@secondly` are not standard.

Other details
-------------
* If only five fields are present, the Year and Second fields are omitted. The omitted Year and Second are `*` and `0` respectively.
* If only six fields are present, the Year field is omitted. The omitted Year is set to `*`. Note that this is different from [1] which has Second field omitted in this case and [2] which doesn't allow five fields.
* Only proper expressions are guaranteed to work.
* Cron doesn't decide calendar, it follows it. It doesn't and it should not disallow combinations like 31st April or 30rd February. Not only that these dates hisctorically happened, but they may very well happen based on timezone configuration. Within reasonable constrains, it should work under changed conditions.

Config
------

TinyCron can be configured by setting the below environmental variables to a non-empty value:

Variable | Description
--- | ---
TINYCRON_VERBOSE | Enable verbose output


Cron expression parsing in ANSI C
=================================

[![Build](https://github.com/mdvorak/ccronexpr/actions/workflows/build.yml/badge.svg)](https://github.com/mdvorak/ccronexpr/actions/workflows/build.yml)

Given a cron expression and a date, you can get the next date which satisfies the cron expression.

Supports cron expressions with `seconds` field. Based on implementation of [CronSequenceGenerator](https://github.com/spring-projects/spring-framework/blob/babbf6e8710ab937cd05ece20270f51490299270/spring-context/src/main/java/org/springframework/scheduling/support/CronSequenceGenerator.java) from Spring Framework.

Compiles and should work on Linux (GCC/Clang), Mac OS (Clang), Windows (MSVC), Android NDK, iOS and possibly on other platforms with `time.h` support.

Supports compilation in C (89) and in C++ modes.

Usage example
-------------

    #include "ccronexpr.h"

    cron_expr expr;
    const char* err = NULL;
    memset(&expr, 0, sizeof(expr));
    cron_parse_expr("0 */2 1-4 * * *", &expr, &err);
    if (err) ... /* invalid expression */
    time_t cur = time(NULL);
    time_t next = cron_next(&expr, cur);


Compilation and tests run examples
----------------------------------

    gcc ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c89 -o a.out && ./a.out
    g++ ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c++11 -o a.out && ./a.out
    g++ ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c++11 -DCRON_COMPILE_AS_CXX -o a.out && ./a.out

    clang ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c89 -o a.out && ./a.out
    clang++ ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c++11 -o a.out && ./a.out
    clang++ ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c++11 -DCRON_COMPILE_AS_CXX -o a.out && ./a.out

    cl ccronexpr.c ccronexpr_test.c /W4 /D_CRT_SECURE_NO_WARNINGS && ccronexpr.exe


    gcc ccronexpr.c supertinycron.c -I. -Wall -Wextra -std=c89 -o a.out && ./a.out
    g++ ccronexpr.c supertinycron.c -I. -Wall -Wextra -std=c++11 -o a.out && ./a.out
    g++ ccronexpr.c supertinycron.c -I. -Wall -Wextra -std=c++11 -DCRON_COMPILE_AS_CXX -o a.out && ./a.out

    clang ccronexpr.c supertinycron.c -I. -Wall -Wextra -std=c89 -o a.out && ./a.out
    clang++ ccronexpr.c supertinycron.c -I. -Wall -Wextra -std=c++11 -o a.out && ./a.out
    clang++ ccronexpr.c supertinycron.c -I. -Wall -Wextra -std=c++11 -DCRON_COMPILE_AS_CXX -o a.out && ./a.out

    cl ccronexpr.c supertinycron.c /W4 /D_CRT_SECURE_NO_WARNINGS && supertinycron.exe

Examples of supported expressions
---------------------------------

Expression, input date, next date:

    "*/15 * 1-4 * * *",  "2012-07-01_09:53:50", "2012-07-02_01:00:00"
    "0 */2 1-4 * * *",   "2012-07-01_09:00:00", "2012-07-02_01:00:00"
    "0 0 7 ? * MON-FRI", "2009-09-26_00:42:55", "2009-09-28_07:00:00"
    "0 30 23 30 1/3 ?",  "2011-04-30_23:30:00", "2011-07-30_23:30:00"

See more examples in [tests](https://github.com/staticlibs/ccronexpr/blob/a1343bc5a546b13430bd4ac72f3b047ac08f8192/ccronexpr_test.c#L251).

Timezones
---------

This implementation does not support explicit timezones handling. By default, all dates are
processed as UTC (GMT) dates without timezone information.

To use local dates (current system timezone) instead of GMT compile with `-DCRON_USE_LOCAL_TIME`, example:

    gcc -DCRON_USE_LOCAL_TIME ccronexpr.c ccronexpr_test.c -I. -Wall -Wextra -std=c89 -o a.out && TZ="America/Toronto" ./a.out

Years
-----

To disable Year field use `-DCRON_DISABLE_YEARS`. This will lower memory footriprint by 29 bytes for `cron_expr`. It will still accept year field, but field will not be validated and it will be triggered every year.

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Changelog
---------

**2023**
* major extension of supported expressions
* command line tool

**2022**

* added CMake build
* added GitHub Workflow for continuous testing
* fixed type casts to support `-Wconvert`
* added tests for cron_prev and leap years
* fixed tests to work with `CRON_USE_LOCAL_TIME`
* added [ESP-IDF](./ESP-IDF.md) usage guide

**2019-03-27**

 * `CRON_USE_LOCAL_TIME` usage fixes

**2018-05-23**

 * merged [#8](https://github.com/staticlibs/ccronexpr/pull/8)
 * merged [#9](https://github.com/staticlibs/ccronexpr/pull/9)
 * minor cleanups

**2018-01-27**

 * merged [#6](https://github.com/staticlibs/ccronexpr/pull/6)
 * updated license file (to the one parse-able by github)

**2017-09-24**

 * merged [#4](https://github.com/staticlibs/ccronexpr/pull/4)

**2016-06-17**

 * use thread-safe versions of `gmtime` and `localtime`

**2015-02-28**

 * initial public version

