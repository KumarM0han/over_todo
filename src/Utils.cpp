#include "Utils.h"
#include <cstdio>
#include <ctime>

void get_current_date(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &now); // Windows (thread-safe)
#else
    localtime_r(&now, &tm_info);   // POSIX (thread-safe)
#endif
    strftime(buffer, size, "%Y/%m/%d", &tm_info);
}

long days_since(const char *date_str)
{
    int year, month, day;
    if (sscanf(date_str, "%d/%d/%d", &year, &month, &day) != 3)
        return -1;
    struct tm tm_date = {0};
    tm_date.tm_year = year - 1900;
    tm_date.tm_mon = month - 1;
    tm_date.tm_mday = day;
    tm_date.tm_hour = 0;
    tm_date.tm_min = 0;
    tm_date.tm_sec = 0;
    time_t past = mktime(&tm_date);
    if (past == (time_t)-1)
        return -1;
    time_t now = time(NULL);
    double diff_seconds = difftime(now, past);
    return (long)(diff_seconds / 86400);
}

long days_since_2(const char *start, const char *end)
{
    int st_year, st_month, st_day;
    if (sscanf(start, "%d/%d/%d", &st_year, &st_month, &st_day) != 3)
        return -1;
    
    int end_year, end_month, end_day;
    if (sscanf(end, "%d/%d/%d", &end_year, &end_month, &end_day) != 3)
        return -1;
    
    struct tm tm_start = {0};
    tm_start.tm_year = st_year - 1900;
    tm_start.tm_mon = st_month - 1;
    tm_start.tm_mday = st_day;
    tm_start.tm_hour = 0;
    tm_start.tm_min = 0;
    tm_start.tm_sec = 0;
    time_t start_time = mktime(&tm_start);

    struct tm tm_end = {0};
    tm_end.tm_year = end_year - 1900;
    tm_end.tm_mon = end_month - 1;
    tm_end.tm_mday = end_day;
    tm_end.tm_hour = 0;
    tm_end.tm_min = 0;
    tm_end.tm_sec = 0;
    time_t end_time = mktime(&tm_end);

    if (start_time == (time_t)-1 || end_time == (time_t)-1)
        return -1;
    double diff_seconds = difftime(end_time, start_time);
    return (long)(diff_seconds / 86400);
}