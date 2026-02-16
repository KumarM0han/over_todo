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