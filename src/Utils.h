#pragma once

void get_current_date(char *buffer, size_t size);

long days_since(const char *date_str);
long days_since_2(const char *start, const char *end);